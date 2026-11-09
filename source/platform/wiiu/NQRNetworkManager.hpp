#ifndef PLATFORM_WIIU_NQRNETWORKMANAGER_HPP
#define PLATFORM_WIIU_NQRNETWORKMANAGER_HPP

#include <deque>
#include <string>

#include "game/core/CriticalSection.hpp"
#include "game/world/generation/PlayerUID.hpp"

class CMatchMakeSearchParam;
class NQRNetworkPlayer;

namespace nn
{
namespace fp
{
struct FriendPresence;
}
}

class NQRNetworkManager
{
public:
    NQRNetworkManager();
    virtual ~NQRNetworkManager() {}

    enum VTableSlot
    {
        eVTableDestructor = 0,
        eVTableTick = 1,
        eVTableInitialise,
        eVTableTerminate,
        eVTableGetState,
        eVTableIsHost,
        eVTableIsReadyToPlayOrIdle,
        eVTableIsInSession,
        eVTableIsMatchmakingEnabled,
        eVTableCreateAndJoinRoom,
        eVTableUpdateExternalRoomData,
        eVTableFriendRoomManagerIsBusy,
        eVTableFriendRoomManagerSearch,
        eVTableFriendRoomManagerSearch2,
        eVTableFriendRoomManagerGetCount,
        eVTableFriendRoomManagerGetRoomInfo,
        eVTableJoinRoomResult,
        eVTableJoinRoomSession,
        eVTableJoinRoomFriend,
        eVTableFriendPresenceUpdatedRoomList,
        eVTableGetFriendRoomSearchResults,
        eVTableGetExtDataForRoom,
        eVTableFindAndJoinRoom,
        eVTableStartGame,
        eVTableLeaveRoom,
        eVTableEndGame,
        eVTableSessionHasSpace,
        eVTableSessionHasMii,
        eVTableAddLocalPlayerByUserIndex,
        eVTableRemoveLocalPlayerByUserIndex,
        eVTableSendInviteGUI,
        eVTableGetPlayerCount,
        eVTableGetOnlinePlayerCount,
        eVTableGetPlayerByIndex,
        eVTableGetPlayerBySmallId,
        eVTableGetPlayerByXuid,
        eVTableGetDisplayNameByGamertag,
        eVTableGetLocalPlayerByUserIndex,
        eVTableGetHostPlayer,
        eVTableSetPresenceDataStartHostingGame,
        eVTableGetJoiningReadyPercentage,
        eVTableLocalDataSend,
        eVTableRemoteDataSend,
        eVTableGetSessionIndex,
        eVTableTryToConnectOnline,
        eVTableIsAdhocConnected,
        eVTableConnectToAdhoc,
        eVTableIsLanConnected,
        eVTableCount = 48
    };

    enum eNQRNetworkManagerState
    {
        eNQRNetworkManagerState_Uninitialised,
        eNQRNetworkManagerState_Initialising,
        eNQRNetworkManagerState_Idle,
        eNQRNetworkManagerState_Hosting,
        eNQRNetworkManagerState_Joining,
        eNQRNetworkManagerState_Starting,
        eNQRNetworkManagerState_Playing,
        eNQRNetworkManagerState_Leaving,
        eNQRNetworkManagerState_Ending
    };

    enum eNQRNetworkManagerInternalState
    {
        eNQRNetworkManagerInternalState_Uninitialised,
        eNQRNetworkManagerInternalState_InitialiseFailed,
        eNQRNetworkManagerInternalState_Idle,
        eNQRNetworkManagerInternalState_LoggingIn,
        eNQRNetworkManagerInternalState_HostWaitingForLogin,
        eNQRNetworkManagerInternalState_HostCreatingSession,
        eNQRNetworkManagerInternalState_HostSessionCreated,
        eNQRNetworkManagerInternalState_HostCreateSessionFailed,
        eNQRNetworkManagerInternalState_HostRoomReady,
        eNQRNetworkManagerInternalState_JoinWaitingForLogin,
        eNQRNetworkManagerInternalState_JoinRequested,
        eNQRNetworkManagerInternalState_JoinJoiningSession,
        eNQRNetworkManagerInternalState_JoinSessionFailed,
        eNQRNetworkManagerInternalState_JoinRoomReady,
        eNQRNetworkManagerInternalState_JoinSearchingMatchmakeSession,
        eNQRNetworkManagerInternalState_JoinNoMatchmakeSessionFound,
        eNQRNetworkManagerInternalState_JoinJoiningMatchmakeSession,
        eNQRNetworkManagerInternalState_JoinMatchmakeSessionFailed,
        eNQRNetworkManagerInternalState_EndingSession,
        eNQRNetworkManagerInternalState_StartingGame,
        eNQRNetworkManagerInternalState_Playing,
        eNQRNetworkManagerInternalState_LeavingRoom,
        eNQRNetworkManagerInternalState_LeaveRoomFailed,
        eNQRNetworkManagerInternalState_Ending
    };

    enum eNQRNetworkManagerFriendSearchState
    {
        eNQRNetworkManagerFriendSearchState_Idle,
        eNQRNetworkManagerFriendSearchState_Searching,
        eNQRNetworkManagerFriendSearchState_Complete
    };

    enum eNQRForceError
    {
        eNQRForceError_None,
        eNQRForceError_Connect,
        eNQRForceError_CreateSession,
        eNQRForceError_JoinSession,
        eNQRForceError_StartGame,
        eNQRForceError_InGame,
        eNQRForceError_LeaveSession
    };

