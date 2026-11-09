#include "vendor/MilesAsync.hpp"

#include <stddef.h>

#include "vendor/MilesIO.hpp"
#include "vendor/MilesMem.hpp"

typedef int (*MilesAsyncFileReadFunc)(MilesAsyncRead* pRead);
typedef int (*MilesAsyncFileCancelFunc)(MilesAsyncRead* pRead);
typedef int (*MilesAsyncFileStatusFunc)(MilesAsyncRead* pRead, unsigned int uTimeoutMilliseconds);
typedef int (*MilesAsyncStartupFunc)();
typedef int (*MilesAsyncShutdownFunc)();

static int ThreadProc(void* pUserData);
void set_status(MilesAsyncRead* pRead, int iStatus);
int InternalCancelOrWait(MilesAsyncRead* pRead, unsigned int uTimeoutMilliseconds, int bCancel);

static const int MILES_ASYNC_WAIT_SLOTS = 32;
static const int MILES_ASYNC_READ_CHUNK = 0x8000;
static const unsigned int MILES_ASYNC_WAIT_INFINITE = 0xFFFFFFFF;

static const int MILES_ASYNC_PENDING = 0;
static const int MILES_ASYNC_COMPLETE = 1;
static const int MILES_ASYNC_OPEN_FAILED = 0x1003;
static const int MILES_ASYNC_READ_FAILED = 0x1004;
static const int MILES_ASYNC_ABORTED = 0x1005;
static const int MILES_ASYNC_CANCELLED = 0x1006;
static const int MILES_ASYNC_OUT_OF_MEMORY = 0x1007;
static const int MILES_ASYNC_FINISHED_MASK = 0x1000;

static const unsigned int MILES_ASYNC_FLAG_KEEP_OPEN = 1;
static const unsigned int MILES_ASYNC_FLAG_NO_READ = 2;
static const unsigned int MILES_ASYNC_FLAG_NO_SEEK = 4;

static const unsigned int MILES_LOCK_STATE_RELEASED = 1;
static const unsigned int MILES_LOCK_STATE_ACQUIRED = 2;

static const int MILES_ASYNC_MUTEX_KIND = 1;

static const unsigned int MILES_ASYNC_THREAD_STACK_SIZE = 0x4000;
static const unsigned int MILES_ASYNC_THREAD_BLOCK_SIZE = 0x4100;
static const unsigned int MILES_ASYNC_THREAD_FLAGS = 8;
static const unsigned int MILES_ASYNC_THREAD_PRIORITY = 0x10;
static const unsigned int MILES_ASYNC_THREAD_CORE = 2;

static const float MILES_ASYNC_PRIORITY_THRESHOLD = 30.0f;

static const char MILES_ASYNC_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\milesasync.cpp";
static const char MILES_ASYNC_MUTEX_BLOCK_NAME[] = "Miles IO mutex block";
static const char MILES_ASYNC_MUTEX_NAME[] = "Miles IO mutex";
static const char MILES_ASYNC_THREAD_NAME[] = "Miles IO";
static const char MILES_ASYNC_WAIT_NAME[] = "Waiting for IO request";
static const char MILES_ASYNC_UNKNOWN_OWNER[] = "unknown";

MilesProfile* MSS_profile = NULL;

MilesAsyncFileReadFunc MilesAsyncFileReadHandler = MilesAsyncFileRead_default;
MilesAsyncFileCancelFunc MilesAsyncFileCancelHandler = MilesAsyncFileCancel_default;
MilesAsyncFileStatusFunc MilesAsyncFileStatusHandler = MilesAsyncFileStatus_default;
MilesAsyncStartupFunc MilesAsyncStartupHandler = MilesAsyncStartup_default;
MilesAsyncShutdownFunc MilesAsyncShutdownHandler = MilesAsyncShutdown_default;

MilesAsyncRead* MilesAsyncQueueHead = NULL;
int MilesAsyncQuitting = 0;
int MilesAsyncStarted = 0;
int MilesAsyncPaused = 0;

