#include "platform/wiiu/CPlatformNetworkManagerNintendo.hpp"

#include <algorithm>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <nn/result.h>
#include <nn/fp/fp_cpp.h>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Socket.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundPreLoginPacket.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/NQRNetworkManager_Lan_WiiU.hpp"
#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "platform/wiiu/NetworkPlayerNintendo.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

#if defined(RECON_NATIVE)
#include "network_manager.hpp"
#endif
#ifdef MINECRAFT_PC
#include "platform/pc/PcNetworkSession.hpp"
#include "platform/pc/PcDedicated.hpp"
#endif

namespace nn
{
namespace ec
{
namespace applet
{
bool IsExecuting() asm("IsExecuting__Q3_2nn2ec6appletFv");
void Finish() asm("Finish__Q3_2nn2ec6appletFv");
int GetReturnCode() asm("GetReturnCode__Q3_2nn2ec6appletFv");
}
}
}

bool CommerceAppletIsExecuting()
{
    return nn::ec::applet::IsExecuting();
}

void CommerceAppletFinish()
{
    nn::ec::applet::Finish();
}

int CommerceAppletGetReturnCode()
{
    return nn::ec::applet::GetReturnCode();
}

NQRNetworkManager_Lan_WiiU* gLanNetworkManager;

namespace
{
CPlatformNetworkManagerNintendo* s_platformNetworkManager;
const eGameHostOption kGameHostOption3 = eGameHostOption_FriendsOfFriends;
const int cMaxSearchParams = 5;

inline int countLocalPlayers(int mask)
{
    int count = 0;
    if ((mask & 1) != 0)
        ++count;
    if ((mask & 2) != 0)
        ++count;
    if ((mask & 4) != 0)
        ++count;
    if ((mask & 8) != 0)
        ++count;
    return count;
}
}

CPlatformNetworkManagerNintendo::PlayerFlags::PlayerFlags(INetworkPlayer* player, unsigned int flagCount)
{
    unsigned int bits = flagCount + 7;
    unsigned int aligned = bits & ~7U;
    m_player = player;
    m_flags = new unsigned char[bits >> 3];
    memset(m_flags, 0, bits >> 3);
    m_flagCount = aligned;
}

CPlatformNetworkManagerNintendo::PlayerFlags::~PlayerFlags()
{
    delete[] m_flags;
}

CPlatformNetworkManagerNintendo::CPlatformNetworkManagerNintendo()
{
}

INetworkPlayer* CPlatformNetworkManagerNintendo::getNetworkPlayer(NQRNetworkPlayer* player)
{
    return player == 0 ? 0 : (INetworkPlayer*)player->GetCustomDataValue();
}

INetworkPlayer* NetworkPlayerNintendo_Create(NQRNetworkPlayer* player);

INetworkPlayer* CPlatformNetworkManagerNintendo::addNetworkPlayer(NQRNetworkPlayer* player)
{
    INetworkPlayer* networkPlayer = NetworkPlayerNintendo_Create(player);
    player->SetCustomDataValue((NQRCustomData)networkPlayer);
    m_networkPlayers.push_back(networkPlayer);
    return networkPlayer;
}

void CPlatformNetworkManagerNintendo::HandleDataReceived(NQRNetworkPlayer* from, NQRNetworkPlayer* to,
                                                         unsigned char* data, unsigned int size)
{
    if (m_networkManager->GetState() == NQRNetworkManager::eNQRNetworkManagerState_Ending)
        return;

    if (to->IsHost())
    {
        Socket* socket = getNetworkPlayer(from)->GetSocket();
        if (socket != 0)
            socket->pushDataToQueue(data, size, false);
    }
    else
    {
        Socket* socket = getNetworkPlayer(to)->GetSocket();
        if (socket != 0)
            socket->pushDataToQueue(data, size, true);
    }
}

bool CPlatformNetworkManagerNintendo::isSystemPrimaryPlayer(NQRNetworkPlayer* player)
{
    bool found = false;
    for (std::vector<NQRNetworkPlayer*>::iterator it = m_systemPrimaryPlayers.begin();
         it < m_systemPrimaryPlayers.end(); ++it)
    {
        if (*it == player)
        {
            found = true;
            break;
        }
    }
    return found;
}

void CPlatformNetworkManagerNintendo::SystemFlagRemovePlayer(INetworkPlayer* player)
{
    for (unsigned int index = 0; index < m_playerFlags.size(); ++index)
    {
        if (m_playerFlags[index]->m_player == player)
        {
            delete m_playerFlags[index];
            m_playerFlags[index] = m_playerFlags.back();
            m_playerFlags.pop_back();
            return;
        }
    }
}

void CPlatformNetworkManagerNintendo::removeNetworkPlayer(NQRNetworkPlayer* player)
{
    INetworkPlayer* networkPlayer = getNetworkPlayer(player);
    for (std::vector<INetworkPlayer*>::iterator it = m_networkPlayers.begin(); it != m_networkPlayers.end(); it++)
    {
        if (*it == networkPlayer)
        {
            m_networkPlayers.erase(it);
            return;
        }
    }
}

