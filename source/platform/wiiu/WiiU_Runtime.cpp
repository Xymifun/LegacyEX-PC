#include <cstdlib>

#include <coreinit/filesystem.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/systeminfo.h>
#include <coreinit/time.h>

#include "platform/wiiu/WiiU_Runtime.hpp"
#include "game/core/CriticalSection.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"

FSClient* gFileSystemClient = 0;
FSClient* gFileCacheClient = 0;

void stateChangeCallback(FSClient*, FSVolumeState, void*);

unsigned int FileTimeToSystemTime(const FILETIME* fileTime, SYSTEMTIME* systemTime)
{
    (void)fileTime;
    (void)systemTime;
    return 0;
}

void WiiUInitialise()
{
    FSInit();
    gFileSystemClient = static_cast<FSClient*>(std::malloc(0x1700));
    FSAddClient(gFileSystemClient, FS_ERROR_FLAG_NONE);
    gFileCacheClient = gFileSystemClient;
    FSStateChangeParams notification = {stateChangeCallback, 0, 0};
    FSSetStateChangeNotification(gFileSystemClient, &notification);
    s_writeBuffer = static_cast<unsigned char*>(MEMAllocFromDefaultHeapEx(FILE_CACHE_BUFFER_SIZE, 64));
    InitializeCriticalSection(&s_writeMutex);
    InitializeCriticalSection(&s_commandBlockMutex);
}

#if defined(__POWERPC__) || defined(__powerpc__) || defined(__PPC__)
void __debugbreak()
{
}
#endif

unsigned int GetLastFileErrorCodeForViewer()
{
    return static_cast<unsigned int>(FSGetLastErrorCodeForViewer(gFileSystemClient));
}

unsigned int GetLastError()
{
    return 0;
}

void GlobalMemoryStatus(MEMORYSTATUS* status)
{
    status->dwTotalPhys = 0;
    status->dwAvailPhys = 0;
    status->dwAvailVirtual = 0;
}

void GlobalMemoryStatus(_MEMORYSTATUS* status)
{
    status->dwTotalPhys = 0;
    status->dwAvailPhys = 0;
    status->dwAvailVirtual = 0;
}

unsigned int GetTickCount()
{
    const unsigned long long microseconds = OSGetTime() * 1000000ULL;
    const unsigned long long ticksPerMicrosecond = OSGetSystemInfo()->busClockSpeed >> 2;
    return static_cast<unsigned int>(microseconds / ticksPerMicrosecond / 1000ULL);
}

void GetSystemTime(SYSTEMTIME* systemTime)
{
    OSCalendarTime calendar;
    OSTicksToCalendarTime(OSGetTime(), &calendar);
    systemTime->year = static_cast<unsigned short>(calendar.tm_year);
    systemTime->month = static_cast<unsigned short>(calendar.tm_mon + 1);
    systemTime->dayOfWeek = static_cast<unsigned short>(calendar.tm_wday);
    systemTime->day = static_cast<unsigned short>(calendar.tm_mday);
    systemTime->hour = static_cast<unsigned short>(calendar.tm_hour);
    systemTime->minute = static_cast<unsigned short>(calendar.tm_min);
    systemTime->second = static_cast<unsigned short>(calendar.tm_sec);
    systemTime->milliseconds = static_cast<unsigned short>(calendar.tm_msec);
}

void GetLocalTime(SYSTEMTIME* systemTime)
{
    GetSystemTime(systemTime);
}

void EnterCriticalSection(OSFastMutex* criticalSection)
{
    OSFastMutex_Lock(criticalSection);
}

void LeaveCriticalSection(OSFastMutex* criticalSection)
{
    OSFastMutex_Unlock(criticalSection);
}

void InitializeCriticalSection(OSFastMutex* criticalSection)
{
    OSFastMutex_Init(criticalSection, "");
}

void InitializeCriticalSectionAndSpinCount(OSFastMutex* criticalSection, unsigned long)
{
    InitializeCriticalSection(criticalSection);
}

void DeleteCriticalSection(OSFastMutex*)
{
}

int TryEnterCriticalSection(OSFastMutex* criticalSection)
{
    return OSFastMutex_TryLock(criticalSection);
}