rrThread* MilesAsyncThread;
MilesProfileSite MilesAsyncFileOpenSite;
MilesProfileSite MilesAsyncFileSeekSite;
MilesProfileSite MilesAsyncFileReadSite;
rrMutex MilesAsyncQueueMutex;
rrSemaphore MilesAsyncWorkSemaphore;
rrSemaphore MilesAsyncWaitSemaphores[MILES_ASYNC_WAIT_SLOTS + 1];
MilesAsyncRead* MilesAsyncWaitSlots[MILES_ASYNC_WAIT_SLOTS];
MilesProfileSite MilesAsyncQueueLockWaitSite;
MilesProfileSite MilesAsyncQueueLockSite;
MilesProfileSite MilesAsyncQueueUnlockSite;
MilesProfileSite MilesAsyncThreadNameSite;
MilesProfileSite MilesAsyncWaitSite;
MilesProfileSite MilesAsyncQueueMutexNameSite;

void LockQueue()
{
    if (rrMutexLockTimeout(&MilesAsyncQueueMutex, 0) == 0)
    {
        if (MSS_profile != NULL)
        {
            MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, 1, MILES_ASYNC_FILE, 126, &MilesAsyncQueueLockWaitSite,
                                    MILES_ASYNC_MUTEX_BLOCK_NAME);
        }

        rrMutexLock(&MilesAsyncQueueMutex);

        if (MSS_profile != NULL)
        {
            MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_ASYNC_FILE, 128);
        }
    }

    if (MSS_profile != NULL)
    {
        MSS_profile->pLockState(MSS_profile, &MilesAsyncQueueMutex, MILES_LOCK_STATE_ACQUIRED, MILES_ASYNC_FILE, 130,
                                &MilesAsyncQueueLockSite, MILES_ASYNC_MUTEX_NAME);
    }
}

void UnlockQueue()
{
    if (MSS_profile != NULL)
    {
        MSS_profile->pLockState(MSS_profile, &MilesAsyncQueueMutex, MILES_LOCK_STATE_RELEASED, MILES_ASYNC_FILE, 136,
                                &MilesAsyncQueueUnlockSite, MILES_ASYNC_MUTEX_NAME);
    }

    rrMutexUnlock(&MilesAsyncQueueMutex);
}

int InternalCancelOrWait(MilesAsyncRead* pRead, unsigned int uTimeoutMilliseconds, int bCancel)
{
    int iStartTime;
    int iNow;
    int iSlot;
    unsigned int uElapsed;
    unsigned int uRemaining;

    if (pRead->iStatus != MILES_ASYNC_PENDING)
    {
        return pRead->iStatus;
    }

    if (bCancel == 0)
    {
        if (uTimeoutMilliseconds == 0)
        {
            return MILES_ASYNC_PENDING;
        }
    }
    else
    {
        pRead->iCancel = 1;
    }

    rrSemaphoreIncrement(&MilesAsyncWorkSemaphore, 1);

    iStartTime = AIL_ms_count();

    LockQueue();

    iSlot = 0;
    while (iSlot < MILES_ASYNC_WAIT_SLOTS && MilesAsyncWaitSlots[iSlot] != NULL)
    {
        iSlot = iSlot + 1;
    }

    if (iSlot == MILES_ASYNC_WAIT_SLOTS)
    {
        UnlockQueue();

        uElapsed = (unsigned int)(AIL_ms_count() - iStartTime);
        while (uElapsed < uTimeoutMilliseconds && pRead->iStatus == MILES_ASYNC_PENDING)
        {
            rrThreadSleep(1);
            uElapsed = (unsigned int)(AIL_ms_count() - iStartTime);
        }

        return pRead->iStatus;
    }

    MilesAsyncWaitSlots[iSlot] = pRead;

    UnlockQueue();

    if (MSS_profile != NULL)
    {
        MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, 1, MILES_ASYNC_FILE, 208, &MilesAsyncWaitSite,
                                MILES_ASYNC_WAIT_NAME);
    }

    rrSemaphoreDecrementOrWait(&MilesAsyncWaitSemaphores[0], MILES_ASYNC_WAIT_INFINITE);

    uRemaining = uTimeoutMilliseconds;
    while (MilesAsyncQuitting == 0 && pRead->iStatus == MILES_ASYNC_PENDING)
    {
        rrSemaphoreDecrementOrWait(&MilesAsyncWaitSemaphores[iSlot + 1],
                                   uRemaining == MILES_ASYNC_WAIT_INFINITE ? 10 : 0);

        if (pRead->iStatus != MILES_ASYNC_PENDING)
        {
            break;
        }

        if (uTimeoutMilliseconds != MILES_ASYNC_WAIT_INFINITE)
        {
            iNow = AIL_ms_count();
            uElapsed = (unsigned int)(iNow - iStartTime);
            if (uElapsed >= uTimeoutMilliseconds)
            {
                break;
            }

            uRemaining = uTimeoutMilliseconds - uElapsed;
        }
    }

    if (MSS_profile != NULL)
    {
        MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_ASYNC_FILE, 240);
    }

    rrSemaphoreIncrement(&MilesAsyncWaitSemaphores[0], 1);

    LockQueue();
    MilesAsyncWaitSlots[iSlot] = NULL;
    UnlockQueue();

    return pRead->iStatus;
}