void CPlatformNetworkManagerNintendo::HandlePlayerJoined(NQRNetworkPlayer* player)
{
    CGameNetworkManager& manager = CGameNetworkManager::sInstance;
    bool createSocket = false;
    bool localConnection = false;
    INetworkPlayer* networkPlayer = addNetworkPlayer(player);

    if (player->IsLocal())
    {
        localConnection = true;
        if (player->IsHost())
            m_systemPrimaryPlayers.push_back(player);
        else
            createSocket = true;
    }
    else
    {
        if (!player->IsHost() && m_networkManager->IsHost())
            createSocket = true;
        if (m_networkManager->IsHost() && !m_suppressConnections)
        {
            bool found = false;
            for (std::vector<NQRNetworkPlayer*>::iterator it = m_systemPrimaryPlayers.begin();
                 it < m_systemPrimaryPlayers.end(); ++it)
            {
                if (player->IsSameSystem(*it))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                m_systemPrimaryPlayers.push_back(player);
        }
    }

    manager.PlayerJoining(networkPlayer);
    if (createSocket && !m_suppressConnections)
        manager.CreateSocket(networkPlayer, localConnection);

    if (m_networkManager->IsHost())
    {
        manager.UpdateAndSetGameSessionData(0);
        SystemFlagAddPlayer(networkPlayer);
    }

    for (int index = 0; index < 4; ++index)
        if (m_playerChangedCallbacks[index] != 0)
            m_playerChangedCallbacks[index](m_playerChangedCallbackData[index], networkPlayer, false);

    for (int index = 0; index < 4; ++index)
        m_networkManager->GetLocalPlayerByUserIndex(index);
    m_lastPlayerChangeTime = (float)gApp.getAppTime();
}

void CPlatformNetworkManagerNintendo::HandlePlayerLeaving(NQRNetworkPlayer* player)
{
    INetworkPlayer* networkPlayer = getNetworkPlayer(player);
    if (networkPlayer == 0)
        return;

    Socket* socket = networkPlayer->GetSocket();
    if (socket != 0)
    {
        if (m_networkManager->IsHost() && !m_suppressConnections)
            CGameNetworkManager::sInstance.CloseConnection(networkPlayer);
        networkPlayer->SetSocket(0);
    }

    if (m_networkManager->IsHost() && !m_suppressConnections)
    {
        if (isSystemPrimaryPlayer(player))
        {
            NQRNetworkPlayer* replacement = 0;
            for (unsigned int index = 0; index < m_networkManager->GetPlayerCount(); ++index)
            {
                NQRNetworkPlayer* candidate = m_networkManager->GetPlayerByIndex(index);
                if (candidate != 0 && candidate != player && candidate->IsSameSystem(player))
                {
                    replacement = candidate;
                    break;
                }
            }
            std::vector<NQRNetworkPlayer*>::iterator found = std::find(m_systemPrimaryPlayers.begin(),
                                                                       m_systemPrimaryPlayers.end(), player);
            if (found != m_systemPrimaryPlayers.end())
                m_systemPrimaryPlayers.erase(found);
            if (replacement != 0)
                m_systemPrimaryPlayers.push_back(replacement);
        }
        CGameNetworkManager::sInstance.UpdateAndSetGameSessionData(networkPlayer);
        SystemFlagRemovePlayer(networkPlayer);
    }

    CGameNetworkManager::sInstance.PlayerLeaving(networkPlayer);
    for (int index = 0; index < 4; ++index)
        if (m_playerChangedCallbacks[index] != 0)
            m_playerChangedCallbacks[index](m_playerChangedCallbackData[index], networkPlayer, true);

    if (m_networkManager->GetState() == NQRNetworkManager::eNQRNetworkManagerState_Playing)
    {
        for (int index = 0; index < 4; ++index)
            m_networkManager->GetLocalPlayerByUserIndex(index);
        m_lastPlayerChangeTime = (float)gApp.getAppTime();
    }
    removeNetworkPlayer(player);
}

void CPlatformNetworkManagerNintendo::HandleStateChange(NQRNetworkManager::eNQRNetworkManagerState oldState,
                                                        NQRNetworkManager::eNQRNetworkManagerState newState, bool error)
{
    if (newState == NQRNetworkManager::eNQRNetworkManagerState_Hosting)
    {
        m_leavingGame = false;
        m_leavePending = false;
        m_suppressConnections = false;
        CGameNetworkManager::sInstance.StateChange_AnyToHosting();
    }
    else if (newState == NQRNetworkManager::eNQRNetworkManagerState_Joining)
    {
        m_leavingGame = false;
        m_leavePending = false;
        m_suppressConnections = false;
        CGameNetworkManager::sInstance.StateChange_AnyToJoining(false);
    }
    else if (newState == NQRNetworkManager::eNQRNetworkManagerState_Idle
             && oldState == NQRNetworkManager::eNQRNetworkManagerState_Joining)
    {
        if (error)
            CGameNetworkManager::sInstance.StateChange_JoiningToIdle(eJoinFailedReason_ConnectionLost);
        else
            CGameNetworkManager::sInstance.StateChange_JoiningToIdle(eJoinFailedReason_SessionUnavailable);
    }
    else if (newState == NQRNetworkManager::eNQRNetworkManagerState_Idle
             && oldState == NQRNetworkManager::eNQRNetworkManagerState_Hosting)
    {
        m_leavingGame = true;
    }
    else if (newState == NQRNetworkManager::eNQRNetworkManagerState_Starting)
    {
        m_lastPlayerChangeTime = (float)gApp.getAppTime();
        CGameNetworkManager::sInstance.StateChange_AnyToStarting();
    }
    else if (newState == NQRNetworkManager::eNQRNetworkManagerState_Ending)
    {
        CGameNetworkManager::sInstance.StateChange_AnyToEnding(
            oldState == NQRNetworkManager::eNQRNetworkManagerState_Playing, false);
        if (m_networkManager->IsHost())
            m_leavingGame = true;
    }
    else if (error && newState == NQRNetworkManager::eNQRNetworkManagerState_Idle
             && oldState == NQRNetworkManager::eNQRNetworkManagerState_Ending)
    {
        gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NoSuitableGame);
    }
}

