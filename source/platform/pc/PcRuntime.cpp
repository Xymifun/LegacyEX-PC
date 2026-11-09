#include "platform/pc/PcOs.hpp"

#include "game/core/CriticalSection.hpp"
#include "game/core/System.hpp"
#include "platform/wiiu/PlatformTiming.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

#include <coreinit/memdefaultheap.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

extern "C" void* memalign(size_t alignment, size_t bytes);
#include <string.h>

namespace
{
const unsigned int kMemRelease = 0x8000;

struct PcCriticalSection
{
    unsigned char lock[kPcOsLockBytes];
    uintptr_t owner;
    int depth;
};

PcCriticalSection* AsCriticalSection(OSFastMutex* cs)
{
    typedef char FitsInOSFastMutex[sizeof(PcCriticalSection) <= sizeof(OSFastMutex) ? 1 : -1];
    (void)sizeof(FitsInOSFastMutex);
    return reinterpret_cast<PcCriticalSection*>(cs);
}

uintptr_t CurrentThreadKey()
{
    static __thread unsigned char s_key;
    return reinterpret_cast<uintptr_t>(&s_key);
}

bool OwnsCriticalSection(PcCriticalSection* cs)
{
    return __atomic_load_n(&cs->depth, __ATOMIC_ACQUIRE) > 0
           && __atomic_load_n(&cs->owner, __ATOMIC_RELAXED) == CurrentThreadKey();
}

unsigned long long MonotonicNanoseconds()
{
    return PcOsMonotonicNanoseconds();
}

void FillSystemTime(long long secs, int ms, bool local, SYSTEMTIME* st)
{
    PcOsCalendar calendar;
    PcOsCalendarFromUnixSeconds(secs, local, &calendar);
    st->year = (unsigned short)calendar.year;
    st->month = (unsigned short)calendar.month;
    st->dayOfWeek = (unsigned short)calendar.dayOfWeek;
    st->day = (unsigned short)calendar.day;
    st->hour = (unsigned short)calendar.hour;
    st->minute = (unsigned short)calendar.minute;
    st->second = (unsigned short)calendar.second;
    st->milliseconds = (unsigned short)ms;
}

void* PcAllocFromDefaultHeapEx(uint32_t size, int32_t alignment)
{
    if (alignment < 0)
        alignment = -alignment;
    if (alignment < static_cast<int32_t>(2 * sizeof(void*)))
        return malloc(size);
    return memalign(static_cast<size_t>(alignment), size);
}

void* PcAllocFromDefaultHeap(uint32_t size)
{
    return malloc(size);
}

void PcFreeToDefaultHeap(void* block)
{
    free(block);
}
}

MEMAllocFromDefaultHeapFn MEMAllocFromDefaultHeap = PcAllocFromDefaultHeap;
MEMAllocFromDefaultHeapExFn MEMAllocFromDefaultHeapEx = PcAllocFromDefaultHeapEx;
MEMFreeToDefaultHeapFn MEMFreeToDefaultHeap = PcFreeToDefaultHeap;

void InitializeCriticalSection(OSFastMutex* cs)
{
    memset(AsCriticalSection(cs), 0, sizeof(PcCriticalSection));
}

void InitializeCriticalSectionAndSpinCount(OSFastMutex* cs, unsigned long)
{
    InitializeCriticalSection(cs);
}

void EnterCriticalSection(OSFastMutex* criticalSection)
{
    PcCriticalSection* cs = AsCriticalSection(criticalSection);
    if (OwnsCriticalSection(cs))
    {
        ++cs->depth;
        return;
    }
    PcOsLockAcquire(cs->lock);
    __atomic_store_n(&cs->owner, CurrentThreadKey(), __ATOMIC_RELAXED);
    __atomic_store_n(&cs->depth, 1, __ATOMIC_RELEASE);
}

int TryEnterCriticalSection(OSFastMutex* criticalSection)
{
    PcCriticalSection* cs = AsCriticalSection(criticalSection);
    if (OwnsCriticalSection(cs))
    {
        ++cs->depth;
        return 1;
    }
    if (!PcOsLockTryAcquire(cs->lock))
        return 0;
    __atomic_store_n(&cs->owner, CurrentThreadKey(), __ATOMIC_RELAXED);
    __atomic_store_n(&cs->depth, 1, __ATOMIC_RELEASE);
    return 1;
}

void LeaveCriticalSection(OSFastMutex* criticalSection)
{
    PcCriticalSection* cs = AsCriticalSection(criticalSection);
    if (!OwnsCriticalSection(cs))
        return;
    if (cs->depth > 1)
    {
        --cs->depth;
        return;
    }
    __atomic_store_n(&cs->owner, 0UL, __ATOMIC_RELAXED);
    __atomic_store_n(&cs->depth, 0, __ATOMIC_RELEASE);
    PcOsLockRelease(cs->lock);
}

void DeleteCriticalSection(OSFastMutex*)
{
}

unsigned int QueryPerformanceCounter(_LARGE_INTEGER* v)
{
    v->QuadPart = (long long)(MonotonicNanoseconds() / 1000ULL);
    return 1;
}

unsigned int QueryPerformanceFrequency(_LARGE_INTEGER* v)
{
    v->QuadPart = 1000000;
    return 1;
}

unsigned int GetTickCount()
{
    static const unsigned long long s_start = MonotonicNanoseconds();
    return (unsigned int)((MonotonicNanoseconds() - s_start) / 1000000ULL);
}

void GetSystemTime(SYSTEMTIME* st)
{
    unsigned long long now = PcOsRealTimeMilliseconds();
    FillSystemTime((long long)(now / 1000ULL), (int)(now % 1000ULL), false, st);
}

