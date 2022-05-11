#include "Thread.h"
#include "debug.h"

static DWORD CountSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest)?1:0);
        bitTest/=2;
    }

    return bitSetCount;
}

int Thread::LogicalCoresCount()
{
    static int count = 0;
    static Mutex mutex;

    MutexLocker lock(&mutex);

    if (count != 0)
        return count;

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pinfo = nullptr, ptr;
    DWORD length = 0;
    DWORD offset = 0;

    if (GetLogicalProcessorInformation(pinfo, &length))
        return 1;

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return 1;

    pinfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(length);
    if (!GetLogicalProcessorInformation(pinfo, &length))
    {
        free(pinfo);
        return 1;
    }

    ptr = pinfo;
    while (offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= length)
    {
        if (ptr->Relationship == RelationProcessorCore)
        {
            count += CountSetBits(ptr->ProcessorMask);
        }

        offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    free(pinfo);
    return (count >= 1) ? count : 1;
}

int Thread::PhisycalCoresCount()
{
    static int count = 0;
    static Mutex mutex;

    MutexLocker lock(&mutex);

    if (count != 0)
        return count;

    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION pinfo = nullptr, ptr;
    DWORD length = 0;
    DWORD offset = 0;

    if (GetLogicalProcessorInformation(pinfo, &length))
        return 1;

    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return 1;

    pinfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(length);
    if (!GetLogicalProcessorInformation(pinfo, &length))
    {
        free(pinfo);
        return 1;
    }

    ptr = pinfo;
    while (offset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= length)
    {
        if (ptr->Relationship == RelationProcessorCore)
        {
            count++;
        }

        offset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    free(pinfo);
    return (count >= 1) ? count : 1;
}

class PoolThread : public Runnable
{
private:

    ThreadPool *pool;
    Thread *thread;
    Runnable *work;
    Event event;

    bool auto_delete;
    bool leave;

public:

    PoolThread(ThreadPool *pool, bool auto_delete, unsigned int stack_size) : pool(pool), auto_delete(auto_delete)
    {
        thread = new Thread(this, false, stack_size);
        work = nullptr;
        leave = false;
        thread->Start();
    }

    ~PoolThread()
    {
        delete thread;
    }

    virtual uint32_t Run() override
    {
        while (!leave)
        {
            event.Wait();

            if (!leave && work)
                work->Run();

            if (work)
            {
                Runnable *temp = work;
                work = nullptr;
                pool->WorkDone(temp);

                if (auto_delete)
                    delete temp;
            }
        }

        return 0;
    }

    bool IsFree()
    {
        return !work;
    }

    void AddWork(Runnable *work)
    {
        this->work = work;
        event.Notify();
    }

    void Kill()
    {
        leave = true;
        event.Notify();
        thread->Kill();
    }
};

ThreadPool::ThreadPool(int num_threads, bool auto_delete, unsigned int stack_size)
{
    if (num_threads <= 0)
        num_threads = Thread::LogicalCoresCount();

    threads.resize(num_threads);

    for (PoolThread *&thread : threads)
    {
        thread = new PoolThread(this, auto_delete, stack_size);
    }
}

ThreadPool::~ThreadPool()
{
    Wait();

    for (PoolThread *&thread : threads)
    {
        thread->Kill();
        delete thread;
    }
}

void ThreadPool::ProcessQueue()
{
    for (size_t i = 0; i < works.size(); i++)
    {
        if (running[i])
            continue;

        Runnable *&work = works[i];

        for (PoolThread *&thread : threads)
        {
            if (thread->IsFree())
            {
                running[i] = true;
                thread->AddWork(work);
                break;
            }
        }
    }

    if (works.size() == 0)
        completion.Notify();
}

void ThreadPool::AddWork(Runnable *work)
{
    MutexLocker lock(&mutex);
    works.push_back(work);
    running.push_back(false);
    ProcessQueue();
}

void ThreadPool::Wait()
{
    {
        MutexLocker lock(&mutex);

        if (works.size() == 0)
            return;
    }

    completion.Wait();
}

void ThreadPool::WorkDone(Runnable *work)
{
    MutexLocker lock(&mutex);

    for (size_t i = 0; i < works.size(); i++)
    {
        Runnable *&this_work = works[i];

        if (running[i] && this_work == work)
        {
            running.erase(running.begin()+i);
            works.erase(works.begin()+i);
            break;
        }
    }

    ProcessQueue();
}