int MilesAsyncFileStatus_default(MilesAsyncRead* pRead, unsigned int uTimeoutMilliseconds)
{
    if (MilesAsyncPaused != 0 && uTimeoutMilliseconds < 2)
    {
        return MILES_ASYNC_PENDING;
    }

    return InternalCancelOrWait(pRead, uTimeoutMilliseconds, 0);
}

int MilesAsyncFileCancel_default(MilesAsyncRead* pRead)
{
    return InternalCancelOrWait(pRead, 0, 1);
}

int MilesAsyncFileRead_default(MilesAsyncRead* pRead)
{
    MilesAsyncRead* pTail;

    if (MilesAsyncStarted == 0)
    {
        return 0;
    }

    pRead->iStatus = MILES_ASYNC_PENDING;
    pRead->iBufferOwned = 0;
    pRead->iSeekDone = 0;
    pRead->iCancel = 0;

    LockQueue();

    if (MilesAsyncQueueHead == NULL)
    {
        MilesAsyncQueueHead = pRead;
    }
    else
    {
        pTail = MilesAsyncQueueHead;
        while (pTail->pNext != NULL)
        {
            pTail = pTail->pNext;
        }

        pTail->pNext = pRead;
        pRead->pPrevious = pTail;
    }

    UnlockQueue();

    rrSemaphoreIncrement(&MilesAsyncWorkSemaphore, 1);

    return 1;
}

void set_status(MilesAsyncRead* pRead, int iStatus)
{
    MilesAsyncRead* pNext;
    MilesAsyncRead* pPrevious;
    int iSlot;

    LockQueue();

    pNext = pRead->pNext;
    pPrevious = pRead->pPrevious;

    if (MilesAsyncQueueHead == pRead)
    {
        MilesAsyncQueueHead = pNext;
    }

    if (pNext != NULL)
    {
        pNext->pPrevious = pPrevious;
    }

    if (pPrevious != NULL)
    {
        pPrevious->pNext = pNext;
    }

    UnlockQueue();

    pRead->pNext = NULL;
    pRead->pPrevious = NULL;

    rrAtomicMemoryBarrierFull();

    pRead->iStatus = iStatus;

    if ((iStatus & MILES_ASYNC_FINISHED_MASK) != 0 && pRead->pBuffer != NULL && pRead->iBufferOwned != 0)
    {
        pRead->iBufferOwned = 0;
        AIL_mem_free_lock(pRead->pBuffer);
        pRead->pBuffer = NULL;
    }

    if (pRead->uFileHandle != 0)
    {
        if ((pRead->uFlags & MILES_ASYNC_FLAG_KEEP_OPEN) == 0)
        {
            AIL_file_close(pRead->uFileHandle);
            pRead->uFileHandle = 0;
        }
    }

    LockQueue();

    for (iSlot = 0; iSlot < MILES_ASYNC_WAIT_SLOTS; iSlot++)
    {
        if (MilesAsyncWaitSlots[iSlot] == pRead)
        {
            rrSemaphoreIncrement(&MilesAsyncWaitSemaphores[iSlot + 1], 1);
        }
    }

    UnlockQueue();
}

