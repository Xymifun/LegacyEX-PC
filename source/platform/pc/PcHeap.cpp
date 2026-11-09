#ifdef _WIN32

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* gHeapStart = 0;
char* gHeapEnd = 0;
char* gHeapTop = 0;
#if __SIZEOF_POINTER__ == 8
char* gLowHeapStart = 0;
char* gLowHeapEnd = 0;
extern "C" void PcCreateLowHeap(void* base, size_t size);
#endif

namespace
{
const size_t kMegabyte = static_cast<size_t>(1024) * 1024;
#if __SIZEOF_POINTER__ == 8
const size_t kArenaSizes[] = {16384 * kMegabyte, 8192 * kMegabyte, 4096 * kMegabyte, 2048 * kMegabyte,
                              1536 * kMegabyte, 1024 * kMegabyte, 512 * kMegabyte, 384 * kMegabyte};
const DWORD kArenaPlacement = MEM_TOP_DOWN;
const size_t kLowHeapSize = 256 * kMegabyte;
const size_t kLowHeapLimit = static_cast<size_t>(4096) * kMegabyte;
#else
const size_t kArenaSizes[] = {1536 * kMegabyte, 1024 * kMegabyte, 768 * kMegabyte, 512 * kMegabyte, 384 * kMegabyte};
const DWORD kArenaPlacement = 0;
#endif
const size_t kCommitStep = 32 * kMegabyte;
const size_t kInitialCommit = 1024 * kMegabyte;
char* s_committedEnd = 0;

void StartupLog(const char* text, bool truncate)
{
    char path[MAX_PATH];
    DWORD length = GetModuleFileNameA(0, path, MAX_PATH);
    while (length > 0 && path[length - 1] != '\\' && path[length - 1] != '/')
        --length;
    if (length + sizeof("startup.log") > MAX_PATH)
        return;
    strcpy(path + length, "startup.log");
    HANDLE file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, 0, truncate ? CREATE_ALWAYS : OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE)
        return;
    SetFilePointer(file, 0, 0, FILE_END);
    DWORD written = 0;
    WriteFile(file, text, static_cast<DWORD>(strlen(text)), &written, 0);
    WriteFile(file, "\r\n", 2, &written, 0);
    CloseHandle(file);
}

bool ReadHeapSettings(size_t* megabytes, bool* commitAll)
{
    char path[MAX_PATH];
    DWORD length = GetModuleFileNameA(0, path, MAX_PATH);
    while (length > 0 && path[length - 1] != '\\' && path[length - 1] != '/')
        --length;
    if (length + sizeof("heap.txt") > MAX_PATH)
        return false;
    strcpy(path + length, "heap.txt");
    char text[128];
    const DWORD fromEnvironment = GetEnvironmentVariableA("MCLCE_HEAP", text, sizeof(text));
    if (fromEnvironment == 0 || fromEnvironment >= sizeof(text))
    {
        FILE* file = fopen(path, "r");
        if (file == 0)
            return false;
        const size_t read = fread(text, 1, sizeof(text) - 1, file);
        fclose(file);
        text[read] = 0;
    }
    *megabytes = static_cast<size_t>(strtoul(text + strcspn(text, "0123456789"), 0, 10));
    *commitAll = strstr(text, "commit") != 0;
    return true;
}

void LogAddressSpace()
{
    MEMORY_BASIC_INFORMATION info;
    size_t freeTotal = 0;
    size_t freeLargest = 0;
    char* address = 0;
    while (VirtualQuery(address, &info, sizeof(info)) == sizeof(info))
    {
        if (info.State == MEM_FREE)
        {
            freeTotal += info.RegionSize;
            if (info.RegionSize > freeLargest)
                freeLargest = info.RegionSize;
        }
        char* next = static_cast<char*>(info.BaseAddress) + info.RegionSize;
        if (next <= address)
            break;
        address = next;
    }
    char line[128];
    snprintf(line, sizeof(line), "address space: %u MB free after the heap, largest free block %u MB",
             static_cast<unsigned int>(freeTotal / kMegabyte), static_cast<unsigned int>(freeLargest / kMegabyte));
    StartupLog(line, false);
}

