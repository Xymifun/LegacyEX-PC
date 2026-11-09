#ifndef PLATFORM_WIIU_NQRNETWORKMANAGER_WIIU_HPP
#define PLATFORM_WIIU_NQRNETWORKMANAGER_WIIU_HPP

#include <deque>
#include <map>
#include <string>
#include <vector>

#include <nn/result.h>
#include <nn/fp/fp_cpp.h>

#include "game/core/CriticalSection.hpp"
#include "game/network/INQRNetworkManagerListener.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"
#include "nn/pia/inet/NexSessionSearchCriteria.hpp"
#include "nn/pia/session/Session.hpp"

class C4JEventImpl;
class C4JThreadImpl;

namespace nn
{
namespace nex
{
class Gathering;
class ProtocolCallContext;
class String;
template <typename T, typename KeyType> class AnyObjectHolder;
}
}

namespace nn
{
namespace fp
{
struct FriendPresence;
}
}

class CMatchMakeSearchParam
{
public:
    enum EComparison
    {
        eComparison_Equal,
        eComparison_NotEqual,
        eComparison_LessThan,
        eComparison_LessThanOrEqual,
        eComparison_GreaterThan,
        eComparison_GreaterThanOrEqual
    };

    unsigned int id;
    unsigned int value;
    unsigned int comparison;
};

class NQRNetworkManager_WiiU : public NQRNetworkManager
{
    friend class NintendoVoiceChat_WiiU;

public:
    enum ENEXAsyncProcess
    {
        eNEXAsyncProcess_None,
        eNEXAsyncProcess_Login,
        eNEXAsyncProcess_Logout,
        eNEXAsyncProcess_Finalize,
        eNEXAsyncProcess_Shutdown,
        eNEXAsyncProcess_Search,
        eNEXAsyncProcess_Update,
        eNEXAsyncProcess_CreateSession,
        eNEXAsyncProcess_JoinSession,
        eNEXAsyncProcess_LeaveSession,
        eNEXAsyncProcess_BrowseSession,
        eNEXAsyncProcess_UpdateSession,
        eNEXAsyncProcess_JoinRandomSession
    };

    enum ENEXLoginState
    {
        eNEXLoginState_Pending = 0,
        eNEXLoginState_LoggedIn = 2,
        eNEXLoginState_Failed = 3
    };

    enum EInviteConnectPrompt
    {
        eInviteConnectPrompt_None,
        eInviteConnectPrompt_Showing,
        eInviteConnectPrompt_Answered
    };

    enum EJoinFailedReason
    {
        eJoinFailedReason_GameFull,
        eJoinFailedReason_MiiInUse
    };

    struct SendQueueItem
    {
        unsigned long long stationId;
        unsigned char* data;
        unsigned int size;
    };

    struct FriendSearchResult
    {
        unsigned int friendPrincipalId;
        unsigned int hostPrincipalId;
        char friendAccountId[34];
        unsigned int sessionId;
        bool joinable;
        void* extData;
    };

    struct ConnectedStation
    {
        unsigned long long stationId;
        int state;
        unsigned int playerMask;
    };

    class ClientJoinData
    {
    public:
        ClientJoinData();

        unsigned int playerMask;
        PlayerUID playerUIDs[4];
        char onlineIds[4][21];
        wchar_t displayNames[4][11];
    };

    NQRNetworkManager_WiiU(INQRNetworkManagerListener* listener);
    virtual ~NQRNetworkManager_WiiU();