void CPlatformNetworkManagerNintendo::UpdateMatchMakingData()
{
    unsigned int data = m_sessionData.m_version;
    if (m_matchmakingSession)
        data |= 0x20000;
    if (m_sessionData.m_bGameStarted)
        data |= 0x10000;
    data |= (unsigned int)m_sessionMode << 24;
    m_matchMakingData[0] = data;
    m_matchMakingData[1] = CGameNetworkManager::sInstance.GetMaxPlayers(&m_sessionData) - m_sessionData.m_numPlayers;
}

void CPlatformNetworkManagerNintendo::HandleResyncPlayerRequest(NQRNetworkPlayer** players)
{
    m_sessionData.m_numPlayers = 0;
    for (int index = 0; index < GameSessionData::MAX_SESSION_PLAYERS; ++index)
    {
#ifdef MINECRAFT_PC
        if (players[index] != 0 && PcIsDedicatedServer() && players[index]->IsHost())
        {
            memset(m_sessionData.m_playerGamertags[index].m_name, 0, GameSessionData::MAX_GAMERTAG_LENGTH);
            continue;
        }
#endif
        if (players[index] != 0)
        {
            GameSessionData::Gamertag gamertag;
            strncpy(gamertag.m_name, players[index]->GetNetworkName(), GameSessionData::MAX_GAMERTAG_LENGTH);
            m_sessionData.m_playerGamertags[index] = gamertag;
            ++m_sessionData.m_numPlayers;
        }
        else
        {
            memset(m_sessionData.m_playerGamertags[index].m_name, 0, GameSessionData::MAX_GAMERTAG_LENGTH);
        }
    }
    UpdateMatchMakingData();
    m_networkManager->UpdateExternalRoomData();
}

void CPlatformNetworkManagerNintendo::HandleAddLocalPlayerFailed(int userIndex)
{
    CGameNetworkManager::sInstance.AddLocalPlayerFailed(userIndex, false);
}

void CPlatformNetworkManagerNintendo::HandleDisconnect(bool error)
{
    CGameNetworkManager::sInstance.HandleDisconnect(error);
}

void CPlatformNetworkManagerNintendo::HandleInviteReceived(int userIndex, nn::fp::FriendPresence* presence)
{
    CGameNetworkManager::sInstance.GameInviteReceived(userIndex, presence);
}

void CPlatformNetworkManagerNintendo::Initialise(CGameNetworkManager* manager, int maxSystemFlags)
{
    m_lanMode = false;
    m_enteringLanMode = false;
    m_leavingLanMode = false;
#if defined(RECON_NATIVE)
    m_onlineNetworkManager = 0;
    m_networkManager = 0;
#else
    m_onlineNetworkManager = new NQRNetworkManager_WiiU(this);
    gLanNetworkManager = new NQRNetworkManager_Lan_WiiU(this);
    m_networkManager = m_onlineNetworkManager;
    m_onlineNetworkManager->Initialise();
    gLanNetworkManager->Initialise();
#endif
    m_suppressConnections = false;
    m_lastFriendSearchTime = 0;
    m_endSession = true;
    m_maxSystemFlags = maxSystemFlags;
    m_sessionsUpdatedCallback = 0;
    s_platformNetworkManager = this;
    m_friendSearchCount = 0;
    m_leavingGame = false;
    m_playerChangedCallbacks[0] = 0;
    m_friendSearchResults = 0;
    m_leavePending = false;
    m_localGame = false;
    m_friendSearchInProgress = false;
    m_playerChangedCallbacks[1] = 0;
    m_gameNetworkManager = manager;
    m_playerChangedCallbacks[2] = 0;
    m_playerChangedCallbacks[3] = 0;
    m_sessionsUpdatedCallbackData = 0;
#if defined(RECON_NATIVE)
    m_adhocMode = false;
    m_enteringAdhocMode = false;
    m_leavingAdhocMode = false;
    m_worldSession = false;
    m_reservedMode = false;
    m_pendingPlayer = 0;
    m_leaveError = false;
    m_privateGame = false;
    m_reservedSession = false;
    m_matchmakingSession = false;
    m_sessionMode = 0;
    m_lastPlayerChangeTime = 0;
    m_friendSearchState = 0;
    m_sessionListState = 0;
    for (int index = 0; index < 4; ++index)
        m_playerChangedCallbackData[index] = 0;
    m_networkManager = new NativeNetworkManager(this);
    m_networkManager->Initialise();
#endif
}

void CPlatformNetworkManagerNintendo::Terminate()
{
    m_networkManager->Terminate();
}

int CPlatformNetworkManagerNintendo::GetJoiningReadyPercentage()
{
    return m_networkManager->GetJoiningReadyPercentage();
}

int CPlatformNetworkManagerNintendo::CorrectErrorIDS(int errorIds)
{
    return errorIds;
}

void CPlatformNetworkManagerNintendo::TickSearch()
{
    if (m_friendSearchInProgress || m_networkManager->FriendPresenceUpdatedRoomList())
    {
        if (!m_networkManager->FriendRoomManagerIsBusy())
        {
            delete m_friendSearchResults;
            m_friendSearchResults = m_networkManager->GetFriendRoomSearchResults(&m_friendSearchCount);
            m_friendSearchInProgress = false;
            if (m_sessionsUpdatedCallback != 0)
                m_sessionsUpdatedCallback(m_sessionsUpdatedCallbackData);
        }
    }
    else
    {
#ifdef MINECRAFT_PC
        const bool searchDue = m_lastFriendSearchTime == 0 || m_lastFriendSearchTime + 30000 < GetTickCount();
#else
        const bool searchDue = m_lastFriendSearchTime + 30000 < GetTickCount();
#endif
        if (!m_networkManager->FriendRoomManagerIsBusy() && m_sessionsUpdatedCallback != 0 && searchDue
            && m_networkManager->FriendRoomManagerSearch())
        {
            m_friendSearchInProgress = true;
            m_lastFriendSearchTime = GetTickCount();
        }
    }
}

