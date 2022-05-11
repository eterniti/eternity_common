#ifndef __THREAD_H__
#define __THREAD_H__

#include "Mutex.h"
#include "Event.h"

class Runnable
{
public:

    virtual ~Runnable() { }

    virtual uint32_t Run() = 0;
    virtual void Stop() { }
};

typedef uint32_t (* RUNNABLE_FUNCTION)(void *arg);

class RunnableFunction : public Runnable
{
private:

    RUNNABLE_FUNCTION function;
    void *arg;

public:

    RunnableFunction(RUNNABLE_FUNCTION function, void *arg=nullptr) : function(function), arg(arg) { }
    virtual ~RunnableFunction() { }

    virtual uint32_t Run() override
    {
        return function(arg);
    }
};

class Thread
{
private:

    HANDLE handle;
    Runnable *runnable;
    bool auto_delete;
    bool started;
    bool running;
    bool finished;

    mutable Mutex mutex;

    static DWORD WINAPI ThreadProc(LPVOID pthis)
    {
        return (((Thread *)pthis)->Run());
    }

    uint32_t Run()
    {
         uint32_t ret = runnable->Run();

         MutexLocker lock(&mutex);
         running = false;
         finished = true;
         return ret;
    }

public:

    Thread(Runnable *runnable, bool auto_delete, unsigned int stack_size=0) : runnable(runnable), auto_delete(auto_delete)
    {
        DWORD tid;
        handle = CreateThread(nullptr, stack_size, ThreadProc, this, STACK_SIZE_PARAM_IS_A_RESERVATION|CREATE_SUSPENDED, &tid);
        started = finished = running = false;
    }

    ~Thread()
    {
        Kill();
    }

    bool IsStarted() const
    {
        MutexLocker lock(&mutex);
        return started;
    }

    bool IsRunning() const
    {
        MutexLocker lock(&mutex);
        return running;
    }

    bool IsFinished() const
    {
        MutexLocker lock(&mutex);
        return finished;
    }

    void Start()
    {
        if (IsStarted())
            return;

        Resume();
    }

    void Suspend()
    {
        MutexLocker lock(&mutex);

        if (!running || finished)
            return;

        SuspendThread(handle);
        running = false;
    }

    void Resume()
    {
        MutexLocker lock(&mutex);

        if (running || finished)
            return;

        ResumeThread(handle);
        running = true;
        started = true;
    }

    void Wait()
    {
        HANDLE h = handle;
        if (!h)
            return;

        WaitForSingleObject(h, INFINITE);

        MutexLocker lock(&mutex);
        running = false;
        finished = true;
    }

    void Kill()
    {
        if (IsFinished())
            return;

        runnable->Stop();
        Resume();
        Wait();

        MutexLocker lock(&mutex);

        if (auto_delete)
        {
            delete runnable;
            runnable = nullptr;
        }

        CloseHandle(handle);
        handle = nullptr;
        finished = true;
    }

    static int LogicalCoresCount();
    static int PhisycalCoresCount();
};

class PoolThread;

class ThreadPool
{
private:

    friend class PoolThread;

    mutable Mutex mutex;
    std::vector<PoolThread *> threads;
    std::vector<Runnable *> works;
    std::vector<bool> running;
    Event completion;

    void ProcessQueue();
    void WorkDone(Runnable *work);

public:

    ThreadPool(int num_threads=0, bool auto_delete=true, unsigned int stack_size=0);
    ~ThreadPool();

    void AddWork(Runnable *work);
    void Wait();
};

#endif // __THREAD_H__
