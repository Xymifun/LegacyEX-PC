#include "vendor/MilesRR.hpp"

#include <stddef.h>
#include <string.h>

#ifdef __ghs__
#include <ppc_ghs.h>
#endif

#include <coreinit/atomic.h>
#include <coreinit/atomic64.h>
#include <coreinit/cache.h>
#include <coreinit/filesystem.h>
#include <coreinit/memdefaultheap.h>
#include <coreinit/mutex.h>
#include <coreinit/semaphore.h>
#include <coreinit/systeminfo.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>

struct rriMutex
{
    OSMutex mutex;
    unsigned int uCreated;
};

struct rriSemaphore
{
    OSSemaphore semaphore;
    unsigned int uCreated;
};

struct rriThread
{
    unsigned long long uId;
    int (*pEntry)(void*);
    void* pUserData;
    const char* pName;
    void* pField14;
    void* pField18;
    void* pField1C;
    void* pField20;
    void* pField24;
    void* pField28;
    void* pField2C;
    void* pField30;
    OSThread* pOSThread;
    unsigned int uFlags;
    unsigned int uState;
    unsigned int uExitCode;
    unsigned int uSlot;
    unsigned char* pStack;
};

static const unsigned int RR_SEMAPHORE_CREATED = 0x231D;
static const unsigned int RR_SPIN_SLEEP_MICROSECONDS = 250;
static const unsigned int RR_WAIT_INFINITE = 0xFFFFFFFF;
static const unsigned int RR_DEFAULT_SLEEP_MILLISECONDS = 10000;

static const int RR_TLS_THREAD = 0;
static const int RR_TLS_SIGNATURE = 1;
static const unsigned int RR_TLS_SIGNATURE_VALUE = 0x56781239;

static const unsigned int RR_THREAD_FLAG_CREATE_SUSPENDED = 1;
static const unsigned int RR_THREAD_FLAG_USER_MEMORY = 8;

static const unsigned int RR_THREAD_STATE_RUNNING = 1;
static const unsigned int RR_THREAD_STATE_RUNNING_DETACHED = 3;

static const unsigned int RR_THREAD_EXIT_CODE_JOINED = 0x80000000;

static const unsigned int RR_THREAD_SLOT_WORDS = 2;
static const unsigned int RR_THREAD_NATIVE_OFFSET = 0x120;
static const unsigned int RR_THREAD_STACK_ALIGNMENT = 32;

static const unsigned int RR_THREAD_PRIORITY_LOWEST = 0xFFFFFFFE;
static const unsigned int RR_THREAD_PRIORITY_LOW = 0xFFFFFFFF;
static const unsigned int RR_THREAD_PRIORITY_NORMAL = 0;
static const unsigned int RR_THREAD_PRIORITY_HIGH = 1;
static const unsigned int RR_THREAD_PRIORITY_HIGHEST = 2;
static const unsigned int RR_THREAD_PRIORITY_RAISE = 0x10;
static const unsigned int RR_THREAD_PRIORITY_LOWER = 0x11;
static const unsigned int RR_THREAD_PRIORITY_CALLER = 0x12;
static const unsigned int RR_THREAD_PRIORITY_RAISE_RELATIVE = 0x13;
static const unsigned int RR_THREAD_PRIORITY_LOWER_RELATIVE = 0x14;

static const unsigned int RR_FILE_READ_ALIGNMENT = 64;

static const char RR_FILE_OPEN_MODE[] = "r";

static const unsigned char rrThreadCoreAffinity[3] = {0x01, 0x02, 0x04};
static const unsigned char rrThreadPriorityLevel[5] = {0x1F, 0x18, 0x10, 0x08, 0x00};

static long long rrThreadIdCounter;
static unsigned int rrThreadSlots[RR_THREAD_SLOT_WORDS];
static int rrThreadCount;

FSClient* MilesFsClient;
FSCmdBlock* MilesFsCmdBlock;

#ifdef __ghs__
static inline void FullBarrier()
{
    __SYNC();
}

static inline void AcquireBarrier()
{
    __LWSYNC();
}

