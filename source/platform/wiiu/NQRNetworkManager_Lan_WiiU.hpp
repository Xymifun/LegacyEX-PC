#ifndef PLATFORM_WIIU_NQRNETWORKMANAGER_LAN_WIIU_HPP
#define PLATFORM_WIIU_NQRNETWORKMANAGER_LAN_WIIU_HPP

#include <deque>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <nn/fp/fp_cpp.h>

#include "game/core/CriticalSection.hpp"
#include "game/network/INQRNetworkManagerListener.hpp"
#include "nn/pia/Common.hpp"
#include "nn/pia/inet/NexSessionSearchCriteria.hpp"
#include "nn/pia/lan/LanSessionInfo.hpp"
#include "nn/pia/session/Session.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"

class C4JEventImpl;
class GameSessionData;
class C4JThread;
class C4JThreadImpl;


namespace nn
{
namespace fp
{
struct FriendPresence;
}
namespace nex
{
class ProtocolCallContext;
}
namespace pia
{
namespace session
{
class SessionSearchCriteria;
}
}
}

class NQRNetworkManager_Lan_WiiU : public NQRNetworkManager
{
public:
    enum ESaveTransferState
    {
        eSaveTransfer_Idle,
        eSaveTransfer_CreatingSession,
        eSaveTransfer_CreateSessionFailed,
        eSaveTransfer_WaitingForReceiver,
        eSaveTransfer_Transferring,
        eSaveTransfer_SendComplete,
        eSaveTransfer_Searching,
        eSaveTransfer_NoSessionFound,
        eSaveTransfer_Unused,
        eSaveTransfer_SessionFound,
        eSaveTransfer_Joining,
        eSaveTransfer_JoinFailed,
        eSaveTransfer_Joined,
        eSaveTransfer_ReceiveComplete,
        eSaveTransfer_ConnectionLost
    };