void CPlatformNetworkManagerNintendo::DoWork()
{
    TickSearch();
    if (m_leavePending)
    {
        m_networkManager->LeaveRoom(m_endSession);
        m_leavePending = false;
    }
    m_networkManager->Tick();
    if (m_enteringLanMode)
        setLanMode(true);
    else if (m_leavingLanMode)
        setLanMode(false);
}

int CPlatformNetworkManagerNintendo::GetPlayerCount()
{
    return m_networkManager->GetPlayerCount();
}
int CPlatformNetworkManagerNintendo::GetOnlinePlayerCount()
{
    return m_networkManager->GetOnlinePlayerCount();
}
int CPlatformNetworkManagerNintendo::GetLocalPlayerMask(int userIndex)
{
    return 1 << userIndex;
}
bool CPlatformNetworkManagerNintendo::AddLocalPlayerByUserIndex(int userIndex)
{
    return m_networkManager->AddLocalPlayerByUserIndex(userIndex);
}

int CPlatformNetworkManagerNintendo::RemovePlayerOnSocketClosedThreadProc(void* data)
{
    INetworkPlayer* player = (INetworkPlayer*)data;
    Socket* socket = player->GetSocket();
    if (socket != 0)
    {
        socket->mCloseEvent->WaitForSignal(-1);
        player->SetSocket(0);
        delete socket;
    }
    return s_platformNetworkManager->RemoveLocalPlayer(player);
}

bool CPlatformNetworkManagerNintendo::RemoveLocalPlayerByUserIndex(int userIndex)
{
    NQRNetworkPlayer* nqrPlayer = m_networkManager->GetLocalPlayerByUserIndex(userIndex);
    INetworkPlayer* player = getNetworkPlayer(nqrPlayer);
    if (player != 0)
    {
        Socket* socket = player->GetSocket();
        if (socket != 0)
        {
            C4JThreadImpl* thread = new C4JThreadImpl(RemovePlayerOnSocketClosedThreadProc, player,
                                                      "RemovePlayerOnSocketClosed", 0);
            thread->SetProcessor(2);
            thread->Run();
        }
        else
        {
            return m_networkManager->RemoveLocalPlayerByUserIndex(userIndex);
        }
    }
    return true;
}

bool CPlatformNetworkManagerNintendo::AddLocalPlayerByPlayerUID(PlayerUID playerUid)
{
    (void)playerUid;
    const bool result = false;
    return result;
}
INetworkPlayer* CPlatformNetworkManagerNintendo::GetLocalPlayerByUserIndex(int userIndex)
{
    return getNetworkPlayer(m_networkManager->GetLocalPlayerByUserIndex(userIndex));
}
INetworkPlayer* CPlatformNetworkManagerNintendo::GetPlayerByIndex(int index)
{
    return getNetworkPlayer(m_networkManager->GetPlayerByIndex(index));
}
INetworkPlayer* CPlatformNetworkManagerNintendo::GetPlayerByXuid(PlayerUID uid)
{
    return getNetworkPlayer(m_networkManager->GetPlayerByXuid(uid));
}
INetworkPlayer* CPlatformNetworkManagerNintendo::GetPlayerBySmallId(unsigned char smallId)
{
    return getNetworkPlayer(m_networkManager->GetPlayerBySmallId(smallId));
}
std::wstring CPlatformNetworkManagerNintendo::GetDisplayNameByGamertag(std::wstring gamertag)
{
    return m_networkManager->GetDisplayNameByGamertag(gamertag);
}
bool CPlatformNetworkManagerNintendo::ShouldMessageForFullSession()
{
    return false;
}
INetworkPlayer* CPlatformNetworkManagerNintendo::GetHostPlayer()
{
    return getNetworkPlayer(m_networkManager->GetHostPlayer());
}
bool CPlatformNetworkManagerNintendo::IsHost()
{
    return m_networkManager->IsHost() && !m_suppressConnections;
}

bool CPlatformNetworkManagerNintendo::JoinGameFromInviteInfo(int, int userIndexMask,
                                                             const nn::fp::FriendPresence* presence)
{
    return m_networkManager->JoinRoom(userIndexMask, presence);
}

bool CPlatformNetworkManagerNintendo::LeaveGame(bool error)
{
    if (m_leavingGame)
        return true;
    m_leavingGame = true;
    NQRNetworkPlayer* nqrPlayer = m_networkManager->GetLocalPlayerByUserIndex(
        CGameNetworkManager::sInstance.GetPrimaryPad());
    INetworkPlayer* player = getNetworkPlayer(nqrPlayer);
    if (player != 0)
    {
        Socket* socket = player->GetSocket();
        if (socket != 0)
        {
            socket->mCloseEvent->WaitForSignal(-1);
            if (IsInSession())
            {
                player = getNetworkPlayer(
                    m_networkManager->GetLocalPlayerByUserIndex(CGameNetworkManager::sInstance.GetPrimaryPad()));
                player->SetSocket(0);
            }
            delete socket;
        }
    }
    if (m_networkManager->IsHost() && CGameNetworkManager::sInstance.ServerStoppedValid())
    {
        m_networkManager->EndGame();
        CGameNetworkManager::sInstance.ServerStoppedWait();
        CGameNetworkManager::sInstance.ServerStoppedDestroy();
    }
    return _LeaveGame(error, true);
}

