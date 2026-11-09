#include "platform/pc/PcCrashHandler.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

#include "game/core/CriticalSection.hpp"
#include "platform/pc/PcEvent.hpp"
#include "platform/pc/PcOs.hpp"

#include <coreinit/debug.h>
#include <coreinit/fastmutex.h>
#include <coreinit/atomic.h>
#include <coreinit/atomic64.h>
#include <coreinit/cache.h>
#include <coreinit/messagequeue.h>
#include <coreinit/mutex.h>
#include <coreinit/semaphore.h>
#include <coreinit/systeminfo.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <deque>
#include <map>
#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace
{
const uint32_t BUS_CLOCK_SPEED = 248625000;
const uint32_t CORE_CLOCK_SPEED = 1243125000;
const long long TIMER_TICKS_PER_SECOND = BUS_CLOCK_SPEED / 4;
const long long SECONDS_FROM1970_TO2000 = 946684800LL;

const int THREAD_SPECIFIC_SLOTS = 16;

struct PcCafeThread
{
    PcCafeThread() : key(0), entry(0), argumentCount(0), argument(0), started(false), result(0), done(true) {}

    OSThread* key;
    int (*entry)(int, void*);
    int argumentCount;
    void* argument;
    pthread_t thread;
    bool started;
    int result;
    PcEvent done;
};

struct PcCafeMessageQueue
{
    PcCafeMessageQueue() : capacity(1) {}

    std::deque<OSMessage> messages;
    size_t capacity;
};

OSFastMutex registrySection;

std::map<OSThread*, PcCafeThread*>& Threads()
{
    static std::map<OSThread*, PcCafeThread*>* s_threads = 0;
    if (s_threads == 0)
        s_threads = new std::map<OSThread*, PcCafeThread*>;
    return *s_threads;
}

std::map<OSMessageQueue*, PcCafeMessageQueue*>& Queues()
{
    static std::map<OSMessageQueue*, PcCafeMessageQueue*>* s_queues = 0;
    if (s_queues == 0)
        s_queues = new std::map<OSMessageQueue*, PcCafeMessageQueue*>;
    return *s_queues;
}

pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueChanged = PTHREAD_COND_INITIALIZER;
__thread PcCafeThread* currentThread = 0;
__thread OSThread* currentOsThread = 0;
__thread void* threadSpecific[THREAD_SPECIFIC_SLOTS];
OSSystemInfo systemInfo;

struct PcCafeSemaphore
{
    int32_t count;
};

std::map<OSSemaphore*, PcCafeSemaphore*>& Semaphores()
{
    static std::map<OSSemaphore*, PcCafeSemaphore*>* s_semaphores = 0;
    if (s_semaphores == 0)
        s_semaphores = new std::map<OSSemaphore*, PcCafeSemaphore*>;
    return *s_semaphores;
}

pthread_mutex_t semaphoreMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t semaphoreChanged = PTHREAD_COND_INITIALIZER;

PcCafeSemaphore* FindSemaphore(OSSemaphore* sem)
{
    std::map<OSSemaphore*, PcCafeSemaphore*>::iterator it = Semaphores().find(sem);
    if (it != Semaphores().end())
        return it->second;
    PcCafeSemaphore* created = new PcCafeSemaphore;
    created->count = 0;
    Semaphores()[sem] = created;
    return created;
}

OSFastMutex* AsFastMutex(OSMutex* mtx)
{
    typedef char SameStorage[sizeof(OSMutex) >= sizeof(OSFastMutex) ? 1 : -1];
    (void)sizeof(SameStorage);
    return reinterpret_cast<OSFastMutex*>(mtx);
}

PcCafeThread* FindThread(OSThread* t)
{
    EnterCriticalSection(&registrySection);
    std::map<OSThread*, PcCafeThread*>::iterator it = Threads().find(t);
    PcCafeThread* found = it != Threads().end() ? it->second : 0;
    LeaveCriticalSection(&registrySection);
    return found;
}

void* RunCafeThread(void* arg)
{
    PcCafeThread* t = static_cast<PcCafeThread*>(arg);
    PcCrashHandlerPrepareThread();
    currentThread = t;
    currentOsThread = t->key;
    t->result = t->entry(t->argumentCount, t->argument);
    t->done.Set();
    return 0;
}

PcCafeMessageQueue* FindQueue(OSMessageQueue* q)
{
    std::map<OSMessageQueue*, PcCafeMessageQueue*>::iterator it = Queues().find(q);
    if (it != Queues().end())
        return it->second;
    PcCafeMessageQueue* created = new PcCafeMessageQueue;
    Queues()[q] = created;
    return created;
}

long long UnixMillisecondsToTicks(unsigned long long unixMilliseconds)
{
    long long ms = (long long)unixMilliseconds - SECONDS_FROM1970_TO2000 * 1000LL;
    return ms / 1000LL * TIMER_TICKS_PER_SECOND + ms % 1000LL * TIMER_TICKS_PER_SECOND / 1000LL;
}
}