    virtual void Tick();
    virtual void Initialise();
    virtual void Terminate();
    virtual eNQRNetworkManagerState GetState();
    virtual bool IsHost();
    virtual bool IsReadyToPlayOrIdle();
    virtual bool IsInSession();
    virtual bool IsMatchmakingEnabled();
    virtual void CreateAndJoinRoom(int userIndex, int localUserMask, void* extData, int extDataSize, int* attributes,
                                   int attributeCount, bool localGame, unsigned char gameMode);
    virtual void UpdateExternalRoomData();
    virtual bool FriendRoomManagerIsBusy();
    virtual bool FriendRoomManagerSearch();
    virtual bool FriendRoomManagerSearch2();
    virtual int FriendRoomManagerGetCount();
    virtual void FriendRoomManagerGetRoomInfo(int index, SessionSearchResult* result);
    virtual bool JoinRoom(SessionSearchResult* result, int userIndexMask);
    virtual bool JoinRoom(unsigned int sessionId, int userIndexMask, const PresenceSyncInfo* presenceInfo);
    virtual bool JoinRoom(int userIndexMask, const nn::fp::FriendPresence* friendPresence);
    virtual bool FriendPresenceUpdatedRoomList();
    virtual SessionSearchResult* GetFriendRoomSearchResults(int* count);
    virtual void GetExtDataForRoom(unsigned int sessionId, void* extData, void (*callback)(bool, void*),
                                   void* callbackData);
    virtual bool FindAndJoinRoom(int userIndexMask, unsigned int searchFlags, CMatchMakeSearchParam* searchParam,
                                 unsigned char gameMode);
    virtual void StartGame();
    virtual void LeaveRoom(bool actuallyLeaveRoom);
    virtual void EndGame();
    virtual bool SessionHasSpace(int extraPlayers);
    virtual bool SessionHasMii(int userIndex);
    virtual bool AddLocalPlayerByUserIndex(int userIndex);
    virtual bool RemoveLocalPlayerByUserIndex(int userIndex);
    virtual void SendInviteGUI();
    virtual int GetPlayerCount();
    virtual int GetOnlinePlayerCount();
    virtual NQRNetworkPlayer* GetPlayerByIndex(int index);
    virtual NQRNetworkPlayer* GetPlayerBySmallId(int smallId);
    virtual NQRNetworkPlayer* GetPlayerByXuid(PlayerUID uid);
    virtual std::wstring GetDisplayNameByGamertag(std::wstring gamertag);
    virtual NQRNetworkPlayer* GetLocalPlayerByUserIndex(int userIndex);
    virtual NQRNetworkPlayer* GetHostPlayer();
    virtual void SetPresenceDataStartHostingGame();
    virtual int GetJoiningReadyPercentage();
    virtual void LocalDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data, unsigned int size);
    virtual void RemoteDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data, unsigned int size);
    virtual int GetSessionIndex(NQRNetworkPlayer* player);
    virtual void TryToConnectOnline(int (*callback)(void*, bool, int), void* callbackData, bool showProgress);
    virtual bool isAdhocConnected();
    virtual bool connectToAdhoc();
    virtual bool isLanConnected();

    void SetNexAsyncProcess(ENEXAsyncProcess process);
    static int ConnectOnlineNintendo_Returned(void* context, bool connected, int error);
    static int ConnectOnlineNintendoThreadProc(void* context);
    void CancelConnectCallbacks();
    static void CancelConnectOnlineNintendo(void* context);
    static int ShowConnectOnlineNintendoProgress(int userIndex, int (*callback)(void*, bool, int, int),
                                                 void* callbackData);
    static int InvitationSignInReturned(void* context, bool signedIn, int error, int userIndex);
    static int InvitationMustConnectReturned(void* context, int pad, C4JStorage::EMessageResult result);
    void UpdateInvitationChecks();
    NQRNetworkPlayer* GetPlayerFromStationIdAndLocalIdx(unsigned long long stationId, int localIndex);
    void RemoveConnectedStation(unsigned long long stationId);
    void RemoveConnectedPlayer(NQRNetworkPlayer* player);
    void NonNetworkPlayerComplete(NQRNetworkPlayer* player, unsigned char smallId);
    void AddConnectedPlayer(NQRNetworkPlayer* player);
    void HandlePlayerJoined(NQRNetworkPlayer* player);
    void FindOrCreateNonNetworkPlayer(int slotIndex, int type, PlayerSyncData* syncData);
    void NetworkPlayerSmallIdAllocated(NQRNetworkPlayer* player, unsigned char smallId);
    void MapRoomSlotPlayers(int previousPlayerCount);
    void UpdateRoomSyncUIDsFromPlayers();
    void RemoteDataSend(unsigned long long stationId, unsigned char channel, const void* data, unsigned int size);
    void SyncRoomData();
    void RemoveRemotePlayersAndSync(unsigned long long stationId, int playerMask);
    void ClearSendQueue();
    void SetState(eNQRNetworkManagerInternalState state);
    void TickSendQueue();
    nn::pia::Result Pia_InitializeCommon(unsigned char* memory, unsigned int memorySize, unsigned long long traceFlags,
                                         bool enableCachedPrint, bool);
    nn::pia::Result Pia_InitializeInet();
    nn::pia::Result Pia_InitializeSession(const nn::pia::session::Session::Setting& setting);
    ConnectedStation* GetConnectedStation(unsigned long long stationId);
    void AddConnectedStation(unsigned long long stationId);
    void Pia_SignalLeaveSession();
    static void Pia_SessionCallback(nn::pia::session::Session::EventType eventType, unsigned long long stationId,
                                    void* context);
    bool NexFacadeLogin(unsigned int timeout);
    void InitialiseAfterOnline();
    void OnlineCheck();
    void FriendSearchTick();
    static void FriendPresenceLoginCallback(nn::Result result, void* context);
    void Session_CleanupSession();
    bool Inet_Logout();
    void Inet_FinalizeNetwork();
    void Session_FinalizePiaSession();
    void Inet_FinalizePiaInet();
    void Pia_FinalizeCommon();
    void Pia_Finalize();
    void Pia_Shutdown();
    void SetLastNetworkError(nn::pia::Result result);
    void UpdateApplicationConnected();
    bool Pia_CreateSession();
    bool canRunRateControlledEvent();
    bool Pia_JoinSession();
    bool Pia_JoinMatchmakeSession();
    bool Pia_LeaveSession();
    void Pia_SignalBrowseSession();
    bool Pia_BrowseSession();
    bool Pia_JoinRandomSession();
    void UpdateExternalRoomDataFromThread();
    void UpdateMatchmakingAttributes();
    bool Pia_StartupSession();
    void Pia_SignalCreateSession();
    void Pia_SignalJoinSession();
    void SetLocalPlayersAndSync();
    static void ClearIsHandlingBootInvite();
    static bool s_applicationConnected;
    static bool s_piaShutdownPending;
    void SetNQRPresenceInfoFromExtData(PresenceSyncInfo* presenceInfo, void* extData);
    void UpdateRichPresenceCustomData(void* extData, unsigned int extDataSize);
    static void MatchmakingExtDataUpdatedCallback(bool success, void* context);
    void RemoveAllPlayersOnClient();
    void ResetToIdle();
    void TickNexAsyncProcess();
    void SendJoinData();
    void NetworkPlayerConnectionComplete(NQRNetworkPlayer* player);
    void PollUnconnectedStations();
    bool AddRemotePlayersAndSync(unsigned long long stationId, ClientJoinData& joinData, EJoinFailedReason* reason);
    void SendJoinFailedData(unsigned long long stationId, unsigned int playerMask, EJoinFailedReason reason);
    void PollReceiveData();
    static void ShowErrorViewer();
    void RemoveLocalClientPlayers(int playerMask);
    void ErrorHandlingTick();
    void TryToConnectOnline();
    void TickFromThread();
    void RunTickThread();
    static int TickThreadProc(void* context);
    void MallocAndSetExtDataFromNQRPresenceInfo(void** extData, PresenceSyncInfo* presenceInfo);
    void UpdateFriendFromPresence(unsigned int principalId);
    static void UpdateFriendFromPresence_callback(unsigned int principalId, void* context);
    unsigned int GetFriendsFromPresence();
    static int GetFriendsThreadProc(void* context);
    void ClearRichPresenceCustomData();
    void UpdateRemotePlay();
    void SendLeaveData(unsigned int playerMask);
    static bool IsHandlingBootInvite();
    NQRNetworkPlayer* GetPlayerIfReady(NQRNetworkPlayer* player);
    static void SetAppSuspended(bool suspended);
    void SetApplicationDisconnected();
    void Pia_ShutdownForLan();

    void TickNexFacadeLogin();
    void UpdatePlayersFromRoomSyncUIDs();
    int GetOldMask(int mask);
    int GetAddedMask(int oldMask, int newMask);
    int GetRemovedMask(int oldMask, int newMask);
    void ForceErrorPoint(eNQRForceError error);
    static void RefreshChatAndContentRestrictionsReturned_HandleInvite(void* context);
    void ResetNEXAsyncContext();
    void NexFacadeLogout();
    void Pia_FinalizeInet();
    void Pia_SignalJoinMatchmakeSession();
    void Pia_SignalJoinRandomSession();
    static void Pia_CheckGetLocalAndHostStationId();
    void CheckApplicationStillConnected();
    const char* GetNexASyncProcessString(ENEXAsyncProcess process);