bool CPlatformNetworkManagerNintendo::IsInSession()
{
    return m_networkManager->IsInSession();
}
bool CPlatformNetworkManagerNintendo::IsInGameplay()
{
    return m_networkManager->GetState() == NQRNetworkManager::eNQRNetworkManagerState_Playing;
}
bool CPlatformNetworkManagerNintendo::IsReadyToPlayOrIdle()
{
    return m_networkManager->IsReadyToPlayOrIdle();
}
bool CPlatformNetworkManagerNintendo::IsInStatsEnabledSession()
{
    return true;
}
bool CPlatformNetworkManagerNintendo::IsMatchmakingEnabled()
{
    return m_networkManager->IsMatchmakingEnabled();
}
bool CPlatformNetworkManagerNintendo::IsHandlingBootInvite()
{
    return NQRNetworkManager_WiiU::IsHandlingBootInvite();
}
bool CPlatformNetworkManagerNintendo::SessionHasSpace(unsigned int extraPlayers)
{
    return m_networkManager->SessionHasSpace(extraPlayers);
}
bool CPlatformNetworkManagerNintendo::SessionHasMii(int userIndex)
{
    return m_networkManager->SessionHasMii(userIndex);
}
void CPlatformNetworkManagerNintendo::SendInviteGUI(int)
{
    m_networkManager->SendInviteGUI();
}
bool CPlatformNetworkManagerNintendo::IsAddingPlayer()
{
    return false;
}

void CPlatformNetworkManagerNintendo::SystemFlagReset()
{
    for (unsigned int index = 0; index < m_playerFlags.size(); ++index)
        delete m_playerFlags[index];
    m_playerFlags.clear();
}

void CPlatformNetworkManagerNintendo::HostGame(int userIndexMask, bool onlineGame, bool privateGame,
                                               unsigned char gameMode, EMiniGameId miniGameId, unsigned int worldId,
                                               unsigned int* attributes)
{
    SetLocalGame(!onlineGame);
    SetPrivateGame(privateGame);
    SystemFlagReset();
    m_matchmakingSession = worldId != 0;
    userIndexMask |= GetLocalPlayerMask(CGameNetworkManager::sInstance.GetPrimaryPad());
    _HostGame(userIndexMask, gameMode, miniGameId, worldId, attributes);
}

int CPlatformNetworkManagerNintendo::JoinGame(FriendSessionInfo* sessionInfo, int userIndexMask, int, bool)
{
    if (static_cast<int>(sessionInfo->m_gameSessionData.m_numPlayers + countLocalPlayers(userIndexMask))
        > CGameNetworkManager::sInstance.GetMaxPlayers(&sessionInfo->m_gameSessionData))
        return eJoinFailedReason_SessionUnavailable;
    NQRNetworkManager::SessionSearchResult* result = static_cast<NQRNetworkManager::SessionSearchResult*>(
        static_cast<void*>(&sessionInfo->m_searchResult));
    return m_networkManager->JoinRoom(result, userIndexMask) ^ 1;
}

int CPlatformNetworkManagerNintendo::FindAndJoinGame(int userIndexMask, int, EMiniGameId miniGameId,
                                                     unsigned int attributeCount, unsigned int* attributeValues,
                                                     unsigned int* attributeIds)
{
    unsigned int localPlayerCount = countLocalPlayers(userIndexMask);
    CMatchMakeSearchParam search[cMaxSearchParams];
    search[0].id = 0;
    search[0].value = ((unsigned int)miniGameId << 24) | 0x30881;
    search[0].comparison = CMatchMakeSearchParam::eComparison_Equal;
    search[1].id = 1;
    search[1].value = localPlayerCount;
    search[1].comparison = CMatchMakeSearchParam::eComparison_GreaterThanOrEqual;
    for (int index = 0; index < attributeCount; ++index)
    {
        search[index + 2].id = attributeIds[index] + 2;
        search[index + 2].value = attributeValues[index];
        search[index + 2].comparison = CMatchMakeSearchParam::eComparison_Equal;
    }
    return m_networkManager->FindAndJoinRoom(userIndexMask, attributeCount + 2, search, (unsigned char)miniGameId) ^ 1;
}

bool CPlatformNetworkManagerNintendo::SetLocalGame(bool localGame)
{
    if (m_networkManager->GetState() == NQRNetworkManager::eNQRNetworkManagerState_Idle)
        m_localGame = localGame;
    return true;
}

bool CPlatformNetworkManagerNintendo::IsLocalGame()
{
    return m_localGame;
}
void CPlatformNetworkManagerNintendo::SetPrivateGame(bool privateGame)
{
    m_privateGame = privateGame;
}
bool CPlatformNetworkManagerNintendo::IsPrivateGame()
{
    return m_privateGame;
}
bool CPlatformNetworkManagerNintendo::IsLeavingGame()
{
    return m_leavingGame;
}
void CPlatformNetworkManagerNintendo::ResetLeavingGame()
{
    m_leavingGame = false;
}

void CPlatformNetworkManagerNintendo::RegisterPlayerChangedCallback(int userIndex,
                                                                    void (*callback)(void*, INetworkPlayer*, bool),
                                                                    void* callbackData)
{
    m_playerChangedCallbacks[userIndex] = callback;
    m_playerChangedCallbackData[userIndex] = callbackData;
}

void CPlatformNetworkManagerNintendo::UnRegisterPlayerChangedCallback(int userIndex,
                                                                      void (*)(void*, INetworkPlayer*, bool),
                                                                      void* callbackData)
{
    if (m_playerChangedCallbackData[userIndex] == callbackData)
    {
        m_playerChangedCallbacks[userIndex] = 0;
        m_playerChangedCallbackData[userIndex] = 0;
    }
}

void CPlatformNetworkManagerNintendo::HandleSignInChange()
{
    return;
}

