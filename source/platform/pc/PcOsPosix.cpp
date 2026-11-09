#ifndef _WIN32

#include "platform/pc/PcOs.hpp"

#include <stdlib.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

unsigned long long PcOsMonotonicNanoseconds()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return static_cast<unsigned long long>(now.tv_sec) * 1000000000ULL + static_cast<unsigned long long>(now.tv_nsec);
}

unsigned long long PcOsRealTimeMilliseconds()
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return static_cast<unsigned long long>(now.tv_sec) * 1000ULL + static_cast<unsigned long long>(now.tv_nsec / 1000000L);
}

void PcOsCalendarFromUnixSeconds(long long seconds, bool local, PcOsCalendar* calendar)
{
    time_t value = static_cast<time_t>(seconds);
    struct tm parts;
    if (local)
        localtime_r(&value, &parts);
    else
        gmtime_r(&value, &parts);
    calendar->year = parts.tm_year + 1900;
    calendar->month = parts.tm_mon + 1;
    calendar->dayOfWeek = parts.tm_wday;
    calendar->day = parts.tm_mday;
    calendar->hour = parts.tm_hour;
    calendar->minute = parts.tm_min;
    calendar->second = parts.tm_sec;
}

[[noreturn]] void PcOsExitImmediately(int code)
{
    fflush(NULL);
    _exit(code);
}

std::string PcOsUserDataDirectory()
{
    const char* dataHome = getenv("XDG_DATA_HOME");
    if (dataHome != 0 && dataHome[0] != 0)
        return std::string(dataHome) + "/MinecraftWiiU-PC";
    const char* home = getenv("HOME");
    if (home == 0 || home[0] == 0)
        return std::string();
    return std::string(home) + "/.local/share/MinecraftWiiU-PC";
}

void PcOsUsePreciseTimers()
{
}

void PcOsPrepareOutput(bool)
{
}

void PcOsSleepMilliseconds(unsigned int milliseconds)
{
    struct timespec duration;
    duration.tv_sec = milliseconds / 1000;
    duration.tv_nsec = static_cast<long>(milliseconds % 1000) * 1000000L;
    while (nanosleep(&duration, &duration) == -1 && errno == EINTR)
    {
    }
}

void* PcOsAllocPages(void* address, size_t size)
{
    void* block = mmap(address, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return block == MAP_FAILED ? 0 : block;
}

bool PcOsFreePages(void* address, size_t size)
{
    return size != 0 && munmap(address, size) == 0;
}

void PcOsPhysicalMemory(unsigned long long* total, unsigned long long* available)
{
    unsigned long long pageSize = static_cast<unsigned long long>(sysconf(_SC_PAGESIZE));
    *total = static_cast<unsigned long long>(sysconf(_SC_PHYS_PAGES)) * pageSize;
    *available = static_cast<unsigned long long>(sysconf(_SC_AVPHYS_PAGES)) * pageSize;
}

uintptr_t PcOsProgramBase()
{
    return 0;
}

void PcOsSetCurrentThreadName(const char* name)
{
    char shortName[16];
    size_t length = 0;
    while (name[length] != '\0' && length < sizeof(shortName) - 1)
    {
        shortName[length] = name[length];
        ++length;
    }
    shortName[length] = '\0';
    pthread_setname_np(pthread_self(), shortName);
}

clockid_t PcOsConditionClock()
{
    return CLOCK_MONOTONIC;
}

void PcOsLockAcquire(void* lock)
{
    typedef char FitsInLockStorage[sizeof(pthread_mutex_t) <= kPcOsLockBytes ? 1 : -1];
    (void)sizeof(FitsInLockStorage);
    pthread_mutex_lock(static_cast<pthread_mutex_t*>(lock));
}

bool PcOsLockTryAcquire(void* lock)
{
    return pthread_mutex_trylock(static_cast<pthread_mutex_t*>(lock)) == 0;
}

void PcOsLockRelease(void* lock)
{
    pthread_mutex_unlock(static_cast<pthread_mutex_t*>(lock));
}

bool PcOsMakeDirectory(const char* path)
{
    return mkdir(path, 0755) == 0 || errno == EEXIST;
}

bool PcOsReplaceFile(const char* from, const char* to)
{
    return rename(from, to) == 0;
}

bool PcOsRemoveFile(const char* path)
{
    return unlink(path) == 0 || errno == ENOENT;
}

bool PcOsRemoveDirectory(const char* path)
{
    return rmdir(path) == 0;
}

namespace
{
void (*s_stopHandler)() = NULL;

void StopSignal(int)
{
    if (s_stopHandler != NULL)
        s_stopHandler();
}
}

void PcOsInstallStopHandler(void (*handler)())
{
    s_stopHandler = handler;
    signal(SIGINT, StopSignal);
    signal(SIGTERM, StopSignal);
}

bool PcOsFileSystemIsCaseSensitive()
{
    return true;
}

bool PcOsRandomBytes(void* buffer, size_t size)
{
    FILE* file = fopen("/dev/urandom", "rb");
    if (file == 0)
        return false;
    const bool read = fread(buffer, 1, size, file) == size;
    fclose(file);
    return read;
}

#endif
