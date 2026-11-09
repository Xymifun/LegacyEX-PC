#ifndef PLATFORM_PC_PCNETWORKSESSION_HPP
#define PLATFORM_PC_PCNETWORKSESSION_HPP

#include <deque>
#include <string>
#include <vector>

#include "game/core/CriticalSection.hpp"
#include "game/core/GameSessionData.hpp"
#include "platform/pc/PcContent.hpp"
#include "platform/pc/PcLanTransport.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"

class C4JEventImpl;
class C4JThreadImpl;
class INQRNetworkManagerListener;
class NQRNetworkPlayer;

class PcNetworkSession : public NQRNetworkManager
{
public:
    static const int kMaxRoomPlayers = 25;

    PcNetworkSession(INQRNetworkManagerListener* listener, bool lanEnabled = false);
    virtual ~PcNetworkSession();

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
    virtual bool JoinRoom(SessionSearchResult* result, int localUserMask);
    virtual bool JoinRoom(unsigned int sessionId, int localUserMask, const PresenceSyncInfo* presence);
    virtual bool JoinRoom(int localUserMask, const nn::fp::FriendPresence* presence);
    virtual bool FriendPresenceUpdatedRoomList();
    virtual SessionSearchResult* GetFriendRoomSearchResults(int* count);
    virtual void GetExtDataForRoom(unsigned int sessionId, void* extData, void (*callback)(bool, void*),
                                   void* callbackData);
    virtual bool FindAndJoinRoom(int localUserMask, unsigned int searchParamCount, CMatchMakeSearchParam* searchParams,
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

    bool JoinRoomByAddress(unsigned int address, unsigned short port, int localUserMask, unsigned int lobbyWorld = 0);
    static bool HostedSessionData(std::vector<unsigned char>* data);
    static bool IsLobbySession(unsigned int sessionId);
    static bool IsFriendSession(unsigned int sessionId);
    static bool JoinedDedicatedServer();
    static int VoicePeerCount();
    static void SetGamePort(int port);
    static int CountMatchmakingGames(unsigned char miniGameId);
    static bool AcceptRelayedConnection(PcOsSocket socket, unsigned int address, const PcVerifiedIdentities* verified);
    static bool VerifiedJoinUid(int smallId, PlayerUID* uid);
    static bool VerifiedUuid(int smallId, unsigned char uuid[16]);
    static bool AddDirectHost(unsigned int address, unsigned short port);
    static bool WatchDirectHost(unsigned int address, unsigned short port, bool anyState = false);
    static PcLanDiscovery::ProbeStatus DirectHostStatus(unsigned int address, unsigned short port);
    static unsigned int DirectHostSessionId(unsigned int address, unsigned short port);
    static void DescribeState(char* text, unsigned int size);
    static void SetPublicWorldTitle(const wchar_t* title);
    static std::wstring WorldTitle(unsigned int sessionId);
    static std::wstring ContentLabel(unsigned int sessionId);
    static void TickRefusalMessage();

protected:
    enum EJoinFailedReason
    {
        eJoinFailedReason_GameFull,
        eJoinFailedReason_MiiInUse,
        eJoinFailedReason_MismatchedContent
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

    struct SendQueueItem
    {
        unsigned long long stationId;
        std::vector<unsigned char> data;
    };

    struct SearchResult
    {
        unsigned int sessionId;
        unsigned int address;
        unsigned short port;
        unsigned int lobbyWorld;
        GameSessionData* sessionData;
        std::vector<unsigned int> attributes;
        bool friendHost;
        std::wstring worldTitle;
        PcContentIdentity content;
    };

    void SetState(eNQRNetworkManagerInternalState state);
    void TickFromThread();
    void ResetToIdle();
    void MapRoomSlotPlayers();
    NQRNetworkPlayer* FindPlayer(unsigned long long stationId, unsigned int localIndex);
    NQRNetworkPlayer* CreateRoomPlayer(int slotIndex, NQRNetworkPlayer::eNQRNetworkPlayerType type);
    void HandlePlayerJoined(NQRNetworkPlayer* player);
    void NetworkPlayerSmallIdAllocated(NQRNetworkPlayer* player, unsigned char smallId);
    void FillLocalSlot(PlayerSyncData& slot, int userIndex);
    unsigned int AllocateSmallId(const PlayerSyncData* filling);
    void MakeNetworkNameUnique(PlayerSyncData& slot);
    void RemovePlayer(NQRNetworkPlayer* player);

    void QueueSend(unsigned long long stationId, unsigned char channel, const void* data, unsigned int size);
    void TickSendQueue();
    void ClearSendQueue();
    void SyncRoomData();
    void SendJoinData(unsigned int playerMask);
    void SendLeaveData(unsigned int playerMask);
    void SendJoinFailedData(unsigned long long stationId, unsigned int playerMask, EJoinFailedReason reason);
    bool AddRemotePlayersAndSync(unsigned long long stationId, const ClientJoinData& joinData,
                                 EJoinFailedReason* reason);
    void RemoveRemotePlayersAndSync(unsigned long long stationId, unsigned int playerMask);
    void RemoveLocalClientPlayers(unsigned int playerMask);
    void PollTransportEvents();
    void PollReceiveData();
    void TickVoice();
    void HandleVoice(unsigned long long stationId, const unsigned char* data, unsigned int size);
    void HandleGameData(unsigned long long stationId, unsigned char channel, const unsigned char* packet,
                        unsigned int size);
    void HandleJoinFailed(unsigned int playerMask, EJoinFailedReason reason);
    bool HostContentMatches(unsigned long long hostStationId);
    static void SetRefusalMessage(int titleId, int messageId);
    bool GuestContentMatches(unsigned long long stationId, unsigned int playerMask);
    void StartLanHosting();
    void StopLan();
    void TickSearch();
    void RefreshSearchResults();
    static bool IsSameGameAsProbedSession(const PcLanDiscovery::Session& session,
                                          const std::vector<PcLanDiscovery::Session>& sessions);
    static bool IsSameGame(const GameSessionData& first, const GameSessionData& second);
    static bool MatchesSearch(const SearchResult& result, unsigned char gameMode, unsigned int paramCount,
                              const CMatchMakeSearchParam* params);
    void ClearSearchResults();
    void UpdateRemotePlay();
    void PublishSessionData();
    static void ReadSessionDataTail(const std::vector<unsigned char>& data, SearchResult* result);

    static int TickThreadProc(void* argument);

    static PcNetworkSession* s_lanSession;

    INQRNetworkManagerListener* m_listener;
    eNQRNetworkManagerState m_state;
    eNQRNetworkManagerInternalState m_internalState;
    bool m_lanEnabled;
    bool m_host;
    bool m_localGame;
    bool m_receiving;
    bool m_matchmaking;
    volatile bool m_terminate;
    unsigned long long m_localStationId;
    unsigned long long m_hostStationId;
    bool m_joinedDedicated;
    int m_nextSmallId;
    const int* m_roomAttributes;
    int m_roomAttributeCount;
    void* m_externalData;
    int m_externalDataSize;
    std::wstring m_worldTitle;
    unsigned int m_roomPlayerMask;
    int m_localPlayerCount;
    int m_localPlayersJoined;
    PlayerSyncData m_roomPlayers[kMaxRoomPlayers];
    NQRNetworkPlayer* m_roomSlotPlayers[kMaxRoomPlayers];
    std::vector<NQRNetworkPlayer*> m_players;
    OSFastMutex m_playerMutex;
    C4JEventImpl* m_tickEvent;
    C4JThreadImpl* m_tickThread;

    PcLanTransport m_transport;
    PcLanDiscovery m_discovery;
    std::deque<SendQueueItem> m_sendQueue;
    OSFastMutex m_sendQueueMutex;

    std::vector<SearchResult> m_searchResults;
    OSFastMutex m_searchMutex;
    bool m_searching;
    unsigned long long m_searchStartTime;
    unsigned int m_searchRevision;
    unsigned int m_lobbyRevision;
    bool m_searchResultsChanged;
    unsigned int m_extDataSessionId;
    void* m_extDataBuffer;
    void (*m_extDataCallback)(bool, void*);
    void* m_extDataCallbackData;
    volatile bool m_extDataRequested;
};

#endif