bool CPlatformNetworkManagerNintendo::_RunNetworkGame()
{
#if defined(RECON_NATIVE)
    if (m_networkManager->IsHost())
#else
    if (IsHost())
#endif
    {
        m_networkManager->StartGame();
        m_sessionData.m_bGameStarted = true;
        UpdateMatchMakingData();
        m_networkManager->UpdateExternalRoomData();
        m_networkManager->SetPresenceDataStartHostingGame();
    }
    return true;
}

bool CPlatformNetworkManagerNintendo::_LeaveGame(bool error, bool endSession)
{
    m_endSession = endSession;
    m_leaveError = error;
    m_leavePending = true;
    m_leavingGame = true;
    return true;
}

void CPlatformNetworkManagerNintendo::_HostGame(int userIndexMask, unsigned char gameMode, EMiniGameId miniGameId,
                                                unsigned int worldId, unsigned int* attributes)
{
    memset(&m_sessionData, 0, sizeof(m_sessionData));
    m_sessionData.m_version = 0x881;
    m_sessionData.m_bPublicGame = !IsPrivateGame();
    m_sessionData.m_bGameStarted = false;
    m_sessionData.m_numPlayers = 0;
    m_sessionData.m_gameHostOptions = gApp.GetGameHostOption(eGameHostOption_All);
    m_sessionData.m_customGameModeHostOptions = Minecraft::GetInstance()->GetMiniGame()->GetHostOption(
        eCustomGameModeHostOption_All);
    m_sessionData.m_gameModeId = gameMode;
    m_sessionData.m_miniGameId = miniGameId;
    if ((userIndexMask & 1) != 0)
        ++m_sessionData.m_numPlayers;
    if ((userIndexMask & 2) != 0)
        ++m_sessionData.m_numPlayers;
    if ((userIndexMask & 4) != 0)
        ++m_sessionData.m_numPlayers;
    if ((userIndexMask & 8) != 0)
        ++m_sessionData.m_numPlayers;

    if (m_matchmakingSession)
    {
        for (int index = 0; index < worldId; ++index)
            m_matchMakingData[index + 2] = attributes[index];
        m_sessionMode = (unsigned char)miniGameId;
        UpdateMatchMakingData();
        m_networkManager->CreateAndJoinRoom(CGameNetworkManager::sInstance.GetPrimaryPad(), userIndexMask,
                                            &m_sessionData, sizeof(m_sessionData), (int*)m_matchMakingData, 5,
                                            IsLocalGame(), (unsigned char)miniGameId);
    }
    else
    {
        m_networkManager->CreateAndJoinRoom(CGameNetworkManager::sInstance.GetPrimaryPad(), userIndexMask,
                                            &m_sessionData, sizeof(m_sessionData), 0, 0, IsLocalGame(),
                                            (unsigned char)miniGameId);
    }
}

bool CPlatformNetworkManagerNintendo::_StartGame()
{
    m_networkManager->StartGame();
    return true;
}

void CPlatformNetworkManagerNintendo::UpdateAndSetGameSessionData(INetworkPlayer* player)
{
    if (m_leavingGame)
        return;
    INetworkPlayer* host = GetHostPlayer();
    char hostName[GameSessionData::MAX_GAMERTAG_LENGTH];
    wcstombs(hostName, host->GetOnlineName(), GameSessionData::MAX_GAMERTAG_LENGTH);
    m_sessionData.m_hostGamertag = *reinterpret_cast<GameSessionData::Gamertag*>(hostName);
    m_sessionData.m_gameHostOptions = gApp.GetGameHostOption(eGameHostOption_All);
#ifdef MINECRAFT_PC
    m_sessionData.m_pcDedicated = PcIsDedicatedServer() ? 1 : 0;
#endif
    if (player == 0)
    {
        UpdateMatchMakingData();
        m_networkManager->UpdateExternalRoomData();
    }
}

bool CPlatformNetworkManagerNintendo::RemoveLocalPlayer(INetworkPlayer* player)
{
    if (player->IsLocal())
        return m_networkManager->RemoveLocalPlayerByUserIndex(player->GetUserIndex());
    return true;
}

void CPlatformNetworkManagerNintendo::SystemFlagAddPlayer(INetworkPlayer* player)
{
    PlayerFlags* flags = new PlayerFlags(player, m_maxSystemFlags);
    for (unsigned int index = 0; index < m_playerFlags.size(); ++index)
    {
        if (player->IsSameSystem(m_playerFlags[index]->m_player))
        {
            memcpy(flags->m_flags, m_playerFlags[index]->m_flags, m_playerFlags[index]->m_flagCount >> 3);
            break;
        }
    }
    m_playerFlags.push_back(flags);
}

void CPlatformNetworkManagerNintendo::SystemFlagSet(INetworkPlayer* player, int flag)
{
    if (flag < 0 || flag >= (int)m_maxSystemFlags || player == 0)
        return;
    for (unsigned int index = 0; index < m_playerFlags.size(); ++index)
        if (player->IsSameSystem(m_playerFlags[index]->m_player))
            m_playerFlags[index]->m_flags[flag / 8] |= 0x80 >> (flag % 8);
}

bool CPlatformNetworkManagerNintendo::SystemFlagGet(INetworkPlayer* player, int flag)
{
    if (flag < 0 || flag >= (int)m_maxSystemFlags || player == 0)
        return false;
    for (unsigned int index = 0; index < m_playerFlags.size(); ++index)
        if (m_playerFlags[index]->m_player == player)
            return (m_playerFlags[index]->m_flags[flag / 8] & (0x80 >> (flag % 8))) != 0;
    return false;
}

void CPlatformNetworkManagerNintendo::SystemFlagClearAll(INetworkPlayer* player)
{
    if (player == 0)
        return;
    for (unsigned int index = 0; index < m_playerFlags.size(); ++index)
        if (player == m_playerFlags[index]->m_player)
            memset(m_playerFlags[index]->m_flags, 0, m_playerFlags[index]->m_flagCount >> 3);
}

