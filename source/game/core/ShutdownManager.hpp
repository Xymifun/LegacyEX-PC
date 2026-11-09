#ifndef GAME_CORE_SHUTDOWNMANAGER_HPP
#define GAME_CORE_SHUTDOWNMANAGER_HPP

class C4JEventArray;

class ShutdownManager
{
public:
    enum EThreadId
    {
        eThreadId_Main = 0,
        eThreadId_Sound = 1,
        eThreadId_Storage = 2,
        eThreadId_ServerPostUpdate = 3,
        eThreadId_ServerLevelUpdate = 4,
        eThreadId_RenderUpdate = 5,
        eThreadId_ServerMain = 6,
        eThreadId_ChunkSave = 7,
        eThreadId_ConnectionRead = 8,
        eThreadId_ConnectionWrite = 9,
        eThreadId_EventQueue = 10,
        eThreadId_Network = 11,
        eThreadId_Count = 12
    };

    static void Initialise();

    static void StartShutdown();
    static bool IsShuttingDown();

    static void HasStarted(EThreadId threadId);
    static void HasStarted(EThreadId threadId, C4JEventArray* pEventArray);
    static bool ShouldRun(EThreadId threadId);
    static void HasFinished(EThreadId threadId);

    static void RequestThreadToStop(int threadId);
    static void WaitForSignalledToComplete();
    static void MainThreadHandleShutdown();

    static void StorageManagerCompleteFn();
};

#endif
