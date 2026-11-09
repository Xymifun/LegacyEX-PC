#ifndef PLATFORM_WIIU_CPLATFORMNETWORKMANAGERNINTENDO_HPP
#define PLATFORM_WIIU_CPLATFORMNETWORKMANAGERNINTENDO_HPP

#include <string>
#include <vector>

#include "game/core/GameSessionData.hpp"
#include "game/network/CPlatformNetworkManager.hpp"
#include "game/network/INQRNetworkManagerListener.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"

class CGameNetworkManager;
class NQRNetworkManager_WiiU;
class NQRNetworkPlayer;

namespace nn
{
namespace fp
{
struct FriendPresence;
}
}

class CPlatformNetworkManagerNintendo : public CPlatformNetworkManager, public INQRNetworkManagerListener
{
public:
    class PlayerFlags
    {
    public:
        PlayerFlags(INetworkPlayer* player, unsigned int flagCount);
        ~PlayerFlags();
        INetworkPlayer* m_player;
        unsigned char* m_flags;
        unsigned int m_flagCount;
    };

    CPlatformNetworkManagerNintendo();

    static INetworkPlayer* getNetworkPlayer(NQRNetworkPlayer* player);
    INetworkPlayer* addNetworkPlayer(NQRNetworkPlayer* player);
    bool isSystemPrimaryPlayer(NQRNetworkPlayer* player);
    void SystemFlagRemovePlayer(INetworkPlayer* player);
    void removeNetworkPlayer(NQRNetworkPlayer* player);
    void UpdateMatchMakingData();
    void TickSearch();
    void SystemFlagReset();
    static int RemovePlayerOnSocketClosedThreadProc(void* data);

    virtual void HandleDataReceived(NQRNetworkPlayer* from, NQRNetworkPlayer* to, unsigned char* data,
                                    unsigned int size);
    virtual void HandlePlayerJoined(NQRNetworkPlayer* player);
    virtual void HandlePlayerLeaving(NQRNetworkPlayer* player);
    virtual void HandleStateChange(NQRNetworkManager::eNQRNetworkManagerState oldState,
                                   NQRNetworkManager::eNQRNetworkManagerState newState, bool error);
    virtual void HandleResyncPlayerRequest(NQRNetworkPlayer** players);
    virtual void HandleAddLocalPlayerFailed(int userIndex);
    virtual void HandleDisconnect(bool error);
    virtual void HandleInviteReceived(int userIndex, nn::fp::FriendPresence* presence);