std::wstring CPlatformNetworkManagerNintendo::GatherStats()
{
    return std::wstring(L"");
}
std::wstring CPlatformNetworkManagerNintendo::GatherRTTStats()
{
    return std::wstring(L"");
}
void CPlatformNetworkManagerNintendo::SetSessionTexturePackParentId(int id)
{
    m_sessionData.m_texturePackParentId = id;
}
void CPlatformNetworkManagerNintendo::SetSessionSubTexturePackId(int id)
{
    m_sessionData.m_subTexturePackId = (unsigned char)id;
}
void CPlatformNetworkManagerNintendo::SetSessionGameModeId(int id)
{
    m_sessionData.m_gameModeId = id;
}
void CPlatformNetworkManagerNintendo::Notify(int, unsigned long)
{
}

#ifdef MINECRAFT_PC
static const size_t kPcDisplayNameSize = 60;

static bool IsDedicatedSession(const FriendSessionInfo* info)
{
    return info->m_gameSessionData.m_pcDedicated != 0;
}
#endif

std::vector<FriendSessionInfo*>* CPlatformNetworkManagerNintendo::GetSessionList(int, int, bool)
{
    std::vector<FriendSessionInfo*>* sessions = new std::vector<FriendSessionInfo*>();
    for (int index = 0; index < m_friendSearchCount; ++index)
    {
        NQRNetworkManager::SessionSearchResult& result = m_friendSearchResults[index];
        if (result.m_pExtData == 0)
            continue;
        FriendSessionInfo* info = new FriendSessionInfo;
#ifdef MINECRAFT_PC
        info->m_displayName = new wchar_t[kPcDisplayNameSize];
        memset(info->m_displayName, 0, kPcDisplayNameSize * sizeof(wchar_t));
        {
            wchar_t ownerName[22] = L"";
            mbstowcs(ownerName, result.m_szOwnerName, 17);
            const bool dedicated = static_cast<GameSessionData*>(result.m_pExtData)->m_pcDedicated != 0;
            std::wstring label = std::wstring(ownerName) + L" "
                                 + L10N::GetString(dedicated ? static_cast<int>(ePcString_ServerLabel) : 0x566a7b93);
            std::wstring content = PcNetworkSession::ContentLabel(result.m_uiSessionId);
            if (!content.empty())
                content += L" ";
            std::wstring title = PcNetworkSession::WorldTitle(result.m_uiSessionId);
            if (!title.empty() && content.size() + label.size() + 8 < kPcDisplayNameSize)
            {
                const size_t room = kPcDisplayNameSize - 1 - content.size() - label.size() - 5;
                if (title.size() > room)
                    title = title.substr(0, room - 3) + L"...";
                label = L"\"" + title + L"\" - " + label;
            }
            label = content + label;
            label = label.substr(0, kPcDisplayNameSize - 1);
            memcpy(info->m_displayName, label.c_str(), label.size() * sizeof(wchar_t));
        }
#else
        info->m_displayName = new wchar_t[32];
        memset(info->m_displayName, 0, 64);
        if (result.m_uiOwnerPrincipalId == result.m_uiOwnerLocalFriendIndex)
        {
            wchar_t ownerName[22];
            mbstowcs(ownerName, result.m_szOwnerName, 17);
            swprintf(info->m_displayName, 32, L"%ls %ls", ownerName, L10N::GetString(0x566a7b93));
        }
        else
            mbstowcs(info->m_displayName, result.m_szOwnerName, 17);
#endif
        info->m_displayNameLength = (unsigned char)wcslen(info->m_displayName);
        info->m_sessionId = result.m_uiSessionId;
        info->m_searchResult.m_sessionId = result.m_uiSessionId;
        info->m_searchResult.m_ownerPrincipalId = result.m_uiOwnerPrincipalId;
        info->m_searchResult.m_ownerLocalFriendIndex = result.m_uiOwnerLocalFriendIndex;
        memcpy(info->m_searchResult.m_ownerName, result.m_szOwnerName, sizeof(info->m_searchResult.m_ownerName));
        info->m_searchResult.m_externalData = result.m_pExtData;
        memcpy(&info->m_gameSessionData, result.m_pExtData, sizeof(GameSessionData));
        info->m_sessionFlag = false;
        info->m_statusFlag = false;
#ifdef MINECRAFT_PC
        if (info->m_gameSessionData.m_bGameStarted
            && (info->m_gameSessionData.m_bPublicGame || PcNetworkSession::IsLobbySession(info->m_sessionId))
            && info->m_gameSessionData.m_version == 0x881)
            sessions->push_back(info);
#else
        if (info->m_gameSessionData.m_bGameStarted && info->m_gameSessionData.m_bPublicGame
            && info->m_gameSessionData.m_version == 0x881)
            sessions->push_back(info);
#endif
        else
        {
            delete info->m_displayName;
            delete info;
        }
    }
#ifdef MINECRAFT_PC
    std::stable_partition(sessions->begin(), sessions->end(), IsDedicatedSession);
#endif
    return sessions;
}

bool CPlatformNetworkManagerNintendo::GetGameSessionInfo(int userIndex, unsigned int sessionId,
                                                         FriendSessionInfo* sessionInfo)
{
    (void)userIndex;
    (void)sessionId;
    (void)sessionInfo;
    return false;
}

void CPlatformNetworkManagerNintendo::SetSessionsUpdatedCallback(void (*callback)(void*), void* callbackData)
{
    m_sessionsUpdatedCallback = callback;
    m_sessionsUpdatedCallbackData = callbackData;
}

