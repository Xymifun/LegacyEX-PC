#ifndef WIIU_RUNTIME_HPP
#define WIIU_RUNTIME_HPP

#include <coreinit/filesystem.h>

#ifndef ERROR_CANCELLED
#define ERROR_CANCELLED 1223
#endif

#ifndef STILL_ACTIVE
#define STILL_ACTIVE 259
#endif

struct _FILETIME
{
    unsigned int dwLowDateTime;
    unsigned int dwHighDateTime;
};

struct _OVERLAPPED
{
    unsigned int Internal;
    unsigned int InternalHigh;
    unsigned int Offset;
    unsigned int OffsetHigh;
    int hEvent;
};

static const int INVALID_HANDLE_VALUE = -1;
static const unsigned int INVALID_FILE_ATTRIBUTES = 0xFFFFFFFF;
static const unsigned int FILE_ATTRIBUTE_DIRECTORY = 0x00000010;
static const unsigned int FILE_ATTRIBUTE_NORMAL = 0x00000080;
static const unsigned int FILE_FLAG_SEQUENTIAL_SCAN = 0x08000000;
static const unsigned int GENERIC_READ = 0x80000000;
static const unsigned int GENERIC_WRITE = 0x40000000;
static const unsigned int CREATE_NEW = 1;
static const unsigned int CREATE_ALWAYS = 2;
static const unsigned int OPEN_EXISTING = 3;
static const unsigned int OPEN_ALWAYS = 4;
static const unsigned int TRUNCATE_EXISTING = 5;

struct MEMORYSTATUS
{
    unsigned long dwLength;
    unsigned long dwMemoryLoad;
    unsigned long dwTotalPhys;
    unsigned long dwAvailPhys;
    unsigned long dwTotalPageFile;
    unsigned long dwAvailPageFile;
    unsigned long dwTotalVirtual;
    unsigned long dwAvailVirtual;
};

struct _MEMORYSTATUS
{
    unsigned long dwLength;
    unsigned long dwMemoryLoad;
    unsigned long dwTotalPhys;
    unsigned long dwAvailPhys;
    unsigned long dwTotalPageFile;
    unsigned long dwAvailPageFile;
    unsigned long dwTotalVirtual;
    unsigned long dwAvailVirtual;
};

struct _SYSTEMTIME
{
    unsigned short year;
    unsigned short month;
    unsigned short dayOfWeek;
    unsigned short day;
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
    unsigned short milliseconds;
};

typedef _FILETIME FILETIME;
typedef _SYSTEMTIME SYSTEMTIME;

unsigned int GetLastFileErrorCodeForViewer();
unsigned int GetLastError();
void GlobalMemoryStatus(MEMORYSTATUS* status);
void GlobalMemoryStatus(_MEMORYSTATUS* status);
unsigned int GetTickCount();
void GetSystemTime(SYSTEMTIME* systemTime);
void GetLocalTime(SYSTEMTIME* systemTime);
unsigned int FileTimeToSystemTime(const FILETIME* fileTime, SYSTEMTIME* systemTime);

extern FSClient* gFileSystemClient;
extern FSClient* gFileCacheClient;
void WiiUInitialise();

#endif
