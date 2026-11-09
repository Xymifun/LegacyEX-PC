#ifndef PLATFORM_WIIU_CAFE_THREAD_LOCAL_HPP
#define PLATFORM_WIIU_CAFE_THREAD_LOCAL_HPP

#include <coreinit/mutex.h>
#include <coreinit/thread.h>
#include <map>

namespace CafeThreadLocal
{

inline OSMutex& mutex()
{
    struct Mutex
    {
        Mutex() { OSInitMutex(&value); }

        OSMutex value;
    };

    static Mutex mutex;
    return mutex.value;
}

template <typename T>
T& slot(std::map<OSThread*, T>& values)
{
    OSThread* thread = OSGetCurrentThread();
    OSLockMutex(&mutex());
    T& entry = values[thread];
    OSUnlockMutex(&mutex());
    return entry;
}

}

#endif