    enum NetworkType
    {
        eNetworkType_None,
        eNetworkType_Adhoc,
        eNetworkType_Online,
        eNetworkType_Local,
        eNetworkType_Lan
    };

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
        eNEXLoginState_Pending,
        eNEXLoginState_LoggedIn = 2,
        eNEXLoginState_LoggedOut
    };

    enum EJoinFailedReason
    {
        eJoinFailedReason_Full,
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
        unsigned int unknown0;
        unsigned int ownerPrincipalId;
        unsigned int unknown8;
        unsigned int ownerLocalFriendIndex;
        char ownerName[36];
        unsigned int sessionId;
        bool joinable;
        void* extData;
        GameSessionData* gameSessionData;
        nn::pia::lan::LanSessionInfo sessionInfo;
    };

    struct ConnectedStation
    {
        enum EStateFlag
        {
            eStateFlag_InCommunication = 1
        };

        unsigned long long stationId;
        int state;

        ConnectedStation(unsigned long long station) : stationId(station), state(0) {}
    };

    class ClientJoinData
    {
    public:
        ClientJoinData();

        unsigned int playerMask;
        PlayerUID playerUIDs[4];
        char onlineIds[4][21];
        wchar_t displayNames[4][36];
    };

    NQRNetworkManager_Lan_WiiU(INQRNetworkManagerListener* listener);
    virtual ~NQRNetworkManager_Lan_WiiU();

    virtual void Tick();
    virtual void Initialise();
    virtual void Terminate();
    virtual eNQRNetworkManagerState GetState();
    virtual bool IsHost();
    virtual bool IsReadyToPlayOrIdle();
    virtual bool IsInSession();
    virtual bool IsMatchmakingEnabled();
    virtual void CreateAndJoinRoom(int primaryUserIndex, int localUserMask, void* extData, int extDataSize,
                                   int* attributes, int attributeCount, bool localGame, unsigned char gameMode);
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
    void SetFriendSearchState(eNQRNetworkManagerFriendSearchState state);
    static int InvitationSignInReturned(void* context, bool signedIn, int error, int userIndex);
    static int InvitationMustConnectReturned(void* context, int pad, C4JStorage::EMessageResult result);
    NQRNetworkPlayer* GetPlayerFromStationIdAndLocalIdx(unsigned long long stationId, int localIndex);
    void RemoveConnectedStation(unsigned long long stationId);
    void RemoveConnectedPlayer(NQRNetworkPlayer* player);
    void NonNetworkPlayerComplete(NQRNetworkPlayer* player, unsigned char smallId);
    void AddConnectedPlayer(NQRNetworkPlayer* player);
    void HandlePlayerJoined(NQRNetworkPlayer* player);
    void FindOrCreateNonNetworkPlayer(int slotIndex, int type, PlayerSyncData* syncData);
    void NetworkPlayerSmallIdAllocated(NQRNetworkPlayer* player, unsigned char smallId);
    void MapRoomSlotPlayers(int roomSlotPlayerCount);
    void UpdateRoomSyncUIDsFromPlayers();
    void RemoteDataSend(unsigned long long stationId, unsigned char channel, const void* data, unsigned int size);
    void SyncRoomData();
    void RemoveRemotePlayersAndSync(unsigned long long stationId, int playerMask);
    void ClearSendQueue();
    void SetState(eNQRNetworkManagerInternalState state);
    void TickSendQueue();
    int InitialiseNetwork(NetworkType type);
    nn::pia::Result Pia_InitializeCommon(unsigned char* memory, unsigned int memorySize, unsigned long long traceFlags,
                                         bool enableCachedPrint, bool);
    nn::pia::Result Lan_InitializePiaLan();
    nn::pia::Result Pia_InitializeSession(const nn::pia::session::Session::Setting& setting);
    ConnectedStation* GetConnectedStation(unsigned long long stationId);
    void AddConnectedStation(unsigned long long stationId);
    void Pia_SignalLeaveSession();
    static void Pia_SessionCallback(nn::pia::session::Session::EventType eventType, unsigned long long stationId,
                                    void* context);
    void InitialiseAfterOnline();
    bool IsInitialisedAfterOnline();
    void OnlineCheck();
    void FriendSearchTick();
    void Session_CleanupSession();
    bool Inet_Logout();
    void Lan_CleanupPiaLan();
    void Inet_FinalizeNetwork();
    void Session_FinalizePiaSession();
    void Lan_FinalizePiaLan();
    void Inet_FinalizePiaInet();
    void Pia_FinalizeCommon();
    void Pia_Finalize();
    void Pia_Shutdown();
    void SetLastNetworkError(nn::pia::Result result);
    void Net_GetLocalAddress(unsigned int* address, unsigned int* subnetMask);
    nn::pia::Result Lan_StartupPiaLan();
    void UpdateApplicationConnected();
    bool Pia_CreateSession();
    bool canRunRateControlledEvent();
    bool Pia_JoinSession();
    bool Pia_JoinMatchmakeSession();
    bool Pia_LeaveSession();
    bool Pia_BrowseSession();
    bool Pia_JoinRandomSession();
    void UpdateExternalRoomDataFromThread();
    void UpdateMatchmakingAttributes();
    bool Pia_StartupSession();
    void Pia_SignalCreateSession();
    void Pia_SignalJoinSession();
    void SetLocalPlayersAndSync();
    static void ClearIsHandlingBootInvite();
    void SetNQRPresenceInfoFromExtData(PresenceSyncInfo* presenceInfo, void* extData);
    void UpdateRichPresenceCustomData(PresenceSyncInfo* presenceInfo);
    static void MatchmakingExtDataUpdatedCallback(bool success, void* context);
    void RemoveAllPlayersOnClient();
    void ResetToIdle();
    void TickNexAsyncProcess();
    void SendJoinData();
    void SendSaveTransferJoinData();
    void NetworkPlayerConnectionComplete(NQRNetworkPlayer* player);
    void PollUnconnectedStations();
    bool AddRemotePlayersAndSync(unsigned long long stationId, ClientJoinData& joinData, EJoinFailedReason* reason);
    void SendJoinFailedData(unsigned long long stationId, unsigned int playerMask, EJoinFailedReason reason);
    void SendSaveTransferAckPacket();
    void PollReceiveData();
    void SendSaveTransferDataPacket(void* data, int size);
    void TickSaveTransferData();
    static void ShowErrorViewer();
    void RemoveLocalClientPlayers(int playerMask);
    void ErrorHandlingTick();
    void TickFromThread();
    void RunTickThread();
    static int TickThreadProc(void* context);
    void MallocAndSetExtDataFromNQRPresenceInfo(void** extData, PresenceSyncInfo* presenceInfo);
    static int GetFriendsThreadProc(void* context);
    nn::pia::Result Session_BrowseSession(nn::pia::session::SessionSearchCriteria* criteria);
    unsigned int GetFriendsFromSessionBrowse();
    void ClearRichPresenceCustomData();
    void UpdateRemotePlay();
    void SendLeaveData(unsigned int playerMask);
    static bool IsHandlingBootInvite();
    NQRNetworkPlayer* GetPlayerIfReady(NQRNetworkPlayer* player);
    static void SetAppSuspended(bool suspended);
    void SignalInitialiseAfterOnline();
    unsigned int GetSaveTransferConnectionFromSessionBrowse();
    static int GetSaveTransferConnectionThreadProc(void* context);
    void Pia_ShutdownForOnline();
    void CreateSessionForSaveTransfer(int dataSize, void* data, int metadataSize, void* metadata, long long seed,
                                      unsigned int hostOptions, unsigned int texturePack, wchar_t* name);
    unsigned int FindSessionForSaveTransfer();
    void JoinSaveTransferConnection();

    void UpdatePlayersFromRoomSyncUIDs();
    int GetOldMask(int mask);
    int GetAddedMask(int oldMask, int newMask);
    int GetRemovedMask(int oldMask, int newMask);
    void ForceErrorPoint(eNQRForceError error);
    static void RefreshChatAndContentRestrictionsReturned_HandleInvite(void* context);
    void ResetNEXAsyncContext();
    void Pia_SignalJoinMatchmakeSession();
    void Pia_SignalJoinRandomSession();
    static void Pia_CheckGetLocalAndHostStationId();
    const char* GetNexASyncProcessString(ENEXAsyncProcess process);
    int Net_InitialiseNetworkInterface(NetworkType type);
    void Net_InitialiseFriends();
    static void ResumeFromSleep();
    bool IsNetworkConnected();
    static bool s_shutdownTickPending;