static int ThreadProc(void*)
{
    MilesAsyncRead* pRequest;
    MilesAsyncRead* pOther;
    MilesAsyncRead* pSelected;
    MilesAsyncRead* pFirstPending;
    MilesAsyncRead* pFirstComplete;
    MilesAsyncRead* pLastServiced;
    const char* pOwnerName;
    int bDuplicate;
    int iSize;
    int iRemaining;
    int iChunkSize;
    int iBytesRead;

    pLastServiced = NULL;

    if (MSS_profile != NULL)
    {
        MSS_profile->pThreadName(MSS_profile, 0, &MilesAsyncThreadNameSite, MILES_ASYNC_THREAD_NAME);
    }

    while (MilesAsyncQuitting == 0)
    {
        LockQueue();

        pSelected = NULL;
        pFirstPending = NULL;
        pFirstComplete = NULL;

        for (pRequest = MilesAsyncQueueHead; pRequest != NULL; pRequest = pRequest->pNext)
        {
            if (pRequest->uFileHandle != 0)
            {
                bDuplicate = 0;
                for (pOther = MilesAsyncQueueHead; pOther != pRequest; pOther = pOther->pNext)
                {
                    if (pOther->uFileHandle == pRequest->uFileHandle)
                    {
                        bDuplicate = 1;
                        break;
                    }
                }

                if (bDuplicate != 0)
                {
                    continue;
                }
            }

            if (pRequest->pPriorityCallback != NULL
                && pRequest->pPriorityCallback(pRequest->pPriorityUserData) < MILES_ASYNC_PRIORITY_THRESHOLD)
            {
                pSelected = pRequest;
                break;
            }

            if (pRequest->iStatus == MILES_ASYNC_PENDING && pFirstPending == NULL)
            {
                pFirstPending = pRequest;
            }

            if (pRequest->iStatus == MILES_ASYNC_COMPLETE && pFirstComplete == NULL)
            {
                pFirstComplete = pRequest;
            }
        }

        if (pSelected == NULL)
        {
            pSelected = pFirstPending;
            if (pSelected == NULL)
            {
                pSelected = pLastServiced;
            }
            if (pSelected == NULL)
            {
                pSelected = pFirstComplete;
            }
        }

        if (MilesAsyncPaused != 0)
        {
            rrThreadSleep(100);
            UnlockQueue();
        }
        else if (pSelected == NULL)
        {
            UnlockQueue();
        }
        else
        {
            UnlockQueue();

            if (pSelected->iCancel != 0)
            {
                set_status(pSelected, MILES_ASYNC_CANCELLED);
                pLastServiced = NULL;
                continue;
            }

            if (pSelected->uFileHandle == 0)
            {
                if (MSS_profile != NULL)
                {
                    pOwnerName = pSelected->pOwnerName;
                    if (pOwnerName == NULL)
                    {
                        pOwnerName = MILES_ASYNC_UNKNOWN_OWNER;
                    }

                    MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, 0, MILES_ASYNC_FILE, 450, &MilesAsyncFileOpenSite,
                                            "File open: %s (%s line: %d)",
                                            MSS_profile->pDescribe(MSS_profile, pSelected), pOwnerName,
                                            pSelected->iOwnerLine);
                }

                if (AIL_file_open(pSelected->szFileName, &pSelected->uFileHandle) == 0)
                {
                    set_status(pSelected, MILES_ASYNC_OPEN_FAILED);

                    if (MSS_profile != NULL)
                    {
                        MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_ASYNC_FILE, 454);
                    }

                    pLastServiced = NULL;
                    continue;
                }

                if (MSS_profile != NULL)
                {
                    MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_ASYNC_FILE, 457);
                }
            }

            if (pSelected->iLength < 0)
            {
                iSize = AIL_file_seek(pSelected->uFileHandle, 0, 2) - (int)pSelected->lOffset;
                pSelected->iLength = iSize;
                pSelected->iLengthHigh = iSize >> 31;
            }

            if ((pSelected->uFlags & MILES_ASYNC_FLAG_NO_READ) == 0 && pSelected->iLength != 0)
            {
                if (pSelected->pBuffer == NULL)
                {
                    pSelected->pBuffer = AIL_mem_alloc_lock_info(pSelected->iLength + pSelected->iBufferOffset,
                                                                 pSelected->pOwnerName, pSelected->iOwnerLine);

                    if (pSelected->pBuffer == NULL)
                    {
                        set_status(pSelected, MILES_ASYNC_OUT_OF_MEMORY);
                        pLastServiced = NULL;
                        continue;
                    }

                    pSelected->iBufferOwned = 1;
                }

                if ((pSelected->uFlags & MILES_ASYNC_FLAG_NO_SEEK) == 0 && pSelected->iSeekDone == 0)
                {
                    if (MSS_profile != NULL)
                    {
                        pOwnerName = pSelected->pOwnerName;
                        if (pOwnerName == NULL)
                        {
                            pOwnerName = MILES_ASYNC_UNKNOWN_OWNER;
                        }

                        MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, 0, MILES_ASYNC_FILE, 494, &MilesAsyncFileSeekSite,
                                                "File seek: %s to %d (%s line: %d)",
                                                MSS_profile->pDescribe(MSS_profile, pSelected), pSelected->lOffset,
                                                pOwnerName, pSelected->iOwnerLine);
                    }

                    AIL_file_seek(pSelected->uFileHandle, (int)pSelected->lOffset, 0);

                    if (MSS_profile != NULL)
                    {
                        MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_ASYNC_FILE, 496);
                    }

                    pSelected->iSeekDone = 1;
                }

                iRemaining = pSelected->iLength - pSelected->iBytesRead;
                iChunkSize = MILES_ASYNC_READ_CHUNK;
                if (iRemaining - MILES_ASYNC_READ_CHUNK < MILES_ASYNC_READ_CHUNK)
                {
                    iChunkSize = iRemaining;
                }

                if (MSS_profile != NULL)
                {
                    pOwnerName = pSelected->pOwnerName;
                    if (pOwnerName == NULL)
                    {
                        pOwnerName = MILES_ASYNC_UNKNOWN_OWNER;
                    }

                    MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, 0, MILES_ASYNC_FILE, 507, &MilesAsyncFileReadSite,
                                            "File read: %s bytes: %d (%s line: %d)",
                                            MSS_profile->pDescribe(MSS_profile, pSelected), iChunkSize, pOwnerName,
                                            pSelected->iOwnerLine);
                }

                iBytesRead = AIL_file_read(pSelected->uFileHandle,
                                           (char*)pSelected->pBuffer + pSelected->iBytesRead + pSelected->iBufferOffset,
                                           iChunkSize);

                if (MSS_profile != NULL)
                {
                    MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_ASYNC_FILE, 509);
                }

                pSelected->iBytesRead = pSelected->iBytesRead + iBytesRead;

                if (iBytesRead == 0)
                {
                    set_status(pSelected, MILES_ASYNC_READ_FAILED);
                    pLastServiced = NULL;
                    continue;
                }

                if ((pSelected->iBytesRead >> 31 != pSelected->iLengthHigh
                     || pSelected->iBytesRead != pSelected->iLength)
                    && iBytesRead == iChunkSize)
                {
                    pLastServiced = pSelected;
                    continue;
                }
            }

            set_status(pSelected, MILES_ASYNC_COMPLETE);
            pLastServiced = NULL;
            continue;
        }

        if (MSS_profile != NULL)
        {
            MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, 2, MILES_ASYNC_FILE, 607, &MilesAsyncWaitSite,
                                    MILES_ASYNC_WAIT_NAME);
        }

        rrSemaphoreDecrementOrWait(&MilesAsyncWorkSemaphore, MILES_ASYNC_WAIT_INFINITE);

        if (MSS_profile != NULL)
        {
            MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_ASYNC_FILE, 609);
        }
    }

    return 1;
}