uint32_t C4JThreadImpl::OSCreateThread4J(OSThread* t, int (*e)(int, void*), int argc, void* arg,
                                         void*, unsigned int, int, unsigned short)
{
    PcCafeThread* record = new PcCafeThread;
    record->key = t;
    record->entry = e;
    record->argumentCount = argc;
    record->argument = arg;
    EnterCriticalSection(&registrySection);
    std::map<OSThread*, PcCafeThread*>::iterator it = Threads().find(t);
    if (it != Threads().end())
    {
        if (it->second->started)
            pthread_detach(it->second->thread);
        it->second = record;
    }
    else
    {
        Threads()[t] = record;
    }
    LeaveCriticalSection(&registrySection);
    return 1;
}

int32_t OSResumeThread(OSThread* t)
{
    PcCafeThread* record = FindThread(t);
    if (record == 0 || record->started)
        return 0;
    record->started = true;
    record->done.Clear();
    pthread_attr_t attrs;
    pthread_attr_init(&attrs);
    pthread_attr_setstacksize(&attrs, 1024 * 1024);
    pthread_create(&record->thread, &attrs, RunCafeThread, record);
    pthread_attr_destroy(&attrs);
    return 1;
}

BOOL OSJoinThread(OSThread* t, int* threadResult)
{
    PcCafeThread* record = FindThread(t);
    if (record == 0 || !record->started)
        return FALSE;
    record->done.Wait(-1);
    pthread_join(record->thread, 0);
    record->started = false;
    if (threadResult != 0)
        *threadResult = record->result;
    return TRUE;
}

void OSExitThread(int32_t res)
{
    if (currentThread != 0)
    {
        currentThread->result = res;
        currentThread->done.Set();
    }
    pthread_exit(0);
}

void OSSleepTicks(OSTime ticks)
{
    PcOsSleepMilliseconds((unsigned int)(ticks * 1000 / TIMER_TICKS_PER_SECOND));
}

OSTime OSGetTime()
{
    return UnixMillisecondsToTicks(PcOsRealTimeMilliseconds());
}

OSTime OSGetSystemTime()
{
    unsigned long long ns = PcOsMonotonicNanoseconds();
    return static_cast<OSTime>(ns / 1000000000ULL * TIMER_TICKS_PER_SECOND
                               + ns % 1000000000ULL * TIMER_TICKS_PER_SECOND / 1000000000ULL);
}

void OSTicksToCalendarTime(OSTime time, OSCalendarTime* calendarTime)
{
    long long secs = time / TIMER_TICKS_PER_SECOND;
    long long remainder = time % TIMER_TICKS_PER_SECOND;
    if (remainder < 0)
    {
        remainder += TIMER_TICKS_PER_SECOND;
        secs -= 1;
    }
    PcOsCalendar calendar;
    PcOsCalendarFromUnixSeconds(secs + SECONDS_FROM1970_TO2000, false, &calendar);
    calendarTime->tm_sec = calendar.second;
    calendarTime->tm_min = calendar.minute;
    calendarTime->tm_hour = calendar.hour;
    calendarTime->tm_mday = calendar.day;
    calendarTime->tm_mon = calendar.month - 1;
    calendarTime->tm_year = calendar.year;
    calendarTime->tm_wday = calendar.dayOfWeek;
    calendarTime->tm_yday = 0;
    calendarTime->tm_msec = (int32_t)(remainder * 1000 / TIMER_TICKS_PER_SECOND);
    calendarTime->tm_usec = (int32_t)(remainder * 1000000 / TIMER_TICKS_PER_SECOND % 1000);
}