static inline int AddAtomic(volatile int* pTarget, int iValue)
{
    unsigned int uPrevious;

    do
    {
        uPrevious = __LWARX((unsigned int*)pTarget, 0);
        __DCBST((unsigned int*)pTarget, 0);
    } while (!__STWCX((unsigned int*)pTarget, 0, uPrevious + iValue));

    return (int)uPrevious;
}

static inline unsigned int CountLeadingZeros(unsigned int uValue)
{
    return __CLZ32(uValue);
}
#elif defined(__powerpc__) || defined(__PPC__)
static inline void FullBarrier()
{
    __asm__ __volatile__("sync" : : : "memory");
}

static inline void AcquireBarrier()
{
    __asm__ __volatile__("lwsync" : : : "memory");
}

static inline int AddAtomic(volatile int* pTarget, int iValue)
{
    int iPrevious;
    int iNext;
    __asm__ __volatile__("1:\n\tlwarx %0, 0, %2\n\tdcbst 0, %2\n\tadd %1, %0, %3\n\tstwcx. %1, 0, %2\n\tbne- 1b"
                         : "=&r"(iPrevious), "=&r"(iNext)
                         : "r"(pTarget), "r"(iValue)
                         : "cr0", "memory");
    return iPrevious;
}

static inline unsigned int CountLeadingZeros(unsigned int uValue)
{
    unsigned int uCount;
    __asm__("cntlzw %0, %1" : "=r"(uCount) : "r"(uValue));
    return uCount;
}
#else
static inline void FullBarrier()
{
    OSMemoryBarrier();
}

static inline void AcquireBarrier()
{
    OSMemoryBarrier();
}

static inline int AddAtomic(volatile int* pTarget, int iValue)
{
    return OSAddAtomic(pTarget, iValue);
}

static inline unsigned int CountLeadingZeros(unsigned int uValue)
{
    return uValue == 0 ? 32 : (unsigned int)__builtin_clz(uValue);
}
#endif

static rriMutex* AlignMutexStorage(rrMutex* pMutex)
{
    return (rriMutex*)(((size_t)pMutex->ucStorage + 15) & ~(size_t)15);
}

static rriSemaphore* AlignSemaphoreStorage(rrSemaphore* pSemaphore)
{
    return (rriSemaphore*)(((size_t)pSemaphore->ucStorage + 15) & ~(size_t)15);
}

extern "C" int Platform_OpenFile(MilesFile* pFile, const char* pFileName)
{
    FSFileHandle uHandle;
    FSStat stat;

    if (MilesFsClient == NULL)
    {
        return 0;
    }

    if (FSOpenFile(MilesFsClient, MilesFsCmdBlock, pFileName, RR_FILE_OPEN_MODE, &uHandle, FS_ERROR_FLAG_NOT_FOUND)
        != 0)
    {
        return 0;
    }

    if (FSGetStatFile(MilesFsClient, MilesFsCmdBlock, uHandle, &stat, FS_ERROR_FLAG_NONE) != 0)
    {
        return 0;
    }

    pFile->uSize = stat.size;
    pFile->uHandle = uHandle;
    return 1;
}

extern "C" void Platform_CloseFile(MilesFile* pFile)
{
    if (MilesFsClient == NULL)
    {
        return;
    }

    FSFileHandle uHandle = pFile->uHandle;
    FSCloseFile(MilesFsClient, MilesFsCmdBlock, uHandle, FS_ERROR_FLAG_NONE);
}