#if __SIZEOF_POINTER__ == 8
void InitialiseLowHeap()
{
    MEMORY_BASIC_INFORMATION info;
    char* address = reinterpret_cast<char*>(64 * kMegabyte);
    while (gLowHeapStart == 0 && reinterpret_cast<size_t>(address) + kLowHeapSize <= kLowHeapLimit
           && VirtualQuery(address, &info, sizeof(info)) == sizeof(info))
    {
        char* regionEnd = static_cast<char*>(info.BaseAddress) + info.RegionSize;
        if (info.State == MEM_FREE)
        {
            char* start = reinterpret_cast<char*>((reinterpret_cast<size_t>(address) + 0xffff) & ~static_cast<size_t>(0xffff));
            if (start + kLowHeapSize <= regionEnd)
                gLowHeapStart = static_cast<char*>(VirtualAlloc(start, kLowHeapSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
        }
        if (regionEnd <= address)
            break;
        address = regionEnd;
    }
    char line[128];
    if (gLowHeapStart == 0)
    {
        StartupLog("heap: could not reserve the 256 MB sound heap below 4 GB; stopping", false);
        abort();
    }
    gLowHeapEnd = gLowHeapStart + kLowHeapSize;
    PcCreateLowHeap(gLowHeapStart, kLowHeapSize);
    snprintf(line, sizeof(line), "heap: %u MB sound heap at %p (below 4 GB, for 32-bit Miles handles)",
             static_cast<unsigned int>(kLowHeapSize / kMegabyte), static_cast<void*>(gLowHeapStart));
    StartupLog(line, false);
}
#endif

void InitialiseArena()
{
    if (gHeapStart != 0)
        return;
    size_t requested = 0;
    bool commitAll = false;
    const bool configured = ReadHeapSettings(&requested, &commitAll);
    const DWORD allocation = (commitAll ? MEM_RESERVE | MEM_COMMIT : MEM_RESERVE) | kArenaPlacement;
    if (configured && requested >= 128)
    {
        gHeapStart = static_cast<char*>(VirtualAlloc(0, requested * kMegabyte, allocation, PAGE_READWRITE));
        if (gHeapStart != 0)
            gHeapEnd = gHeapStart + requested * kMegabyte;
    }
    for (size_t index = 0; index < sizeof(kArenaSizes) / sizeof(kArenaSizes[0]) && gHeapStart == 0; ++index)
    {
        gHeapStart = static_cast<char*>(VirtualAlloc(0, kArenaSizes[index], allocation, PAGE_READWRITE));
        if (gHeapStart != 0)
            gHeapEnd = gHeapStart + kArenaSizes[index];
    }
    char line[160];
    if (gHeapStart == 0)
    {
        StartupLog("heap: could not reserve even 384 MB of address space; stopping", true);
        abort();
    }
    snprintf(line, sizeof(line), "heap: %u MB %s at %p%s", static_cast<unsigned int>((gHeapEnd - gHeapStart) / kMegabyte),
             commitAll ? "reserved and committed" : "reserved", static_cast<void*>(gHeapStart),
             configured ? " (heap.txt or MCLCE_HEAP)" : "");
    StartupLog(line, true);
    gHeapTop = gHeapStart;
    s_committedEnd = commitAll ? gHeapEnd : gHeapStart;
    if (!commitAll)
    {
        size_t initial = kInitialCommit;
        if (initial > static_cast<size_t>(gHeapEnd - gHeapStart))
            initial = static_cast<size_t>(gHeapEnd - gHeapStart);
        if (VirtualAlloc(gHeapStart, initial, MEM_COMMIT, PAGE_READWRITE) != 0)
        {
            s_committedEnd = gHeapStart + initial;
            snprintf(line, sizeof(line), "heap: first %u MB committed", static_cast<unsigned int>(initial / kMegabyte));
        }
        else
            snprintf(line, sizeof(line), "heap: committing the first %u MB failed (error %lu)",
                     static_cast<unsigned int>(initial / kMegabyte), static_cast<unsigned long>(GetLastError()));
        StartupLog(line, false);
    }
#if __SIZEOF_POINTER__ == 8
    InitialiseLowHeap();
#endif
    LogAddressSpace();
}

void NTAPI InitialiseArenaAtProcessAttach(PVOID, DWORD reason, PVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        InitialiseArena();
}
}

__attribute__((constructor(101))) static void LogConstructorsStarting()
{
    StartupLog("startup: global constructors starting", false);
}

extern "C" void PcStartupLog(const char* text)
{
    StartupLog(text, false);
}

extern "C" int PcHeapCommit(char* top)
{
    if (top <= s_committedEnd)
        return 1;
    size_t wanted = static_cast<size_t>(top - s_committedEnd);
    wanted = (wanted + kCommitStep - 1) / kCommitStep * kCommitStep;
    if (wanted > static_cast<size_t>(gHeapEnd - s_committedEnd))
        wanted = static_cast<size_t>(gHeapEnd - s_committedEnd);
    if (VirtualAlloc(s_committedEnd, wanted, MEM_COMMIT, PAGE_READWRITE) == 0)
    {
        char line[160];
        snprintf(line, sizeof(line), "heap: committing %u MB at %u MB into the heap failed (error %lu)",
                 static_cast<unsigned int>(wanted / kMegabyte),
                 static_cast<unsigned int>((s_committedEnd - gHeapStart) / kMegabyte),
                 static_cast<unsigned long>(GetLastError()));
        StartupLog(line, false);
        return 0;
    }
    s_committedEnd += wanted;
    return top <= s_committedEnd;
}

extern "C" __attribute__((section(".CRT$XLB"), used)) const PIMAGE_TLS_CALLBACK PcHeapTlsCallback =
    InitialiseArenaAtProcessAttach;

#endif
