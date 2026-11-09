#ifndef GAME_NETWORK_CPLATFORMNETWORKMANAGER_HPP
#define GAME_NETWORK_CPLATFORMNETWORKMANAGER_HPP

#include <string>
#include <vector>
#include "game/minigame/EMiniGameId.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GameSessionData.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/PlayerUID.hpp"

class CGameNetworkManager;
class ClientboundPreLoginPacket;
namespace nn
{
namespace fp
{
struct FriendPresence;
}
}

struct FriendSessionSearchResult
{
    unsigned int m_sessionId;
    unsigned int m_ownerPrincipalId;
    unsigned int m_ownerLocalFriendIndex;
    char m_ownerName[17];
    void* m_externalData;
};

struct FriendSessionInfo
{
    unsigned int m_sessionId;
    FriendSessionSearchResult m_searchResult;
    wchar_t* m_displayName;
    unsigned char m_displayNameLength;
    bool m_sessionFlag;
    GameSessionData m_gameSessionData;
    bool m_statusFlag;

    FriendSessionInfo() : m_gameSessionData()
    {
        m_displayNameLength = 0;
        m_statusFlag = false;
        m_displayName = 0;
        m_sessionFlag = false;
    }
};

class CPlatformNetworkManager
{
public:
    enum eJoinFailedReason
    {
        eJoinFailedReason_SessionFull,
        eJoinFailedReason_NotAllowedOnline,
        eJoinFailedReason_SessionUnavailable,
        eJoinFailedReason_ConnectionLost
    };

    static CPlatformNetworkManager* Create();

