#include "game/core/ShutdownManager.hpp"

#include <stddef.h>

#include "game/core/CriticalSection.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

int g_threadRunningCount[ShutdownManager::eThreadId_Count];
C4JEventArray* g_pThreadEventArray[ShutdownManager::eThreadId_Count];
bool g_threadShouldRun[ShutdownManager::eThreadId_Count];
static OSFastMutex g_shutdownLock;

void ShutdownManager::Initialise()
{
    for (int threadId = 0; threadId < eThreadId_Count; threadId++)
    {
        g_threadShouldRun[threadId] = true;
        g_threadRunningCount[threadId] = 0;
        g_pThreadEventArray[threadId] = NULL;
    }

    InitializeCriticalSection(&g_shutdownLock);
}

void ShutdownManager::HasStarted(EThreadId threadId)
{
    EnterCriticalSection(&g_shutdownLock);

    g_threadRunningCount[threadId]++;

    LeaveCriticalSection(&g_shutdownLock);
}

void ShutdownManager::HasStarted(EThreadId threadId, C4JEventArray* pEventArray)
{
    EnterCriticalSection(&g_shutdownLock);

    g_threadRunningCount[threadId]++;

    LeaveCriticalSection(&g_shutdownLock);

    g_pThreadEventArray[threadId] = pEventArray;
}

bool ShutdownManager::ShouldRun(EThreadId threadId)
{
    return g_threadShouldRun[threadId];
}

void ShutdownManager::HasFinished(EThreadId threadId)
{
    EnterCriticalSection(&g_shutdownLock);

    g_threadRunningCount[threadId]--;

    LeaveCriticalSection(&g_shutdownLock);
}

void ShutdownManager::RequestThreadToStop(int threadId)
{
    g_threadShouldRun[threadId] = false;
}

void ShutdownManager::WaitForSignalledToComplete()
{
    bool allStopped;

    do
    {
        Sleep(10);

        allStopped = true;

        for (int threadId = 0; threadId < eThreadId_Count; threadId++)
        {
            if (!g_threadShouldRun[threadId] && g_threadRunningCount[threadId] != 0)
            {
                allStopped = false;
            }
        }
    } while (!allStopped);
}

void ShutdownManager::MainThreadHandleShutdown()
{
    RequestThreadToStop(eThreadId_ConnectionRead);

    WaitForSignalledToComplete();
}

void ShutdownManager::StorageManagerCompleteFn()
{
    HasFinished(eThreadId_Storage);
}