private:
    typedef std::map<std::wstring, std::wstring> PlayerNameMap;

    struct SaveTransferDescriptor
    {
        unsigned short kind;
        unsigned short version;
        unsigned int dataSize;
        unsigned int metadataSize;
        long long seed;
        unsigned int hostOptions;
        unsigned int texturePack;
        wchar_t name[64];
    };

    bool m_friendPresenceUpdated;
    C4JThread* m_tickThread;
    C4JEventImpl* m_tickEvent;
    C4JThreadImpl* m_connectThread;
    PlayerNameMap m_miiNames;
    NetworkType m_networkType;
    bool m_applicationConnected;
    bool m_online;
    INQRNetworkManagerListener* m_listener;
    eNQRNetworkManagerInternalState m_internalState;
    eNQRNetworkManagerState m_state;
    bool m_nextIdleReasonIsFailure;
    bool m_host;
    bool m_hostStationIdCached;
    unsigned long long m_localStationId;
    unsigned long long m_hostStationId;
    int m_localPlayerCount;
    int m_localPlayersJoined;
    nn::pia::inet::NexSessionSearchCriteria m_searchCriteria;
    int m_unknownDC8;
    unsigned char m_nextSmallId;
    int m_unknownDD0;
    bool m_localGame;
    bool m_unknownDD5;
    unsigned char m_unknownDD6[2];
    int m_roomDataUpdateCountdown;
    bool m_unknownDDC;
    unsigned char m_unknownDDD[0xe80 - 0xddd];
    int m_unknownE80;
    bool m_receiving;
    int m_signedInLive;
    bool m_unknownE8C;
    bool m_matchmaking;
    OSFastMutex m_playerMutex;
    PlayerSyncData m_roomPlayers[8];
    void* m_externalData;
    unsigned int m_externalDataSize;
    unsigned char m_unknown1148[8];
    int* m_roomAttributes;
    int m_roomAttributeCount;
    int m_matchmakeAttributes[5];
    std::vector<NQRNetworkPlayer*> m_unconnectedPlayers;
    std::deque<SendQueueItem> m_sendQueue;
    OSFastMutex m_sendQueueMutex;
    NQRNetworkPlayer* m_roomSlotPlayers[8];
    unsigned char m_unknown11DC[12];
    int m_unknown11E8;
    unsigned char m_unknown11EC[4];
    bool m_unknown11F0;
    unsigned char m_unknown11F1[27];
    unsigned int m_pendingSessionId;
    unsigned int m_roomPlayerMask;
    unsigned char m_unknown1214[16];
    OSFastMutex m_playerConnectionMutex;
    eNQRNetworkManagerFriendSearchState m_friendSearchState;
    unsigned char m_unknown1254[4];
    bool m_friendSearchBusy;
    unsigned char m_unknown1259[0x1260 - 0x1259];
    int m_friendSearchStatus;
    C4JThread* m_friendSearchThread;
    OSFastMutex m_friendSearchMutex;
    unsigned char m_unknown1294[4];
    void (*m_extDataCallback)(bool, void*);
    void* m_extDataCallbackData;
    void* m_extDataBuffer;
    std::vector<FriendSearchResult> m_friendSearchResults;
    OSFastMutex m_connectedPlayerMutex;
    std::vector<NQRNetworkPlayer*> m_players;
    OSFastMutex m_connectedStationMutex;
    std::vector<ConnectedStation> m_connectedStations;
    unsigned char m_unknown132C[0x1330 - 0x132c];
    nn::fp::GameMode m_presenceGameMode;
    bool m_unknown135C;
    nn::nex::ProtocolCallContext* m_nexCallContext;
    ENEXAsyncProcess m_nexAsyncProcess;
    std::deque<ENEXAsyncProcess> m_nexAsyncProcessQueue;
    ENEXLoginState m_nexLoginState;
    unsigned char m_unknown1380[1];
    bool m_updateRequested;
    unsigned char m_unknown1382[6];
    long long m_lastRoomDataUpdateTime;
    bool m_createRequested;
    bool m_joinRequested;
    bool m_joinMatchmakeRequested;
    bool m_leaveRequested;
    bool m_browseRequested;
    bool m_randomJoinRequested;
    bool m_saveTransferSearch;
    unsigned char m_unknown1397[0x1398 - 0x1397];