int MilesAsyncStartup_default()
{
    int iSlot;

    if (MilesAsyncStarted == 0)
    {
        MilesAsyncStarted = 1;
        MilesAsyncQuitting = 0;

        rrMutexCreate(&MilesAsyncQueueMutex, MILES_ASYNC_MUTEX_KIND);

        if (MSS_profile != NULL)
        {
            MSS_profile->pLockName(MSS_profile, &MilesAsyncQueueMutex, &MilesAsyncQueueMutexNameSite,
                                   MILES_ASYNC_MUTEX_NAME);
        }

        rrSemaphoreCreate(&MilesAsyncWorkSemaphore, 0, 0x200);
        rrSemaphoreCreate(&MilesAsyncWaitSemaphores[0], MILES_ASYNC_WAIT_SLOTS, MILES_ASYNC_WAIT_SLOTS);

        for (iSlot = 0; iSlot < MILES_ASYNC_WAIT_SLOTS; iSlot++)
        {
            rrSemaphoreCreate(&MilesAsyncWaitSemaphores[iSlot + 1], 0, 0x80);
        }

        MilesAsyncThread = (rrThread*)AIL_mem_alloc_lock_info(MILES_ASYNC_THREAD_BLOCK_SIZE, MILES_ASYNC_FILE, 663);
        rrThreadCreate(MilesAsyncThread, ThreadProc, MILES_ASYNC_THREAD_STACK_SIZE, NULL, MILES_ASYNC_THREAD_FLAGS,
                       "MSSAsync");
        rrThreadSetPriority(MilesAsyncThread, MILES_ASYNC_THREAD_PRIORITY);
        rrThreadSetCPUCore(MilesAsyncThread, MILES_ASYNC_THREAD_CORE);
    }

    return 1;
}

