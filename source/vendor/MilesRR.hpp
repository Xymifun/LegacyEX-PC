#ifndef SOURCE_VENDOR_MILES_RR_HPP
#define SOURCE_VENDOR_MILES_RR_HPP

struct rrMutex
{
    unsigned char ucStorage[124];
    void* pNative;
};

struct rrSemaphore
{
    unsigned char ucStorage[124];
    void* pNative;
};

struct rrThread
{
    unsigned char ucStorage[252];
    void* pNative;
};

struct MilesFile
{
    unsigned int uHandle;
    unsigned int uField04;
    unsigned int uField08;
    unsigned int uField0C;
    unsigned int uField10;
    unsigned int uField14;
    unsigned int uField18;
    unsigned int uField1C;
    unsigned int uPosition;
    unsigned int uBaseOffset;
    unsigned int uLength;
    int iDoNotClose;
    unsigned int uSize;
};

void TlsSetValue(int iIndex, void* pValue);
void* TlsGetValue(int iIndex);

extern "C" int Platform_OpenFile(MilesFile* pFile, const char* pFileName);
extern "C" void Platform_CloseFile(MilesFile* pFile);
extern "C" int Platform_ReadFile(MilesFile* pFile, void* pBuffer, unsigned int uBytes);
extern "C" void Platform_SeekFromBeginning(MilesFile* pFile, unsigned int uPosition);

extern "C" void rrAtomicMemoryBarrierFull();

extern "C" unsigned int rrAtomicLoadAcquire32(const volatile unsigned int* pTarget);
extern "C" unsigned long long rrAtomicLoadAcquire64(const volatile unsigned long long* pTarget);
extern "C" void rrAtomicStoreRelease32(volatile unsigned int* pTarget, unsigned int uValue);
extern "C" unsigned int rrAtomicCmpXchg32(volatile unsigned int* pTarget, unsigned int uNewValue,
                                          unsigned int uComparand);
extern "C" unsigned long long rrAtomicCmpXchg64(volatile unsigned long long* pTarget, unsigned long long uNewValue,
                                                unsigned long long uComparand);
extern "C" int rrAtomicAddExchange32(volatile int* pTarget, int iValue);
extern "C" long long rrAtomicAddExchange64(volatile long long* pTarget, long long lValue);

extern "C" void rrMutexCreate(rrMutex* pMutex, int iKind);
extern "C" void rrMutexDestroy(rrMutex* pMutex);
extern "C" void rrMutexLock(rrMutex* pMutex);
extern "C" void rrMutexUnlock(rrMutex* pMutex);
extern "C" int rrMutexLockTimeout(rrMutex* pMutex, unsigned int uTimeoutMilliseconds);

extern "C" int rrSemaphoreCreate(rrSemaphore* pSemaphore, int iInitialCount, int iMaximumCount);
extern "C" void rrSemaphoreDestroy(rrSemaphore* pSemaphore);
extern "C" void rrSemaphoreIncrement(rrSemaphore* pSemaphore, int iCount);
extern "C" int rrSemaphoreDecrementOrWait(rrSemaphore* pSemaphore, unsigned int uTimeoutMilliseconds);

extern "C" void rrThreadSleep(unsigned int uMilliseconds);

extern "C" rrThread* rrThreadCurrent();
extern "C" int rrThreadCreate(rrThread* pThread, int (*pEntry)(void*), int iStackSize, void* pUserData,
                              unsigned int uFlags, const char* pName);
extern "C" void rrThreadSetPriority(rrThread* pThread, unsigned int uPriority);
extern "C" void rrThreadSetCPUCore(rrThread* pThread, unsigned int uCore);
extern "C" int rrThreadWaitDone(rrThread* pThread, unsigned int uTimeoutMilliseconds, unsigned int* puExitCode);
extern "C" void rrThreadCleanUp(rrThread* pThread);

#endif