OSSystemInfo* OSGetSystemInfo()
{
    systemInfo.busClockSpeed = BUS_CLOCK_SPEED;
    systemInfo.coreClockSpeed = CORE_CLOCK_SPEED;
    systemInfo.baseTime = 0;
    return &systemInfo;
}

void OSReport(const char* fmt, ...)
{
    va_list arguments;
    va_start(arguments, fmt);
    vfprintf(stdout, fmt, arguments);
    va_end(arguments);
    fflush(stdout);
}

void OSFastMutex_Init(OSFastMutex* mtx, const char*)
{
    InitializeCriticalSection(mtx);
}

void OSFastMutex_Lock(OSFastMutex* mtx)
{
    EnterCriticalSection(mtx);
}

void OSFastMutex_Unlock(OSFastMutex* mtx)
{
    LeaveCriticalSection(mtx);
}

BOOL OSFastMutex_TryLock(OSFastMutex* mtx)
{
    return TryEnterCriticalSection(mtx) != 0 ? TRUE : FALSE;
}

void OSInitMessageQueue(OSMessageQueue* q, OSMessage*, int32_t len)
{
    pthread_mutex_lock(&queueMutex);
    PcCafeMessageQueue* record = FindQueue(q);
    record->messages.clear();
    record->capacity = len > 0 ? (size_t)len : 1;
    pthread_mutex_unlock(&queueMutex);
}

BOOL OSSendMessage(OSMessageQueue* q, OSMessage* msg, OSMessageFlags flags)
{
    pthread_mutex_lock(&queueMutex);
    PcCafeMessageQueue* record = FindQueue(q);
    while (record->messages.size() >= record->capacity)
    {
        if ((flags & OS_MESSAGE_FLAGS_BLOCKING) == 0)
        {
            pthread_mutex_unlock(&queueMutex);
            return FALSE;
        }
        pthread_cond_wait(&queueChanged, &queueMutex);
    }
    if ((flags & OS_MESSAGE_FLAGS_HIGH_PRIORITY) != 0)
        record->messages.push_front(*msg);
    else
        record->messages.push_back(*msg);
    pthread_cond_broadcast(&queueChanged);
    pthread_mutex_unlock(&queueMutex);
    return TRUE;
}

BOOL OSReceiveMessage(OSMessageQueue* q, OSMessage* msg, OSMessageFlags flags)
{
    pthread_mutex_lock(&queueMutex);
    PcCafeMessageQueue* record = FindQueue(q);
    while (record->messages.empty())
    {
        if ((flags & OS_MESSAGE_FLAGS_BLOCKING) == 0)
        {
            pthread_mutex_unlock(&queueMutex);
            return FALSE;
        }
        pthread_cond_wait(&queueChanged, &queueMutex);
    }
    *msg = record->messages.front();
    record->messages.pop_front();
    pthread_cond_broadcast(&queueChanged);
    pthread_mutex_unlock(&queueMutex);
    return TRUE;
}

BOOL OSCreateThread(OSThread* t, OSThreadEntryPointFn e, int32_t argc, char* argv, void* stack,
                    uint32_t stackSize, int32_t priority, OSThreadAttributes attrs)
{
    return C4JThreadImpl::OSCreateThread4J(t, reinterpret_cast<int (*)(int, void*)>(e), argc, argv, stack,
                                           stackSize, priority, attrs) != 0
               ? TRUE
               : FALSE;
}

void OSDetachThread(OSThread* t)
{
    PcCafeThread* record = FindThread(t);
    if (record != 0 && record->started)
    {
        pthread_detach(record->thread);
        record->started = false;
    }
}