extern "C" int Platform_ReadFile(MilesFile* pFile, void* pBuffer, unsigned int uBytes)
{
    unsigned char ucScratch[RR_FILE_READ_ALIGNMENT + RR_FILE_READ_ALIGNMENT - 1];
    unsigned char* pAligned;
    unsigned char* pBounce;
    FSFileHandle uHandle;
    unsigned int uHead;
    int iRead;

    if (MilesFsClient == NULL)
    {
        return 0;
    }

    pAligned = (unsigned char*)(((size_t)pBuffer + (RR_FILE_READ_ALIGNMENT - 1))
                                & ~(size_t)(RR_FILE_READ_ALIGNMENT - 1));
    pBounce = (unsigned char*)(((size_t)ucScratch + (RR_FILE_READ_ALIGNMENT - 1))
                               & ~(size_t)(RR_FILE_READ_ALIGNMENT - 1));
    uHandle = pFile->uHandle;
    uHead = (unsigned int)(pAligned - (unsigned char*)pBuffer);

    if (uHead != 0)
    {
        if (uHead > uBytes)
        {
            uHead = uBytes;
        }

        iRead = FSReadFile(MilesFsClient, MilesFsCmdBlock, pBounce, 1, uHead, uHandle, 0, FS_ERROR_FLAG_NONE);
        if (iRead < 0)
        {
            return 0;
        }

        memcpy(pBuffer, pBounce, (size_t)iRead);

        if ((unsigned int)iRead != uHead)
        {
            return iRead;
        }

        uBytes = uBytes - uHead;
    }

    if (uBytes == 0)
    {
        return (int)uHead;
    }

    iRead = FSReadFile(MilesFsClient, MilesFsCmdBlock, pAligned, 1, uBytes, uHandle, 0, FS_ERROR_FLAG_NONE);
    if (iRead < 0)
    {
        return 0;
    }

    return iRead + (int)uHead;
}

extern "C" void Platform_SeekFromBeginning(MilesFile* pFile, unsigned int uPosition)
{
    if (MilesFsClient == NULL)
    {
        return;
    }

    FSSetPosFile(MilesFsClient, MilesFsCmdBlock, pFile->uHandle, uPosition + pFile->uBaseOffset, FS_ERROR_FLAG_NONE);
}

void TlsSetValue(int iIndex, void* pValue)
{
    OSSetThreadSpecific((OSThreadSpecificID)iIndex, pValue);
}

void* TlsGetValue(int iIndex)
{
    return OSGetThreadSpecific((OSThreadSpecificID)iIndex);
}

static rrThread* find_rrthread_in_tls(rrThread* pThread)
{
    if (pThread == NULL)
    {
        if (TlsGetValue(RR_TLS_SIGNATURE) != (void*)RR_TLS_SIGNATURE_VALUE)
        {
            return NULL;
        }

        pThread = (rrThread*)TlsGetValue(RR_TLS_THREAD);
        if (pThread == NULL)
        {
            return NULL;
        }

        if (((rriThread*)pThread->pNative)->pOSThread == NULL)
        {
            return NULL;
        }
    }

    return pThread;
}

extern "C" rrThread* rrThreadCurrent()
{
    return find_rrthread_in_tls(NULL);
}

static inline int AllocateThreadSlot(volatile unsigned int* pSlots)
{
    unsigned int uSlots;
    unsigned int uLeading;

    uSlots = *pSlots;
    while ((uLeading = CountLeadingZeros(~uSlots)) < 32)
    {
        if (rrAtomicCmpXchg32(pSlots, uSlots | (1u << (31 - uLeading)), uSlots) == uSlots)
        {
            return (int)(31 - uLeading);
        }

        uSlots = *pSlots;
    }

    return -1;
}

static void thread_cleanup(rriThread* pNative)
{
    OSThread* pOSThread;
    unsigned int uExitCode;
    unsigned int uSlots;
    rrThread* pCurrent;

#if __SIZEOF_POINTER__ == 8
    pOSThread = pNative->pOSThread;
    uExitCode = pNative->uExitCode;
    pNative->uExitCode = 0xFFFFFFFF;

    if (__sync_val_compare_and_swap(&pNative->pOSThread, pOSThread, (OSThread*)NULL) == pOSThread && pOSThread != NULL)
#else
    unsigned int* puThread = (unsigned int*)&pNative->pOSThread;

    pOSThread = (OSThread*)*puThread;
    uExitCode = pNative->uExitCode;
    pNative->uExitCode = 0xFFFFFFFF;

    if (rrAtomicCmpXchg32(puThread, 0, (unsigned int)pOSThread) == (unsigned int)pOSThread && pOSThread != NULL)
#endif
    {
        rrAtomicAddExchange32(&rrThreadCount, -1);

        do
        {
            uSlots = rrThreadSlots[pNative->uSlot >> 5];
        } while (rrAtomicCmpXchg32(&rrThreadSlots[pNative->uSlot >> 5], uSlots & ~(1 << (pNative->uSlot & 31)), uSlots)
                 != uSlots);

        pCurrent = rrThreadCurrent();
        if (pCurrent != NULL && (rriThread*)pCurrent->pNative == pNative)
        {
            TlsSetValue(RR_TLS_THREAD, NULL);
            TlsSetValue(RR_TLS_SIGNATURE, NULL);
        }

        if (uExitCode == 0)
        {
            OSDetachThread(pNative->pOSThread);
            return;
        }

        if ((pNative->uFlags & RR_THREAD_FLAG_USER_MEMORY) == 0)
        {
            MEMFreeToDefaultHeap(pNative->pStack);
            MEMFreeToDefaultHeap(pOSThread);
        }
    }
}

