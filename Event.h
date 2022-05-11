#ifndef __EVENT_H__
#define __EVENT_H__

#include <windows.h>
#include "Utils.h"

class Event
{
private:

    HANDLE handle;

public:

    Event(bool manual_reset=false)
    {
        handle = CreateEvent(nullptr, manual_reset, FALSE, nullptr);
    }

    ~Event()
    {
        CloseHandle(handle);
    }

    inline void Notify()
    {
        SetEvent(handle);
    }

    inline bool Wait()
    {
        return (WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0);
    }

    inline void Reset()
    {
        ResetEvent(handle);
    }
};

#endif // __EVENT_H__