OSThread* OSGetCurrentThread()
{
    if (currentOsThread == 0)
    {
        currentOsThread = new OSThread;
        memset(currentOsThread, 0, sizeof(OSThread));
    }
    return currentOsThread;
}

void* OSGetThreadSpecific(OSThreadSpecificID id)
{
    return (int)id < THREAD_SPECIFIC_SLOTS ? threadSpecific[id] : 0;
}

void OSSetThreadSpecific(OSThreadSpecificID id, void* v)
{
    if ((int)id < THREAD_SPECIFIC_SLOTS)
        threadSpecific[id] = v;
}

int32_t OSGetThreadPriority(OSThread*)
{
    return 16;
}

BOOL OSSetThreadPriority(OSThread*, int32_t)
{
    return TRUE;
}

BOOL OSSetThreadAffinity(OSThread*, uint32_t)
{
    return TRUE;
}

void OSYieldThread()
{
    sched_yield();
}

void OSMemoryBarrier()
{
    __sync_synchronize();
}

int32_t OSAddAtomic(volatile int32_t* ptr, int32_t v)
{
    return __sync_fetch_and_add(ptr, v);
}

int64_t OSAddAtomic64(volatile int64_t* ptr, int64_t v)
{
    return __sync_fetch_and_add(ptr, v);
}

BOOL OSCompareAndSwapAtomicEx(volatile uint32_t* ptr, uint32_t compare, uint32_t v, uint32_t* old)
{
    uint32_t prev = __sync_val_compare_and_swap(ptr, compare, v);
    if (old != 0)
        *old = prev;
    return prev == compare ? TRUE : FALSE;
}

BOOL OSCompareAndSwapAtomicEx64(volatile uint64_t* ptr, uint64_t compare, uint64_t v, uint64_t* old)
{
    uint64_t prev = __sync_val_compare_and_swap(ptr, compare, v);
    if (old != 0)
        *old = prev;
    return prev == compare ? TRUE : FALSE;
}

void OSInitMutex(OSMutex* mtx)
{
    InitializeCriticalSection(AsFastMutex(mtx));
}

void OSLockMutex(OSMutex* mtx)
{
    EnterCriticalSection(AsFastMutex(mtx));
}

void OSUnlockMutex(OSMutex* mtx)
{
    LeaveCriticalSection(AsFastMutex(mtx));
}

BOOL OSTryLockMutex(OSMutex* mtx)
{
    return TryEnterCriticalSection(AsFastMutex(mtx)) != 0 ? TRUE : FALSE;
}

void OSInitSemaphore(OSSemaphore* sem, int32_t n)
{
    pthread_mutex_lock(&semaphoreMutex);
    FindSemaphore(sem)->count = n;
    pthread_mutex_unlock(&semaphoreMutex);
}

int32_t OSWaitSemaphore(OSSemaphore* sem)
{
    pthread_mutex_lock(&semaphoreMutex);
    PcCafeSemaphore* record = FindSemaphore(sem);
    while (record->count <= 0)
        pthread_cond_wait(&semaphoreChanged, &semaphoreMutex);
    int32_t prev = record->count--;
    pthread_mutex_unlock(&semaphoreMutex);
    return prev;
}

int32_t OSTryWaitSemaphore(OSSemaphore* sem)
{
    pthread_mutex_lock(&semaphoreMutex);
    PcCafeSemaphore* record = FindSemaphore(sem);
    int32_t prev = record->count;
    if (prev > 0)
        --record->count;
    pthread_mutex_unlock(&semaphoreMutex);
    return prev;
}

int32_t OSSignalSemaphore(OSSemaphore* sem)
{
    pthread_mutex_lock(&semaphoreMutex);
    PcCafeSemaphore* record = FindSemaphore(sem);
    int32_t prev = record->count++;
    pthread_cond_broadcast(&semaphoreChanged);
    pthread_mutex_unlock(&semaphoreMutex);
    return prev;
}

OSTick OSGetTick()
{
    return static_cast<OSTick>(OSGetSystemTime());
}

void DCFlushRangeNoSync(void*, uint32_t)
{
}