static int wiiu_shim(int, void* pArgument)
{
    rriThread* pNative;
    unsigned int uState;
    int iResult;

    pNative = (rriThread*)((rrThread*)pArgument)->pNative;

    TlsSetValue(RR_TLS_THREAD, pArgument);
    TlsSetValue(RR_TLS_SIGNATURE, (void*)RR_TLS_SIGNATURE_VALUE);

    iResult = pNative->pEntry(pNative->pUserData);

    do
    {
        uState = pNative->uState;
    } while (rrAtomicCmpXchg32(&pNative->uState, uState & ~RR_THREAD_STATE_RUNNING, uState) != uState);

    if (uState == RR_THREAD_STATE_RUNNING_DETACHED)
    {
        thread_cleanup(pNative);
    }

    TlsSetValue(RR_TLS_THREAD, NULL);
    TlsSetValue(RR_TLS_SIGNATURE, NULL);

    return iResult;
}

extern "C" int rrThreadCreate(rrThread* pThread, int (*pEntry)(void*), int iStackSize, void* pUserData,
                              unsigned int uFlags, const char* pName)
{
    rriThread* pNative;
    long long lId;
    int iSlot;
    unsigned char* pStack;
    unsigned int uStackSize;
    int iCreated;
    unsigned int uSlots;

    pNative = (rriThread*)(((size_t)pThread + 31) & ~(size_t)15);
    pThread->pNative = pNative;

    lId = rrAtomicAddExchange64(&rrThreadIdCounter, 1);

    iSlot = AllocateThreadSlot(&rrThreadSlots[0]);
    if (iSlot < 0)
    {
        iSlot = AllocateThreadSlot(&rrThreadSlots[1]);
        if (iSlot < 0)
        {
            return 0;
        }

        iSlot = iSlot + 32;
    }

    pNative->uId = (unsigned long long)lId;
    pNative->pName = pName;
    pNative->uFlags = uFlags;
    pNative->pEntry = pEntry;
    pNative->uExitCode = 0;
    pNative->pUserData = pUserData;
    pNative->uSlot = (unsigned int)iSlot;
    pNative->uState = RR_THREAD_STATE_RUNNING;

    if ((uFlags & RR_THREAD_FLAG_USER_MEMORY) != 0)
    {
        pNative->pOSThread = (OSThread*)(((size_t)pThread + RR_THREAD_NATIVE_OFFSET)
                                         & ~(size_t)(RR_THREAD_STACK_ALIGNMENT - 1));
        pStack = (unsigned char*)pNative->pOSThread + sizeof(OSThread) + RR_THREAD_STACK_ALIGNMENT;
        pNative->pStack = pStack;
        uStackSize = (unsigned int)(iStackSize + ((size_t)pNative - (size_t)pStack)) & ~(RR_THREAD_STACK_ALIGNMENT - 1);
        iCreated = OSCreateThread(pNative->pOSThread, (OSThreadEntryPointFn)wiiu_shim, 0, (char*)pThread,
                                  pStack + uStackSize, uStackSize, OSGetThreadPriority(OSGetCurrentThread()),
                                  (OSThreadAttributes)0);
    }
    else
    {
        pNative->pOSThread = (OSThread*)MEMAllocFromDefaultHeapEx(sizeof(OSThread), 8);
        pStack = (unsigned char*)MEMAllocFromDefaultHeapEx((unsigned int)iStackSize, 8);
        pNative->pStack = pStack;
        iCreated = OSCreateThread(pNative->pOSThread, (OSThreadEntryPointFn)wiiu_shim, 0, (char*)pThread,
                                  pStack + iStackSize, iStackSize, OSGetThreadPriority(OSGetCurrentThread()),
                                  (OSThreadAttributes)0);
    }

    if (iCreated == 0)
    {
        pNative->uState = 0;
        pNative->uExitCode = 1;

        if ((uFlags & RR_THREAD_FLAG_USER_MEMORY) == 0)
        {
            MEMFreeToDefaultHeap(pNative->pStack);
            MEMFreeToDefaultHeap(pNative->pOSThread);
        }

        pNative->pOSThread = NULL;

        do
        {
            uSlots = rrThreadSlots[iSlot >> 5];
        } while (rrAtomicCmpXchg32(&rrThreadSlots[iSlot >> 5], uSlots & ~(1 << (iSlot & 31)), uSlots) != uSlots);

        return 0;
    }

    rrAtomicAddExchange32(&rrThreadCount, 1);

    if ((uFlags & RR_THREAD_FLAG_CREATE_SUSPENDED) == 0)
    {
        OSResumeThread(pNative->pOSThread);
    }

    return 1;
}

