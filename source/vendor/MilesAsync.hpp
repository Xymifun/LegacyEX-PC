#ifndef SOURCE_VENDOR_MILES_ASYNC_HPP
#define SOURCE_VENDOR_MILES_ASYNC_HPP

#include "vendor/MilesRR.hpp"

struct MilesProfile;

struct MilesProfileSite
{
    unsigned int uSiteState[3];
};

typedef const char* (*MilesProfileDescribeFunc)(MilesProfile* pProfile, void* pObject);
typedef void (*MilesProfileZoneEnterFunc)(MilesProfile* pProfile, unsigned int uZone, unsigned int uParent,
                                          unsigned long long uTimeStamp, unsigned int uFlags, const char* pFile,
                                          int iLine, MilesProfileSite* pSite, const char* pFormat, ...);
typedef void (*MilesProfileZoneLeaveFunc)(MilesProfile* pProfile, unsigned long long uTimeStamp, unsigned int uFlags,
                                          const char* pFile, int iLine);
typedef void (*MilesProfileThreadNameFunc)(MilesProfile* pProfile, unsigned int uThreadId, MilesProfileSite* pSite,
                                           const char* pFormat, ...);
typedef void (*MilesProfileLockNameFunc)(MilesProfile* pProfile, void* pLock, MilesProfileSite* pSite,
                                         const char* pFormat, ...);
typedef void (*MilesProfileLockStateFunc)(MilesProfile* pProfile, void* pLock, unsigned int uState, const char* pFile,
                                          int iLine, MilesProfileSite* pSite, const char* pFormat, ...);
typedef void (*MilesProfileAllocFunc)(MilesProfile* pProfile, void* pBlock, unsigned int uFlags, int iSize,
                                      const char* pFile, int iLine, MilesProfileSite* pSite, const char* pFormat, ...);
typedef void (*MilesProfileFreeFunc)(MilesProfile* pProfile, void* pBlock, const char* pFile, int iLine,
                                     MilesProfileSite* pSite);
typedef unsigned int (*MilesProfileCallStackFunc)(MilesProfile* pProfile, unsigned int uSkip, unsigned int uFlags);
typedef void (*MilesProfileMessageFunc)(MilesProfile* pProfile, unsigned int uContext, unsigned int uFlags,
                                        MilesProfileSite* pSite, const char* pFormat, ...);

typedef void (*MilesProfilePlotFunc)(MilesProfile* pProfile, unsigned long long uContext, unsigned int uType,
                                     unsigned int uFlags, double dValue, MilesProfileSite* pSite, const char* pFormat,
                                     ...);

struct MilesProfile
{
    void* pFunction00;
    void* pFunction04;
    void* pFunction08;
    void* pFunction0C;
    void* pFunction10;
    MilesProfileDescribeFunc pDescribe;
    void* pFunction18;
    void* pFunction1C;
    void* pFunction20;
    void* pFunction24;
    void* pFunction28;
    void* pFunction2C;
    void* pFunction30;
    void* pFunction34;
    void* pFunction38;
    void* pFunction3C;
    void* pFunction40;
    void* pFunction44;
    void* pFunction48;
    void* pFunction4C;
    MilesProfileZoneEnterFunc pZoneEnter;
    MilesProfileZoneLeaveFunc pZoneLeave;
    MilesProfileThreadNameFunc pThreadName;
    MilesProfileLockNameFunc pLockName;
    void* pFunction60;
    void* pFunction64;
    void* pFunction68;
    MilesProfileLockStateFunc pLockState;
    MilesProfileAllocFunc pAlloc;
    MilesProfileFreeFunc pFree;
    void* pFunction78;
    void* pFunction7C;
    void* pFunction80;
    MilesProfilePlotFunc pPlot;
    void* pFunction88;
    void* pFunction8C;
    void* pFunction90;
    void* pFunction94;
    void* pFunction98;
    void* pFunction9C;
    void* pFunctionA0;
    MilesProfileMessageFunc pMessage;
    MilesProfileCallStackFunc pCallStack;
    void* pFunctionAC;
    void* pFunctionB0;
    void* pFunctionB4;
    void* pFunctionB8;
    void* pFunctionBC;
};

extern MilesProfile* MSS_profile;

const int MILES_ASYNC_READ_FIELDS_SIZE = 332;
const int MILES_ASYNC_READ_SIZE = 488;

struct MilesAsyncRead
{
    char szFileName[256];
    long long lOffset;
    int iLengthHigh;
    int iLength;
    void* pBuffer;
    void* pPriorityUserData;
    float (*pPriorityCallback)(void* pUserData);
    const char* pOwnerName;
    int iOwnerLine;
    unsigned int uFileHandle;
    unsigned int uFlags;
    int iBytesRead;
    int iBufferOffset;
    int iStatus;
    MilesAsyncRead* pNext;
    MilesAsyncRead* pPrevious;
    int iSeekDone;
    int iCancel;
    int iBufferOwned;
    unsigned char reserved[MILES_ASYNC_READ_SIZE - MILES_ASYNC_READ_FIELDS_SIZE];
};

extern "C" int MilesAsyncFileRead(MilesAsyncRead* pRead);
extern "C" int MilesAsyncFileCancel(MilesAsyncRead* pRead);
extern "C" int MilesAsyncFileStatus(MilesAsyncRead* pRead, unsigned int uTimeoutMilliseconds);
extern "C" int MilesAsyncStartup();
extern "C" int MilesAsyncShutdown();
extern "C" void MilesAsyncSetPaused(int iPaused);

int MilesAsyncFileRead_default(MilesAsyncRead* pRead);
int MilesAsyncFileCancel_default(MilesAsyncRead* pRead);
int MilesAsyncFileStatus_default(MilesAsyncRead* pRead, unsigned int uTimeoutMilliseconds);
int MilesAsyncStartup_default();
int MilesAsyncShutdown_default();
void MilesAsyncSetPaused_default(int iPaused);

extern rrMutex MilesAsyncQueueMutex;
extern MilesProfileSite MilesAsyncQueueLockSite;
extern MilesProfileSite MilesAsyncQueueUnlockSite;

void LockQueue();
void UnlockQueue();

#endif