void GetLocalTime(SYSTEMTIME* st)
{
    unsigned long long now = PcOsRealTimeMilliseconds();
    FillSystemTime((long long)(now / 1000ULL), (int)(now % 1000ULL), true, st);
}

unsigned int FileTimeToSystemTime(const FILETIME* fileTime, SYSTEMTIME* st)
{
    unsigned long long ticks = ((unsigned long long)fileTime->dwHighDateTime << 32) | fileTime->dwLowDateTime;
    const unsigned long long ticksPerSecond = 10000000ULL;
    const unsigned long long unixEpochInFileTimeSeconds = 11644473600ULL;
    unsigned long long secs = ticks / ticksPerSecond;
    if (secs < unixEpochInFileTimeSeconds)
        return 0;
    FillSystemTime((long long)(secs - unixEpochInFileTimeSeconds),
                   (int)((ticks % ticksPerSecond) / 10000ULL), false, st);
    return 1;
}

unsigned int GetLastError()
{
    return (unsigned int)errno;
}

unsigned int GetLastFileErrorCodeForViewer()
{
    return 0;
}

void GlobalMemoryStatus(MEMORYSTATUS* status)
{
    unsigned long long total;
    unsigned long long available;
    PcOsPhysicalMemory(&total, &available);
    const unsigned long long limit = 0xFFFFFFFFULL;
    status->dwTotalPhys = static_cast<unsigned long>(total < limit ? total : limit);
    status->dwAvailPhys = static_cast<unsigned long>(available < limit ? available : limit);
    status->dwAvailVirtual = status->dwAvailPhys;
}

void GlobalMemoryStatus(_MEMORYSTATUS* status)
{
    GlobalMemoryStatus(reinterpret_cast<MEMORYSTATUS*>(status));
}

void* VirtualAlloc(void* address, unsigned long size, unsigned int, unsigned int)
{
    if (address != 0)
        return address;
    return PcOsAllocPages(0, size);
}

int VirtualFree(void* address, unsigned long size, unsigned int freeType)
{
    if ((freeType & kMemRelease) == 0)
        return 1;
    return PcOsFreePages(address, size) ? 1 : 0;
}

#ifdef _WIN32
#define PC_STRINGIFY_TEXT(text) #text
#define PC_STRINGIFY(text) PC_STRINGIFY_TEXT(text)
#define PC_LABEL_PREFIX PC_STRINGIFY(__USER_LABEL_PREFIX__)
extern "C" void PcIgnoreDebugBreak() asm(PC_LABEL_PREFIX "_Z12__debugbreakv");

extern "C" void PcIgnoreDebugBreak()
{
}
#else
void __debugbreak()
{
}
#endif

void System::arraycopy(arrayWithLength<unsigned char> src, unsigned int srcPos, arrayWithLength<unsigned char>* dest,
                       unsigned int destPos, unsigned int len)
{
    memmove(dest->data + destPos, src.data + srcPos, len);
}

void System::arraycopy(arrayWithLength<Node*> src, unsigned int srcPos, arrayWithLength<Node*>* dest,
                       unsigned int destPos, unsigned int len)
{
    System::arraycopy<Node*>(src, srcPos, dest, destPos, len);
}

void System::arraycopy(arrayWithLength<Biome*> src, unsigned int srcPos, arrayWithLength<Biome*>* dest,
                       unsigned int destPos, unsigned int len)
{
    System::arraycopy<Biome*>(src, srcPos, dest, destPos, len);
}

void System::arraycopy(arrayWithLength<int> src, unsigned int srcPos, arrayWithLength<int>* dest, unsigned int destPos,
                       unsigned int len)
{
    memmove(dest->data + destPos, src.data + srcPos, len * sizeof(int));
}

void System::arraycopy(arrayWithLength<unsigned short> src, unsigned int srcPos, arrayWithLength<unsigned short>* dest,
                       unsigned int destPos, unsigned int len)
{
    memmove(dest->data + destPos, src.data + srcPos, len * sizeof(unsigned short));
}

uint64_t System::processTime(ETimeGranularity granularity)
{
    switch (granularity)
    {
    case eTimeGranularity_MilliSecs:
        return MonotonicNanoseconds() / 1000000ULL;
    case eTimeGranularity_MicroSecs:
        return MonotonicNanoseconds() / 1000ULL;
    case eTimeGranularity_NanoSecs:
        return MonotonicNanoseconds();
    }
    return 0;
}

uint64_t System::processTimeInSeconds()
{
    return processTime(eTimeGranularity_MilliSecs) / 1000;
}

long long System::processTimeInMilliSecs()
{
    return processTime(eTimeGranularity_MilliSecs);
}

uint64_t System::processTimeInNanoSecs()
{
    return processTime(eTimeGranularity_NanoSecs);
}

uint64_t System::currentRealTimeMillis()
{
    return PcOsRealTimeMilliseconds();
}

uint64_t System::currentUnixEpochTimeSeconds()
{
    return currentRealTimeMillis() / 1000;
}

void System::ReverseUSHORT(unsigned short* v)
{
    *v = __builtin_bswap16(*v);
}

void System::ReverseSHORT(short* v)
{
    *v = (short)__builtin_bswap16((unsigned short)(*v));
}

void System::ReverseULONG(unsigned int* v)
{
    *v = __builtin_bswap32(*v);
}

void System::ReverseINT(int* v)
{
    *v = (int)__builtin_bswap32((unsigned int)(*v));
}

void System::ReverseULONGLONG(long long* v)
{
    *v = (long long)__builtin_bswap64((unsigned long long)(*v));
}

void System::ReverseCHAR16A(wchar_t* s, int n)
{
    for (int i = 0; i < n; i++)
        ReverseUSHORT(reinterpret_cast<unsigned short*>(&s[i]));
}