extern "C" void rrThreadSetCPUCore(rrThread* pThread, unsigned int uCore)
{
    rrThread* pFound = find_rrthread_in_tls(pThread);
    if (pFound != NULL && uCore <= 2)
    {
        OSSetThreadAffinity(((rriThread*)pFound->pNative)->pOSThread, rrThreadCoreAffinity[uCore]);
    }
}

extern "C" void rrThreadSetPriority(rrThread* pThread, unsigned int uPriority)
{
    rrThread* pFound;
    rriThread* pNative;

    pFound = find_rrthread_in_tls(pThread);
    if (pFound == NULL)
    {
        return;
    }

    pNative = (rriThread*)pFound->pNative;

    switch (uPriority)
    {
    case RR_THREAD_PRIORITY_LOWEST:
    case RR_THREAD_PRIORITY_LOW:
    case RR_THREAD_PRIORITY_NORMAL:
    case RR_THREAD_PRIORITY_HIGH:
    case RR_THREAD_PRIORITY_HIGHEST:
        OSSetThreadPriority(pNative->pOSThread, rrThreadPriorityLevel[uPriority + 2]);
        break;

    case RR_THREAD_PRIORITY_CALLER:
        OSSetThreadPriority(pNative->pOSThread, OSGetThreadPriority(OSGetCurrentThread()));
        break;

    case RR_THREAD_PRIORITY_RAISE:
    case RR_THREAD_PRIORITY_RAISE_RELATIVE:
        OSSetThreadPriority(pNative->pOSThread, OSGetThreadPriority(pNative->pOSThread) - 1);
        break;

    case RR_THREAD_PRIORITY_LOWER:
    case RR_THREAD_PRIORITY_LOWER_RELATIVE:
        OSSetThreadPriority(pNative->pOSThread, OSGetThreadPriority(pNative->pOSThread) + 1);
        break;

    default:
        break;
    }
}

extern "C" int rrThreadWaitDone(rrThread* pThread, unsigned int, unsigned int* puExitCode)
{
    rriThread* pNative;
    int iExitCode;

    if (pThread != NULL)
    {
        pNative = (rriThread*)pThread->pNative;

        if (pNative->uExitCode != 0)
        {
            if (puExitCode != NULL)
            {
                *puExitCode = pNative->uExitCode;
            }
        }
        else
        {
            if (OSJoinThread(pNative->pOSThread, &iExitCode) == 0)
            {
                return 0;
            }

            pNative->uExitCode = (unsigned int)iExitCode | RR_THREAD_EXIT_CODE_JOINED;

            if (puExitCode != NULL)
            {
                *puExitCode = (unsigned int)iExitCode;
            }
        }

        return 1;
    }

    return 0;
}

extern "C" void rrThreadCleanUp(rrThread* pThread)
{
    rrThread* pFound;
    rriThread* pNative;

    pFound = find_rrthread_in_tls(pThread);
    if (pFound == NULL)
    {
        return;
    }

    pNative = (rriThread*)pFound->pNative;

    rrAtomicCmpXchg32(&pNative->uState, RR_THREAD_STATE_RUNNING_DETACHED, RR_THREAD_STATE_RUNNING);

    if (pNative->uState == 0)
    {
        thread_cleanup(pNative);
    }
}