    virtual void Initialise(CGameNetworkManager* pGameNetworkManager, int iMaxChunkPackets) = 0;
    virtual void Terminate() = 0;
    virtual int GetJoiningReadyPercentage() = 0;
    virtual int CorrectErrorIDS(int iErrorIDS) = 0;
    virtual void DoWork() = 0;
    virtual int GetPlayerCount() = 0;
    virtual int GetOnlinePlayerCount() = 0;
    virtual int GetLocalPlayerMask(int iUserIndex) = 0;
    virtual bool AddLocalPlayerByUserIndex(int iUserIndex) = 0;
    virtual bool RemoveLocalPlayerByUserIndex(int iUserIndex) = 0;
    virtual bool AddLocalPlayerByPlayerUID(PlayerUID uid) = 0;
    virtual INetworkPlayer* GetLocalPlayerByUserIndex(int iUserIndex) = 0;
    virtual INetworkPlayer* GetPlayerByIndex(int iIndex) = 0;
    virtual INetworkPlayer* GetPlayerByXuid(PlayerUID uid) = 0;
    virtual INetworkPlayer* GetPlayerBySmallId(unsigned char ucSmallId) = 0;
    virtual bool ShouldMessageForFullSession() = 0;
    virtual INetworkPlayer* GetHostPlayer() = 0;
    virtual bool IsHost() = 0;
    virtual bool JoinGameFromInviteInfo(int iUserIndex, int iUserIndexMask,
                                        const nn::fp::FriendPresence* pFriendPresence) = 0;
    virtual bool LeaveGame(bool bError) = 0;
    virtual bool IsInSession() = 0;
    virtual bool IsInGameplay() = 0;
    virtual bool IsReadyToPlayOrIdle() = 0;
    virtual bool IsInStatsEnabledSession() = 0;
    virtual bool IsMatchmakingEnabled() = 0;
    virtual bool IsHandlingBootInvite() = 0;
    virtual bool SessionHasSpace(unsigned int uiExtraPlayers) = 0;
    virtual bool SessionHasMii(int iUserIndex) = 0;
    virtual void SendInviteGUI(int iUserIndex) = 0;
    virtual bool IsAddingPlayer() = 0;
    virtual void HostGame(int iUserIndexMask, bool bOnlineGame, bool bPrivateGame, unsigned char ucGameMode,
                          EMiniGameId eMiniGameId, unsigned int uiWorldId, unsigned int* puiAttributes) = 0;
    virtual int JoinGame(FriendSessionInfo* pSessionInfo, int iUserIndex, int iUserIndexMask, bool bMatchmaking) = 0;
    virtual int FindAndJoinGame(int iUserIndexMask, int iSearchFlags, EMiniGameId eMiniGameId,
                                unsigned int uiAttributeCount, unsigned int* puiAttributeValues,
                                unsigned int* puiAttributeIds) = 0;
    virtual void CancelJoinGame();
    virtual bool SetLocalGame(bool bLocalGame) = 0;
    virtual bool IsLocalGame() = 0;
    virtual void SetPrivateGame(bool bPrivateGame) = 0;
    virtual bool IsPrivateGame() = 0;
    virtual bool IsLeavingGame() = 0;
    virtual void ResetLeavingGame() = 0;
    virtual void RegisterPlayerChangedCallback(int iUserIndex, void (*pCallback)(void*, INetworkPlayer*, bool),
                                               void* pCallbackData) = 0;
    virtual void UnRegisterPlayerChangedCallback(int iUserIndex, void (*pCallback)(void*, INetworkPlayer*, bool),
                                                 void* pCallbackData) = 0;
    virtual void HandleSignInChange() = 0;
    virtual bool _RunNetworkGame() = 0;
    virtual bool _LeaveGame(bool bError, bool bEndSession) = 0;
    virtual void _HostGame(int iUserIndexMask, unsigned char ucGameMode, EMiniGameId eMiniGameId,
                           unsigned int uiWorldId, unsigned int* puiAttributes) = 0;
    virtual bool _StartGame() = 0;
    virtual void UpdateAndSetGameSessionData(INetworkPlayer* pPlayer) = 0;
    virtual bool RemoveLocalPlayer(INetworkPlayer* pPlayer) = 0;
    virtual void SystemFlagAddPlayer(INetworkPlayer* pPlayer) = 0;
    virtual void SystemFlagSet(INetworkPlayer* pPlayer, int iFlag) = 0;
    virtual bool SystemFlagGet(INetworkPlayer* pPlayer, int iFlag) = 0;
    virtual void SystemFlagClearAll(INetworkPlayer* pPlayer) = 0;
    virtual std::wstring GatherStats() = 0;
    virtual std::wstring GatherRTTStats() = 0;
    virtual void SetSessionTexturePackParentId(int iTexturePackParentId) = 0;
    virtual void SetSessionSubTexturePackId(int iSubTexturePackId) = 0;
    virtual void SetSessionGameModeId(int iGameModeId) = 0;
    virtual void Notify(int iNotification, unsigned long ulParam) = 0;
    virtual std::vector<FriendSessionInfo*>* GetSessionList(int iUserIndex, int iMaxSessions, bool bFriendsOnly) = 0;
    virtual bool GetGameSessionInfo(int iUserIndex, unsigned int uiSessionId, FriendSessionInfo* pSessionInfo) = 0;
    virtual void SetSessionsUpdatedCallback(void (*pCallback)(void*), void* pCallbackData) = 0;
    virtual void GetFullFriendSessionInfo(FriendSessionInfo* pSessionInfo, void (*pCallback)(bool, void*),
                                          void* pCallbackData) = 0;
    virtual void ForceFriendsSessionRefresh() = 0;
    virtual void ClearFriendsSessions() = 0;
    virtual bool AlreadyInSession(const nn::fp::FriendPresence* pFriendPresence) = 0;
    virtual bool GetAgeRestricted() = 0;
    virtual bool usingAdhocMode() = 0;
    virtual bool setAdhocMode(bool bAdhocMode) = 0;
    virtual void startAdhocMatching() = 0;
    virtual bool isAdhocConnected() = 0;
    virtual bool connectToAdhoc() = 0;
    virtual bool isAdhocModeChangeInProgress() = 0;
    virtual void AttemptAdhocSignIn(int (*pCallback)(void*, bool, int), void* pCallbackData, bool bShowProgress) = 0;
    virtual bool setLanMode(bool bLanMode) = 0;
    virtual bool usingLanMode() = 0;
    virtual bool isLanConnected() = 0;
    virtual bool isLanModeChangeInProgress() = 0;
    virtual bool checkValidInviteData(const nn::fp::FriendPresence* pFriendPresence) = 0;
    virtual void SetExitAndJoinFromInviteInProgress(bool bInProgress) = 0;
    virtual void CancelDisconnectFromExitAndJoin() = 0;
    virtual void FakeLocalPlayerJoined();
    virtual std::wstring GetDisplayNameByGamertag(std::wstring gamertag) = 0;
    virtual int GetOutstandingAckCount(INetworkPlayer* pPlayer) = 0;
    virtual void TryToConnectOnline(int (*pCallback)(void*, bool, int), void* pCallbackData, bool bShowProgress) = 0;
    virtual void AttemptPSNSignIn(int (*pCallback)(void*, bool, int), void* pCallbackData, bool bShowProgress,
                                  int iUserIndex) = 0;
    virtual void RecvInviteGUI() = 0;
    virtual bool UpdateInviteData(void* pInviteData) = 0;
    virtual void CheckFriendsCanPlay(int iUserIndex, int& iFriendCount,
                                     boost::shared_ptr<ClientboundPreLoginPacket> pPreLoginPacket, int iTitleVersion,
                                     int& iCanPlayCount, int& iCannotPlayCount, int& iBlockedCount,
                                     int& iUnknownCount) = 0;
};
#endif