private:
    typedef std::map<std::wstring, std::wstring> PlayerNameMap;

    bool m_friendPresenceUpdated;
    C4JThreadImpl* m_tickThread;
    C4JEventImpl* m_tickEvent;
    nn::nex::AnyObjectHolder<nn::nex::Gathering, nn::nex::String>* m_gatheringHolder;
    PlayerNameMap m_miiNames;
    INQRNetworkManagerListener* m_listener;
    eNQRNetworkManagerInternalState m_internalState;
    eNQRNetworkManagerState m_state;
    bool m_nextIdleReasonIsFailure;
    bool m_host;
    bool m_sessionJoined;
    unsigned long long m_localStationId;
    unsigned long long m_hostStationId;
    int m_localPlayerCount;
    int m_localPlayersJoined;
    nn::pia::inet::NexSessionSearchCriteria m_searchCriteria;
    int m_unknownDC0;
    unsigned char m_nextSmallId;
    int m_unknownDC8;
    bool m_localGame;
    bool m_unknownDCD;
    int m_unknownDD0;
    bool m_unknownDD4;
    unsigned char m_unknownDD5[3];
    unsigned char m_unknownDD8[0xa0];
    int m_unknownE78;
    bool m_receiving;
    int m_signedInLive;
    bool m_unknownE84;
    bool m_matchmaking;
    OSFastMutex m_playerMutex;
    PlayerSyncData m_roomPlayers[8];
    void* m_externalData;
    unsigned int m_externalDataSize;
    unsigned char m_unknown1140[8];
    int* m_roomAttributes;
    int m_roomAttributeCount;
    int m_matchmakeAttributes[5];
    std::vector<NQRNetworkPlayer*> m_unconnectedPlayers;
    std::deque<SendQueueItem> m_sendQueue;
    OSFastMutex m_sendQueueMutex;
    NQRNetworkPlayer* m_roomSlotPlayers[8];
    unsigned char m_unknown11D4[0xc];
    int m_unknown11E0;
    unsigned char m_unknown11E4[4];
    bool m_unknown11E8;
    unsigned char m_unknown11E9[0x1b];
    unsigned int m_pendingSessionId;
    int m_pendingUserMask;
    unsigned char m_unknown120C[0x10];
    OSFastMutex m_playerConnectionMutex;
    eNQRNetworkManagerFriendSearchState m_friendSearchState;
    unsigned char m_unknown124C[4];
    bool m_unknown1250;
    unsigned char m_unknown1251[7];
    unsigned int m_friendPresenceCount;
    C4JThreadImpl* m_friendSearchThread;
    OSFastMutex m_friendSearchMutex;
    unsigned char m_unknown128C[4];
    void (*m_extDataCallback)(bool, void*);
    void* m_extDataCallbackData;
    void* m_extDataBuffer;
    std::vector<FriendSearchResult> m_friendSearchResults;
    OSFastMutex m_connectedPlayerMutex;
    std::vector<NQRNetworkPlayer*> m_players;
    OSFastMutex m_connectedStationMutex;
    std::vector<ConnectedStation> m_connectedStations;
    unsigned char m_unknown1324[4];
    nn::fp::GameMode m_presenceGameMode;
    bool m_unknown1354;
    nn::nex::ProtocolCallContext* m_nexCallContext;
    ENEXAsyncProcess m_nexAsyncProcess;
    std::deque<ENEXAsyncProcess> m_nexAsyncProcessQueue;
    int m_nexLoginState;
    int m_unknown1378;
    bool m_unknown137C;
    bool m_updateRequested;
    long long m_lastSessionUpdateTime;
    bool m_createRequested;
    bool m_joinRequested;
    bool m_joinMatchmakeRequested;
    bool m_leaveRequested;
    bool m_browseRequested;
    bool m_randomJoinRequested;
    unsigned int m_unreliableProtocolHandle;
    unsigned int m_reliableProtocolHandle;
    unsigned char m_unknown1398[8];
    bool m_unknown13A0;
    int m_inviteConnectPromptState;
    int m_pendingAttributeIndex;
    unsigned int m_pendingAttributeValue;
    unsigned long long m_lastRateControlledEvent;
    void* m_matchmakingExtData;
    unsigned char m_gameMode;
};

extern bool g_bGameBootInvitePending;
extern bool g_bHandlingGameBootInvite;
extern unsigned int g_uiGameBootInvitePrincipalId;

#endif