extern "C" void rrMutexCreate(rrMutex* pMutex, int)
{
    rriMutex* pNative;

    pNative = AlignMutexStorage(pMutex);
    pMutex->pNative = pNative;
    pNative->uCreated = 0;
    OSInitMutex(&pNative->mutex);
    rrAtomicMemoryBarrierFull();
    pNative->uCreated = 1;
}

extern "C" void rrMutexDestroy(rrMutex*)
{
}

extern "C" void rrMutexLock(rrMutex* pMutex)
{
    if (pMutex == NULL)
    {
        return;
    }

    OSLockMutex(&((rriMutex*)pMutex->pNative)->mutex);
}

extern "C" void rrMutexUnlock(rrMutex* pMutex)
{
    if (pMutex == NULL)
    {
        return;
    }

    OSUnlockMutex(&((rriMutex*)pMutex->pNative)->mutex);
}

extern "C" int rrMutexLockTimeout(rrMutex* pMutex, unsigned int uTimeoutMilliseconds)
{
    rriMutex* pNative;
    unsigned int uTimeoutTicks;
    unsigned int uStartTick;

    if (pMutex != NULL)
    {
        pNative = (rriMutex*)pMutex->pNative;

        if (uTimeoutMilliseconds == RR_WAIT_INFINITE)
        {
            OSLockMutex(&pNative->mutex);
            return 1;
        }

        if (uTimeoutMilliseconds == 0)
        {
            return OSTryLockMutex(&pNative->mutex);
        }

        uTimeoutTicks = (unsigned int)OSMillisecondsToTicks(uTimeoutMilliseconds);
        uStartTick = OSGetTick();

        for (;;)
        {
            if (OSTryLockMutex(&pNative->mutex) != 0)
            {
                return 1;
            }

            if (OSGetTick() - uStartTick >= uTimeoutTicks)
            {
                return 0;
            }

            OSSleepTicks(OSMicrosecondsToTicks(RR_SPIN_SLEEP_MICROSECONDS));
        }
    }

    return 0;
}

extern "C" void rrThreadSleep(unsigned int uMilliseconds)
{
    if (uMilliseconds == RR_WAIT_INFINITE)
    {
        uMilliseconds = RR_DEFAULT_SLEEP_MILLISECONDS;
    }

    OSSleepTicks(OSMicrosecondsToTicks((unsigned long long)uMilliseconds * 1000));
}

extern "C" int rrSemaphoreCreate(rrSemaphore* pSemaphore, int iInitialCount, int)
{
    rriSemaphore* pNative;

    pNative = AlignSemaphoreStorage(pSemaphore);
    pSemaphore->pNative = pNative;
    OSInitSemaphore(&pNative->semaphore, iInitialCount);
    rrAtomicMemoryBarrierFull();
    pNative->uCreated = RR_SEMAPHORE_CREATED;
    return 1;
}

extern "C" void rrSemaphoreDestroy(rrSemaphore* pSemaphore)
{
    rriSemaphore* pNative;

    if (pSemaphore == NULL)
    {
        return;
    }

    pNative = (rriSemaphore*)pSemaphore->pNative;
    if (pNative->uCreated != RR_SEMAPHORE_CREATED)
    {
        return;
    }

    pNative->uCreated = 0;
}

extern "C" int rrSemaphoreDecrementOrWait(rrSemaphore* pSemaphore, unsigned int uTimeoutMilliseconds)
{
    unsigned int uTimeoutTicks;
    unsigned int uStartTick;

    if (pSemaphore != NULL && ((rriSemaphore*)pSemaphore->pNative)->uCreated == RR_SEMAPHORE_CREATED)
    {
        if (uTimeoutMilliseconds == 0)
        {
            if (OSTryWaitSemaphore(&((rriSemaphore*)pSemaphore->pNative)->semaphore) < 1)
            {
                return 0;
            }
        }
        else
        {
            if (uTimeoutMilliseconds == RR_WAIT_INFINITE)
            {
                OSWaitSemaphore(&((rriSemaphore*)pSemaphore->pNative)->semaphore);
                return 1;
            }

            uTimeoutTicks = OSMillisecondsToTicks(uTimeoutMilliseconds);
            uStartTick = OSGetTick();

            while (OSTryWaitSemaphore(&((rriSemaphore*)pSemaphore->pNative)->semaphore) == 0)
            {
                if (OSGetTick() - uStartTick >= uTimeoutTicks)
                {
                    return 0;
                }

                OSSleepTicks(OSMicrosecondsToTicks(RR_SPIN_SLEEP_MICROSECONDS));
            }
        }

        return 1;
    }

    return 0;
}