void CPlatformNetworkManagerNintendo::GetFullFriendSessionInfo(FriendSessionInfo* info, void (*callback)(bool, void*),
                                                               void* callbackData)
{
    m_networkManager->GetExtDataForRoom(info->m_sessionId, &info->m_gameSessionData, callback, callbackData);
}

void CPlatformNetworkManagerNintendo::ForceFriendsSessionRefresh()
{
    m_lastFriendSearchTime = 0;
    m_friendSearchCount = 0;
    delete m_friendSearchResults;
    m_friendSearchResults = 0;
}

void CPlatformNetworkManagerNintendo::ClearFriendsSessions()
{
    m_friendSearchCount = 0;
    delete m_friendSearchResults;
    m_friendSearchResults = 0;
}

bool CPlatformNetworkManagerNintendo::AlreadyInSession(const nn::fp::FriendPresence*)
{
    return false;
}
bool CPlatformNetworkManagerNintendo::usingAdhocMode()
{
    return false;
}
bool CPlatformNetworkManagerNintendo::setAdhocMode(bool)
{
    return false;
}
bool CPlatformNetworkManagerNintendo::isAdhocConnected()
{
    return false;
}
bool CPlatformNetworkManagerNintendo::connectToAdhoc()
{
    return false;
}
bool CPlatformNetworkManagerNintendo::isAdhocModeChangeInProgress()
{
    return m_enteringAdhocMode || m_leavingAdhocMode;
}
void CPlatformNetworkManagerNintendo::AttemptAdhocSignIn(int (*)(void*, bool, int), void*, bool)
{
}
void CPlatformNetworkManagerNintendo::startAdhocMatching()
{
}
bool CPlatformNetworkManagerNintendo::GetAgeRestricted()
{
    return false;
}
bool CPlatformNetworkManagerNintendo::usingLanMode()
{
    return m_lanMode;
}

bool CPlatformNetworkManagerNintendo::setLanMode(bool mode)
{
#if defined(RECON_NATIVE)
    m_lanMode = mode;
    m_enteringLanMode = false;
    m_leavingLanMode = false;
#else
    if (m_lanMode != mode)
    {
        if (mode)
        {
            if (!m_enteringLanMode)
            {
                m_onlineNetworkManager->Pia_ShutdownForLan();
                m_enteringLanMode = true;
            }
            else if (!NQRNetworkManager_WiiU::s_piaShutdownPending)
            {
                gLanNetworkManager->SignalInitialiseAfterOnline();
                m_enteringLanMode = false;
                m_lanMode = true;
                m_networkManager = gLanNetworkManager;
            }
        }
        else
        {
            if (!m_leavingLanMode)
            {
                gLanNetworkManager->Pia_ShutdownForOnline();
                m_leavingLanMode = true;
            }
            else if (!NQRNetworkManager_Lan_WiiU::s_shutdownTickPending)
            {
                m_leavingLanMode = false;
                m_lanMode = false;
                m_networkManager = m_onlineNetworkManager;
            }
        }
    }
#endif
    return true;
}

bool CPlatformNetworkManagerNintendo::isLanConnected()
{
    return m_networkManager->isLanConnected();
}
bool CPlatformNetworkManagerNintendo::isLanModeChangeInProgress()
{
    return m_enteringLanMode || m_leavingLanMode;
}
bool CPlatformNetworkManagerNintendo::checkValidInviteData(const nn::fp::FriendPresence*)
{
    return true;
}
void CPlatformNetworkManagerNintendo::SetExitAndJoinFromInviteInProgress(bool)
{
}
void CPlatformNetworkManagerNintendo::CancelDisconnectFromExitAndJoin()
{
    return;
}
int CPlatformNetworkManagerNintendo::GetOutstandingAckCount(INetworkPlayer*)
{
    return 0;
}

void CPlatformNetworkManagerNintendo::TryToConnectOnline(int (*callback)(void*, bool, int), void* callbackData,
                                                         bool showProgress)
{
    m_networkManager->TryToConnectOnline(callback, callbackData, showProgress);
}

void CPlatformNetworkManagerNintendo::AttemptPSNSignIn(int (*)(void*, bool, int), void*, bool, int)
{
}
void CPlatformNetworkManagerNintendo::RecvInviteGUI()
{
}
bool CPlatformNetworkManagerNintendo::UpdateInviteData(void* inviteData)
{
    (void)inviteData;
    const bool result = false;
    return result;
}

void CPlatformNetworkManagerNintendo::CheckFriendsCanPlay(int userIndex, int& friendCount,
                                                          boost::shared_ptr<ClientboundPreLoginPacket> packet,
                                                          int titleVersion, int& canPlayCount, int& cannotPlayCount,
                                                          int& blockedCount, int& unknownCount)
{
    CGameNetworkManager& manager = CGameNetworkManager::sInstance;
    bool isPrimaryPad = userIndex == 0;
    if (!manager.IsLocalGame())
        friendCount = !manager.IsUGCRestrictedMode(isPrimaryPad ? 0xff : userIndex, packet->mMiniGameId);

#ifdef MINECRAFT_PC
    return;
#endif
    if (friendCount != 0 && titleVersion == 0 && !manager.IsHost() && gApp.GetGameHostOption(kGameHostOption3) == 0
        && isPrimaryPad)
    {
        unsigned int friendIds[100];
        unsigned int count = 0;
        char accountIds[100][17];
        if (nn::fp::GetFriendList(friendIds, &count, 0, 100).IsSuccess())
            nn::fp::GetFriendAccountId(&accountIds[0][0], friendIds, count);

        char accountId[32];
        wcstombs(accountId, packet->mName.c_str(), sizeof(accountId));
        for (unsigned int i = 0; i < count; ++i)
        {
            if (strcmp(accountIds[i], accountId) == 0)
                return;
        }

        canPlayCount = 0;
        cannotPlayCount = 0;
    }
}