    struct StateChangeInfo
    {
        StateChangeInfo(eNQRNetworkManagerState oldState, eNQRNetworkManagerState newState, bool error)
            : eOldState(oldState),
              eNewState(newState),
              bError(error)
        {
        }

        eNQRNetworkManagerState eOldState;
        eNQRNetworkManagerState eNewState;
        bool bError;
    };

    struct PresenceSyncInfo
    {
        unsigned int m_uiWorldId;
        unsigned short m_usTitleVersion;
        unsigned char m_ucGameMode;
        unsigned char m_ucPlayerCount;
        unsigned char m_ucMaxPlayers;
        bool m_bJoinable;
        unsigned int m_uiHostAttributes;
    };

    struct SessionSearchResult
    {
        unsigned int m_uiSessionId;
        unsigned int m_uiOwnerPrincipalId;
        unsigned int m_uiOwnerLocalFriendIndex;
        char m_szOwnerName[17];
        void* m_pExtData;
    };

    class PlayerSyncData : public PlayerUID
    {
    public:
        PlayerSyncData();

        unsigned long long m_uiStationId;
        char m_szNetworkName[22];
        wchar_t m_szMiiName[11];
        unsigned int m_uiSmallId : 8;
        unsigned int m_uiLocalPlayerIndex : 4;
#ifdef MINECRAFT_PC
        unsigned int m_uiPlayerCountMinusOne : 8;
        unsigned int m_uiSyncFlags : 12;
#else
        unsigned int m_uiPlayerCountMinusOne : 4;
        unsigned int m_uiSyncFlags : 16;
#endif
    };

    virtual void Tick() = 0;
    virtual void Initialise() = 0;
    virtual void Terminate() = 0;
    virtual eNQRNetworkManagerState GetState() = 0;
    virtual bool IsHost() = 0;
    virtual bool IsReadyToPlayOrIdle() = 0;
    virtual bool IsInSession() = 0;
    virtual bool IsMatchmakingEnabled() = 0;
    virtual void CreateAndJoinRoom(int, int, void*, int, int*, int, bool, unsigned char) = 0;
    virtual void UpdateExternalRoomData() = 0;
    virtual bool FriendRoomManagerIsBusy() = 0;
    virtual bool FriendRoomManagerSearch() = 0;
    virtual bool FriendRoomManagerSearch2() = 0;
    virtual int FriendRoomManagerGetCount() = 0;
    virtual void FriendRoomManagerGetRoomInfo(int, SessionSearchResult*) = 0;
    virtual bool JoinRoom(SessionSearchResult*, int) = 0;
    virtual bool JoinRoom(unsigned int, int, const PresenceSyncInfo*) = 0;
    virtual bool JoinRoom(int, const nn::fp::FriendPresence*) = 0;
    virtual bool FriendPresenceUpdatedRoomList() = 0;
    virtual SessionSearchResult* GetFriendRoomSearchResults(int*) = 0;
    virtual void GetExtDataForRoom(unsigned int, void*, void (*)(bool, void*), void*) = 0;
    virtual bool FindAndJoinRoom(int, unsigned int, CMatchMakeSearchParam*, unsigned char) = 0;
    virtual void StartGame() = 0;
    virtual void LeaveRoom(bool) = 0;
    virtual void EndGame() = 0;
    virtual bool SessionHasSpace(int) = 0;
    virtual bool SessionHasMii(int) = 0;
    virtual bool AddLocalPlayerByUserIndex(int) = 0;
    virtual bool RemoveLocalPlayerByUserIndex(int) = 0;
    virtual void SendInviteGUI() = 0;
    virtual int GetPlayerCount() = 0;
    virtual int GetOnlinePlayerCount() = 0;
    virtual NQRNetworkPlayer* GetPlayerByIndex(int) = 0;
    virtual NQRNetworkPlayer* GetPlayerBySmallId(int) = 0;
    virtual NQRNetworkPlayer* GetPlayerByXuid(PlayerUID) = 0;
    virtual std::wstring GetDisplayNameByGamertag(std::wstring) = 0;
    virtual NQRNetworkPlayer* GetLocalPlayerByUserIndex(int) = 0;
    virtual NQRNetworkPlayer* GetHostPlayer() = 0;
    virtual void SetPresenceDataStartHostingGame() = 0;
    virtual int GetJoiningReadyPercentage() = 0;
    virtual void LocalDataSend(NQRNetworkPlayer*, NQRNetworkPlayer*, const void*, unsigned int) = 0;
    virtual void RemoteDataSend(NQRNetworkPlayer*, NQRNetworkPlayer*, const void*, unsigned int) = 0;
    virtual int GetSessionIndex(NQRNetworkPlayer*) = 0;
    virtual void TryToConnectOnline(int (*)(void*, bool, int), void*, bool) = 0;
    virtual bool isAdhocConnected() = 0;
    virtual bool connectToAdhoc() = 0;
    virtual bool isLanConnected() = 0;

    int GetOutstandingAckCount(NQRNetworkPlayer* pPlayer);
    int GetSendQueueSizeBytes();
    int GetSendQueueSizeMessages();

    static void SafeToRespondToGameBootInvite(int iUserIndex);

protected:
    std::deque<StateChangeInfo> m_stateChanges;
    OSFastMutex m_stateChangeLock;
    OSFastMutex m_playerLock;
    OSFastMutex m_localPlayerRequestLock;
    std::deque<int> m_localPlayerRequests;
};

extern int g_iGameBootInviteUserIndex;
extern bool g_bSafeToRespondToGameBootInvite;

#endif