extern "C" void rrSemaphoreIncrement(rrSemaphore* pSemaphore, int iCount)
{
    if (pSemaphore == NULL)
    {
        return;
    }

    if (((rriSemaphore*)pSemaphore->pNative)->uCreated != RR_SEMAPHORE_CREATED)
    {
        return;
    }

    for (int i = 0; i < iCount; i++)
    {
        OSSignalSemaphore(&((rriSemaphore*)pSemaphore->pNative)->semaphore);
    }
}

#ifdef __ghs__
extern "C" BOOL OSCompareAndSwapAtomicEx(volatile uint32_t* pTarget, uint32_t uComparand, uint32_t uNewValue,
                                         uint32_t* puPrevious)
{
    unsigned int* pWord = const_cast<unsigned int*>(pTarget);
    unsigned int uPrevious;
    BOOL bSwapped;

    uPrevious = __LWARX(pWord, 0);
    if (uPrevious == uComparand)
    {
        __DCBST(pWord, 0);
        if (__STWCX(pWord, 0, uNewValue))
        {
            bSwapped = 1;
        }
        else
        {
            for (;;)
            {
                uPrevious = __LWARX(pWord, 0);
                if (uPrevious == uComparand)
                {
                    __DCBST(pWord, 0);
                    if (__STWCX(pWord, 0, uNewValue))
                    {
                        bSwapped = 1;
                        break;
                    }
                }
                else
                {
                    bSwapped = 0;
                    break;
                }
            }
        }
    }
    else
    {
        bSwapped = 0;
    }

    *puPrevious = uPrevious;
    return bSwapped;
}
#endif

extern "C" void rrAtomicMemoryBarrierFull()
{
    FullBarrier();
}

extern "C" unsigned int rrAtomicLoadAcquire32(const volatile unsigned int* pTarget)
{
    unsigned int uValue;

    uValue = *pTarget;
    AcquireBarrier();
    return uValue;
}

extern "C" unsigned long long rrAtomicLoadAcquire64(const volatile unsigned long long* pTarget)
{
    unsigned long long uValue;

    uValue = *pTarget;
    AcquireBarrier();
    return uValue;
}

extern "C" void rrAtomicStoreRelease32(volatile unsigned int* pTarget, unsigned int uValue)
{
    AcquireBarrier();
    *pTarget = uValue;
}

extern "C" unsigned int rrAtomicCmpXchg32(volatile unsigned int* pTarget, unsigned int uNewValue,
                                          unsigned int uComparand)
{
    unsigned int uPrevious;

    rrAtomicMemoryBarrierFull();
    OSCompareAndSwapAtomicEx(pTarget, uComparand, uNewValue, &uPrevious);
    OSMemoryBarrier();
    return uPrevious;
}

extern "C" unsigned long long rrAtomicCmpXchg64(volatile unsigned long long* pTarget, unsigned long long uNewValue,
                                                unsigned long long uComparand)
{
    unsigned long long uPrevious;

    rrAtomicMemoryBarrierFull();
    OSCompareAndSwapAtomicEx64(pTarget, uComparand, uNewValue, &uPrevious);
    AcquireBarrier();
    return uPrevious;
}

extern "C" int rrAtomicAddExchange32(volatile int* pTarget, int iValue)
{
    FullBarrier();
    int iPrevious = AddAtomic(pTarget, iValue);
    AcquireBarrier();
    return iPrevious;
}

extern "C" long long rrAtomicAddExchange64(volatile long long* pTarget, long long lValue)
{
    long long lPrevious;

    FullBarrier();
    lPrevious = OSAddAtomic64(pTarget, lValue);
    AcquireBarrier();
    return lPrevious;
}