    virtual void Initialise(CGameNetworkManager* manager, int maxSystemFlags);
    virtual void Terminate();
    virtual int GetJoiningReadyPercentage();
    virtual int CorrectErrorIDS(int errorIds);
    virtual void DoWork();
    virtual int GetPlayerCount();
    virtual int GetOnlinePlayerCount();
    virtual int GetLocalPlayerMask(int userIndex);
    virtual bool AddLocalPlayerByUserIndex(int userIndex);
    virtual bool RemoveLocalPlayerByUserIndex(int userIndex);
    virtual bool AddLocalPlayerByPlayerUID(PlayerUID uid);
    virtual INetworkPlayer* GetLocalPlayerByUserIndex(int userIndex);
    virtual INetworkPlayer* GetPlayerByIndex(int index);
    virtual INetworkPlayer* GetPlayerByXuid(PlayerUID uid);
    virtual INetworkPlayer* GetPlayerBySmallId(unsigned char smallId);
    virtual bool ShouldMessageForFullSession();
    virtual INetworkPlayer* GetHostPlayer();
    virtual bool IsHost();
    virtual bool JoinGameFromInviteInfo(int userIndex, int userIndexMask, const nn::fp::FriendPresence* presence);
    virtual bool LeaveGame(bool error);
    virtual bool IsInSession();
    virtual bool IsInGameplay();
    virtual bool IsReadyToPlayOrIdle();
    virtual bool IsInStatsEnabledSession();
    virtual bool IsMatchmakingEnabled();
    virtual bool IsHandlingBootInvite();
    virtual bool SessionHasSpace(unsigned int extraPlayers);
    virtual bool SessionHasMii(int userIndex);
    virtual void SendInviteGUI(int userIndex);
    virtual bool IsAddingPlayer();
    virtual void HostGame(int userIndexMask, bool onlineGame, bool privateGame, unsigned char gameMode,
                          EMiniGameId miniGameId, unsigned int worldId, unsigned int* attributes);
    virtual int JoinGame(FriendSessionInfo* sessionInfo, int userIndex, int userIndexMask, bool matchmaking);
    virtual int FindAndJoinGame(int userIndexMask, int searchFlags, EMiniGameId miniGameId, unsigned int attributeCount,
                                unsigned int* attributeValues, unsigned int* attributeIds);
    virtual bool SetLocalGame(bool localGame);
    virtual bool IsLocalGame();
    virtual void SetPrivateGame(bool privateGame);
    virtual bool IsPrivateGame();
    virtual bool IsLeavingGame();
    virtual void ResetLeavingGame();
    virtual void RegisterPlayerChangedCallback(int userIndex, void (*callback)(void*, INetworkPlayer*, bool),
                                               void* callbackData);
    virtual void UnRegisterPlayerChangedCallback(int userIndex, void (*callback)(void*, INetworkPlayer*, bool),
                                                 void* callbackData);
    virtual void HandleSignInChange();
    virtual bool _RunNetworkGame();
    virtual bool _LeaveGame(bool error, bool endSession);
    virtual void _HostGame(int userIndexMask, unsigned char gameMode, EMiniGameId miniGameId, unsigned int worldId,
                           unsigned int* attributes);
    virtual bool _StartGame();
    virtual void UpdateAndSetGameSessionData(INetworkPlayer* player);
    virtual bool RemoveLocalPlayer(INetworkPlayer* player);
    virtual void SystemFlagAddPlayer(INetworkPlayer* player);
    virtual void SystemFlagSet(INetworkPlayer* player, int flag);
    virtual bool SystemFlagGet(INetworkPlayer* player, int flag);
    virtual void SystemFlagClearAll(INetworkPlayer* player);
    virtual std::wstring GatherStats();
    virtual std::wstring GatherRTTStats();
    virtual void SetSessionTexturePackParentId(int id);
    virtual void SetSessionSubTexturePackId(int id);
    virtual void SetSessionGameModeId(int id);
    virtual void Notify(int notification, unsigned long param);
    virtual std::vector<FriendSessionInfo*>* GetSessionList(int userIndex, int maxSessions, bool friendsOnly);
    virtual bool GetGameSessionInfo(int userIndex, unsigned int sessionId, FriendSessionInfo* info);
    virtual void SetSessionsUpdatedCallback(void (*callback)(void*), void* callbackData);
    virtual void GetFullFriendSessionInfo(FriendSessionInfo* info, void (*callback)(bool, void*), void* callbackData);
    virtual void ForceFriendsSessionRefresh();
    virtual void ClearFriendsSessions();
    virtual bool AlreadyInSession(const nn::fp::FriendPresence* presence);
    virtual bool GetAgeRestricted();
    virtual bool usingAdhocMode();
    virtual bool setAdhocMode(bool mode);
    virtual void startAdhocMatching();
    virtual bool isAdhocConnected();
    virtual bool connectToAdhoc();
    virtual bool isAdhocModeChangeInProgress();
    virtual void AttemptAdhocSignIn(int (*callback)(void*, bool, int), void* callbackData, bool showProgress);
    virtual bool setLanMode(bool mode);
    virtual bool usingLanMode();
    virtual bool isLanConnected();
    virtual bool isLanModeChangeInProgress();
    virtual bool checkValidInviteData(const nn::fp::FriendPresence* presence);
    virtual void SetExitAndJoinFromInviteInProgress(bool inProgress);
    virtual void CancelDisconnectFromExitAndJoin();
    virtual std::wstring GetDisplayNameByGamertag(std::wstring gamertag);
    virtual int GetOutstandingAckCount(INetworkPlayer* player);
    virtual void TryToConnectOnline(int (*callback)(void*, bool, int), void* callbackData, bool showProgress);
    virtual void AttemptPSNSignIn(int (*callback)(void*, bool, int), void* callbackData, bool showProgress,
                                  int userIndex);
    virtual void RecvInviteGUI();
    virtual bool UpdateInviteData(void* inviteData);
    virtual void CheckFriendsCanPlay(int userIndex, int& friendCount,
                                     boost::shared_ptr<ClientboundPreLoginPacket> packet, int titleVersion,
                                     int& canPlayCount, int& cannotPlayCount, int& blockedCount, int& unknownCount);

private:
    bool m_adhocMode;
    bool m_enteringAdhocMode;
    bool m_leavingAdhocMode;
    bool m_lanMode;
    bool m_enteringLanMode;
    bool m_leavingLanMode;
    bool m_worldSession;
    bool m_reservedMode;
    NQRNetworkManager_WiiU* m_onlineNetworkManager;
    NQRNetworkManager* m_networkManager;
    NQRNetworkPlayer* m_pendingPlayer;
    std::vector<NQRNetworkPlayer*> m_systemPrimaryPlayers;
    bool m_leavingGame;
    bool m_leavePending;
    bool m_leaveError;
    bool m_suppressConnections;
    bool m_endSession;
    bool m_localGame;
    bool m_privateGame;
    bool m_reservedSession;
    unsigned int m_maxSystemFlags;
    bool m_matchmakingSession;
    GameSessionData m_sessionData;
    unsigned int m_matchMakingData[5];
    unsigned char m_sessionMode;
    CGameNetworkManager* m_gameNetworkManager;
    void (*m_playerChangedCallbacks[4])(void*, INetworkPlayer*, bool);
    void* m_playerChangedCallbackData[4];
    std::vector<PlayerFlags*> m_playerFlags;
    float m_lastPlayerChangeTime;
    std::vector<FriendSessionInfo*> m_friendSessions;
    unsigned int m_lastFriendSearchTime;
    int m_friendSearchCount;
    NQRNetworkManager::SessionSearchResult* m_friendSearchResults;
    unsigned int m_friendSearchState;
    bool m_friendSearchInProgress;
    void* m_sessionsUpdatedCallbackData;
    void (*m_sessionsUpdatedCallback)(void*);
    unsigned int m_sessionListState;
    std::vector<INetworkPlayer*> m_networkPlayers;
};

bool CommerceAppletIsExecuting();
void CommerceAppletFinish();
int CommerceAppletGetReturnCode();

#endif