public:
    int m_saveTransferSize;
    unsigned char* m_saveTransferData;
    int m_saveTransferOffset;

private:
    bool m_saveTransferPacketPending;
    unsigned char m_unknown13A5[0x13a8 - 0x13a5];
    int m_saveTransferReceiveSize;
    unsigned char* m_saveTransferReceiveData;
    int m_saveTransferReceiveOffset;
    wchar_t m_saveTransferSenderName[36];

public:
    ESaveTransferState m_saveTransferState;

private:
    SaveTransferDescriptor* m_saveTransferDescriptor;
    unsigned char m_unknown1404[0x143c - 0x1404];

public:
    unsigned int m_saveTransferSessionId;

private:
    bool m_saveTransferSessionFound;
    unsigned char m_unknown1441[0x1450 - 0x1441];
    nn::pia::lan::LanSessionInfo m_saveTransferSessionInfo;
    unsigned int m_unreliableProtocolHandle;
    unsigned int m_reliableProtocolHandle;
    unsigned char m_unknown1BB0[8];
    bool m_inPiaSession;
    int m_unknown1BBC;
    unsigned char m_unknown1BC0[8];
    unsigned long long m_lastRateControlledEvent;
    int m_unknown1BD0;
    unsigned char m_gameMode;
};

extern NQRNetworkManager_Lan_WiiU* gLanNetworkManager;

#endif
