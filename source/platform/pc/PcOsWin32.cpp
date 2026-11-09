#ifdef _WIN32

#include "platform/pc/PcOs.hpp"

#include <windows.h>
#include <bcrypt.h>
#include <mmsystem.h>

#include <direct.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long long PcOsMonotonicNanoseconds()
{
    static LARGE_INTEGER s_frequency;
    if (s_frequency.QuadPart == 0)
        QueryPerformanceFrequency(&s_frequency);
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    unsigned long long ticks = (unsigned long long)counter.QuadPart;
    unsigned long long frequency = (unsigned long long)s_frequency.QuadPart;
    return ticks / frequency * 1000000000ULL + ticks % frequency * 1000000000ULL / frequency;
}

unsigned long long PcOsRealTimeMilliseconds()
{
    FILETIME fileTime;
    GetSystemTimeAsFileTime(&fileTime);
    unsigned long long ticks = ((unsigned long long)fileTime.dwHighDateTime << 32) | fileTime.dwLowDateTime;
    return ticks / 10000ULL - 11644473600000ULL;
}

void PcOsCalendarFromUnixSeconds(long long secs, bool local, PcOsCalendar* calendar)
{
    __time64_t v = static_cast<__time64_t>(secs);
    struct tm parts;
    if (local)
        _localtime64_s(&parts, &v);
    else
        _gmtime64_s(&parts, &v);
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
    ::TerminateProcess(::GetCurrentProcess(), (UINT)code);
    for (;;)
        ::Sleep(INFINITE);
}

void PcOsSleepMilliseconds(unsigned int ms)
{
    ::Sleep(ms);
}

std::string PcOsUserDataDirectory()
{
    const char* appData = getenv("APPDATA");
    if (appData == 0 || appData[0] == 0)
        return std::string();
    return std::string(appData) + "\\MinecraftWiiU-PC";
}

void PcOsUsePreciseTimers()
{
    timeBeginPeriod(1);
}

void PcOsPrepareOutput(bool ownWindow)
{
    if (ownWindow)
    {
        FreeConsole();
        if (AllocConsole())
        {
            SetConsoleTitleA("Minecraft LCE PC - output");
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
            SetStdHandle(STD_OUTPUT_HANDLE, reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(stdout))));
            SetStdHandle(STD_ERROR_HANDLE, reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(stderr))));
            return;
        }
    }
    HWND console = GetConsoleWindow();
    DWORD consoleProcesses[2];
    const bool ownConsole = console != 0 && GetConsoleProcessList(consoleProcesses, 2) == 1;
    if (ownConsole)
        ShowWindow(console, SW_HIDE);
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD outputType = output != 0 && output != INVALID_HANDLE_VALUE ? GetFileType(output) : FILE_TYPE_UNKNOWN;
    const bool captured = outputType == FILE_TYPE_PIPE || outputType == FILE_TYPE_DISK;
    if (captured || (!ownConsole && outputType == FILE_TYPE_CHAR))
        return;
    if (!ownConsole && AttachConsole(ATTACH_PARENT_PROCESS))
    {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
    else
    {
        char path[MAX_PATH];
        DWORD length = GetModuleFileNameA(0, path, MAX_PATH);
        while (length > 0 && path[length - 1] != '\\' && path[length - 1] != '/')
            --length;
        if (length + sizeof("latest.log") > MAX_PATH)
            return;
        strcpy(path + length, "latest.log");
        if (freopen(path, "w", stdout) == 0)
            return;
        _dup2(_fileno(stdout), _fileno(stderr));
    }
    SetStdHandle(STD_OUTPUT_HANDLE, reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(stdout))));
    SetStdHandle(STD_ERROR_HANDLE, reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(stderr))));
}

void* PcOsAllocPages(void* address, size_t size)
{
    return VirtualAlloc(address, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

bool PcOsFreePages(void* address, size_t)
{
    return VirtualFree(address, 0, MEM_RELEASE) != 0;
}

void PcOsPhysicalMemory(unsigned long long* total, unsigned long long* available)
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    *total = status.ullTotalPhys;
    *available = status.ullAvailPhys;
}

uintptr_t PcOsProgramBase()
{
    return reinterpret_cast<uintptr_t>(GetModuleHandleW(0));
}

void PcOsSetCurrentThreadName(const char*)
{
}

clockid_t PcOsConditionClock()
{
    return CLOCK_REALTIME;
}

void PcOsLockAcquire(void* lock)
{
    typedef char FitsInLockStorage[sizeof(SRWLOCK) <= kPcOsLockBytes ? 1 : -1];
    (void)sizeof(FitsInLockStorage);
    AcquireSRWLockExclusive(static_cast<PSRWLOCK>(lock));
}

bool PcOsLockTryAcquire(void* lock)
{
    return TryAcquireSRWLockExclusive(static_cast<PSRWLOCK>(lock)) != 0;
}

void PcOsLockRelease(void* lock)
{
    ReleaseSRWLockExclusive(static_cast<PSRWLOCK>(lock));
}

bool PcOsMakeDirectory(const char* path)
{
    return _mkdir(path) == 0 || errno == EEXIST;
}

bool PcOsReplaceFile(const char* from, const char* to)
{
    return MoveFileExA(from, to, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
}

bool PcOsRemoveFile(const char* path)
{
    return DeleteFileA(path) != 0 || GetLastError() == ERROR_FILE_NOT_FOUND;
}

bool PcOsRemoveDirectory(const char* path)
{
    return RemoveDirectoryA(path) != 0;
}

namespace
{
void (*s_stopHandler)() = NULL;

BOOL WINAPI ConsoleControl(DWORD ev)
{
    if (s_stopHandler == NULL)
        return FALSE;
    if (ev == CTRL_C_EVENT || ev == CTRL_BREAK_EVENT)
    {
        s_stopHandler();
        return TRUE;
    }
    if (ev == CTRL_CLOSE_EVENT || ev == CTRL_SHUTDOWN_EVENT || ev == CTRL_LOGOFF_EVENT)
    {
        s_stopHandler();
        Sleep(10000);
        return TRUE;
    }
    return FALSE;
}
}

void PcOsInstallStopHandler(void (*handler)())
{
    s_stopHandler = handler;
    SetConsoleCtrlHandler(ConsoleControl, TRUE);
}

bool PcOsFileSystemIsCaseSensitive()
{
    return false;
}

bool PcOsRandomBytes(void* buf, size_t len)
{
    return BCryptGenRandom(NULL, static_cast<PUCHAR>(buf), static_cast<ULONG>(len),
                           BCRYPT_USE_SYSTEM_PREFERRED_RNG)
           == 0;
}

#endif