int MilesAsyncShutdown_default()
{
    int iSlot;

    if (MilesAsyncStarted != 0)
    {
        MilesAsyncQuitting = 1;
        MilesAsyncStarted = 0;

        rrSemaphoreIncrement(&MilesAsyncWorkSemaphore, 4);
        rrThreadWaitDone(MilesAsyncThread, MILES_ASYNC_WAIT_INFINITE, NULL);
        rrThreadCleanUp(MilesAsyncThread);
        AIL_mem_free_lock(MilesAsyncThread);
        MilesAsyncThread = NULL;

        rrSemaphoreDestroy(&MilesAsyncWorkSemaphore);

        while (MilesAsyncQueueHead != NULL)
        {
            set_status(MilesAsyncQueueHead, MILES_ASYNC_ABORTED);
        }

        MilesAsyncQueueHead = NULL;

        for (iSlot = 0; iSlot < MILES_ASYNC_WAIT_SLOTS; iSlot++)
        {
            rrSemaphoreDecrementOrWait(&MilesAsyncWaitSemaphores[0], MILES_ASYNC_WAIT_INFINITE);
        }

        rrSemaphoreDestroy(&MilesAsyncWaitSemaphores[0]);

        for (iSlot = 0; iSlot < MILES_ASYNC_WAIT_SLOTS; iSlot++)
        {
            rrSemaphoreDestroy(&MilesAsyncWaitSemaphores[iSlot + 1]);
        }

        rrMutexDestroy(&MilesAsyncQueueMutex);
    }

    return 1;
}

extern "C" int MilesAsyncFileRead(MilesAsyncRead* pRead)
{
    return MilesAsyncFileReadHandler(pRead);
}

extern "C" int MilesAsyncFileCancel(MilesAsyncRead* pRead)
{
    return MilesAsyncFileCancelHandler(pRead);
}

extern "C" int MilesAsyncFileStatus(MilesAsyncRead* pRead, unsigned int uTimeoutMilliseconds)
{
    return MilesAsyncFileStatusHandler(pRead, uTimeoutMilliseconds);
}

extern "C" int MilesAsyncStartup()
{
    return MilesAsyncStartupHandler();
}

extern "C" int MilesAsyncShutdown()
{
    return MilesAsyncShutdownHandler();
}
