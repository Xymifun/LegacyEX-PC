#include "platform/pc/PcNetworkSession.hpp"

#include "platform/pc/PcAccount.hpp"
#include "platform/pc/PcContent.hpp"
#include "platform/pc/PcLobbyClient.hpp"
#include "platform/pc/PcLobbyProtocol.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcVoice.hpp"
#include "platform/pc/PcSettings.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/network/INQRNetworkManagerListener.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"

#include <algorithm>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <strings.h>
#include <cwchar>

extern C_4JProfile* gProfile;
extern CInput* g_pInput;
extern CConsoleMinecraftApp gApp;

namespace
{
const unsigned int kMaxNetworkNameLength = 20;
const int TICK_MS = 16;
const unsigned int ALL_PLAYERS = 0xF;
const unsigned short kSessionDataVersion = 0x881;
const size_t MAX_TITLE_LENGTH = 48;
std::wstring pendingTitle;
const unsigned long long SEARCH_TIME_NS = 1500ULL * 1000000ULL;
const int ANY_PAD = 0xFF;
const int GAME_FULL_TITLE = 0x1afa54b5;
const unsigned int REFUSAL_DELAY_FRAMES = 30;
volatile int refusalTitle = -1;
volatile int refusalMessage = -1;
unsigned int refusalFrames = 0;
const int GAME_FULL_TEXT = (int)0xf18fccd0;

const unsigned char CHANNEL_VOICE = 251;
const unsigned int MAX_VOICE_BYTES = 400;
const unsigned int VOICE_HEADER_SIZE = 3;
const unsigned char CHANNEL_VOICE_PREFS = 250;
const unsigned long long VOICE_PREFS_INTERVAL_NS = 1000000000ULL;
unsigned long long nextVoicePrefsTime = 0;

struct VoicePreferences
{
    VoicePreferences() : proximity(false), range(kPcProximityRangeDefault) {}
    bool proximity;
    int range;
};

std::map<unsigned long long, VoicePreferences> voicePrefs;

bool ListenerProximity(unsigned long long station, bool local, int* range)
{
    VoicePreferences prefs;
    if (local)
    {
        prefs.proximity = PcSettingsProximityChat();
        prefs.range = PcSettingsProximityRange();
    }
    else
    {
        std::map<unsigned long long, VoicePreferences>::iterator it = voicePrefs.find(station);
        if (it != voicePrefs.end())
            prefs = it->second;
    }
    const int mode = PcSettingsHostProximity();
    *range = PcSettingsHostProximityRange() != 0 ? PcSettingsHostProximityRange() : prefs.range;
    if (mode == kPcHostProximityForce)
        return true;
    if (mode == kPcHostProximityDisable)
        return false;
    return prefs.proximity;
}
volatile int voicePeerCount = 0;
const unsigned char CHANNEL_JOIN_FAILED = 252;
const unsigned char CHANNEL_LEAVE = 253;
const unsigned char CHANNEL_JOIN = 254;
const unsigned char CHANNEL_ROOM_SYNC = 255;
const unsigned char LAST_PLAYER_CHANNEL = 3;

const NQRNetworkManager::eNQRNetworkManagerState externalStates[] = {
    NQRNetworkManager::eNQRNetworkManagerState_Uninitialised, NQRNetworkManager::eNQRNetworkManagerState_Initialising,
    NQRNetworkManager::eNQRNetworkManagerState_Idle,          NQRNetworkManager::eNQRNetworkManagerState_Idle,
    NQRNetworkManager::eNQRNetworkManagerState_Hosting,       NQRNetworkManager::eNQRNetworkManagerState_Hosting,
    NQRNetworkManager::eNQRNetworkManagerState_Hosting,       NQRNetworkManager::eNQRNetworkManagerState_Hosting,
    NQRNetworkManager::eNQRNetworkManagerState_Hosting,       NQRNetworkManager::eNQRNetworkManagerState_Joining,
    NQRNetworkManager::eNQRNetworkManagerState_Joining,       NQRNetworkManager::eNQRNetworkManagerState_Joining,
    NQRNetworkManager::eNQRNetworkManagerState_Joining,       NQRNetworkManager::eNQRNetworkManagerState_Joining,
    NQRNetworkManager::eNQRNetworkManagerState_Joining,       NQRNetworkManager::eNQRNetworkManagerState_Joining,
    NQRNetworkManager::eNQRNetworkManagerState_Joining,       NQRNetworkManager::eNQRNetworkManagerState_Joining,
    NQRNetworkManager::eNQRNetworkManagerState_Ending,        NQRNetworkManager::eNQRNetworkManagerState_Starting,
    NQRNetworkManager::eNQRNetworkManagerState_Playing,       NQRNetworkManager::eNQRNetworkManagerState_Leaving,
    NQRNetworkManager::eNQRNetworkManagerState_Leaving,       NQRNetworkManager::eNQRNetworkManagerState_Ending};

unsigned long long MakeStationId()
{
    unsigned long long id = PcOsMonotonicNanoseconds() ^ ((unsigned long long)PcPrincipalId() << 32);
    return id != 0 ? id : 1;
}

int CountPlayers(unsigned int mask)
{
    int n = 0;
    for (int i = 0; i < 4; ++i)
    {
        if ((mask & (1u << i)) != 0)
            ++n;
    }
    return n;
}
}

const char* const kServerListPath = "servers.db";
const char* const kTextServerListPath = "servers.txt";

PcNetworkSession* PcNetworkSession::s_lanSession = 0;

PcNetworkSession::ClientJoinData::ClientJoinData() : playerMask(0)
{
    std::memset(onlineIds, 0, sizeof(onlineIds));
    std::memset(displayNames, 0, sizeof(displayNames));
}

static int gamePort = kPcLanGamePort;

PcNetworkSession::PcNetworkSession(INQRNetworkManagerListener* listener, bool lanEnabled)
    : m_listener(listener), m_state(eNQRNetworkManagerState_Uninitialised),
      m_internalState(eNQRNetworkManagerInternalState_Uninitialised), m_lanEnabled(lanEnabled), m_host(false),
      m_localGame(false), m_receiving(false), m_matchmaking(false), m_terminate(false),
      m_localStationId(MakeStationId()), m_hostStationId(0), m_joinedDedicated(false), m_nextSmallId(0), m_roomAttributes(0), m_roomAttributeCount(0), m_externalData(0),
      m_externalDataSize(0), m_roomPlayerMask(0), m_localPlayerCount(0), m_localPlayersJoined(0), m_tickEvent(0),
      m_tickThread(0), m_searching(false), m_searchStartTime(0), m_searchRevision(0), m_lobbyRevision(0), m_searchResultsChanged(false),
      m_extDataSessionId(0), m_extDataBuffer(0), m_extDataCallback(0), m_extDataCallbackData(0),
      m_extDataRequested(false)
{
    std::memset(m_roomSlotPlayers, 0, sizeof(m_roomSlotPlayers));
    std::memset(static_cast<void*>(m_roomPlayers), 0, sizeof(m_roomPlayers));
    InitializeCriticalSection(&m_playerMutex);
    InitializeCriticalSection(&m_stateChangeLock);
    InitializeCriticalSection(&m_sendQueueMutex);
    InitializeCriticalSection(&m_searchMutex);
    if (m_lanEnabled)
        s_lanSession = this;
}

PcNetworkSession::~PcNetworkSession()
{
    Terminate();
    ClearSearchResults();
}

void PcNetworkSession::Initialise()
{
    m_internalState = eNQRNetworkManagerInternalState_Idle;
    m_state = eNQRNetworkManagerState_Idle;
    if (m_lanEnabled && PcOsNetInitialise())
        gProfile->SetNetworkStatusWithoutSigninChange(true);
    if (m_tickThread == 0)
    {
        m_tickEvent = new C4JEventImpl(C4JEvent::Mode0);
        m_transport.SetWakeEvent(m_tickEvent);
        m_tickThread = new C4JThreadImpl(TickThreadProc, this, "NetworkSessionTick", 0);
        m_tickThread->Run();
    }
}

void PcNetworkSession::Terminate()
{
    m_transport.SetWakeEvent(0);
    if (m_tickThread != 0)
    {
        m_terminate = true;
        m_tickEvent->Set();
        m_tickThread->WaitForCompletion(-1);
        delete m_tickThread;
        m_tickThread = 0;
    }
    StopLan();
    delete m_tickEvent;
    m_tickEvent = 0;
    m_discovery.StopBrowsing();
}

int PcNetworkSession::TickThreadProc(void* arg)
{
    PcNetworkSession* self = static_cast<PcNetworkSession*>(arg);
    while (!self->m_terminate)
    {
        self->m_tickEvent->WaitForSignal(TICK_MS);
        if (!self->m_terminate)
            self->TickFromThread();
    }
    return 0;
}

void PcNetworkSession::Tick()
{
    if (m_tickEvent != 0)
        m_tickEvent->Set();
}

void PcNetworkSession::SetState(eNQRNetworkManagerInternalState state)
{
    if (state == eNQRNetworkManagerInternalState_Idle)
        m_matchmaking = false;
    eNQRNetworkManagerState prev = externalStates[m_internalState];
    eNQRNetworkManagerState cur = externalStates[state];
    m_internalState = state;
    if (cur != prev)
    {
        EnterCriticalSection(&m_stateChangeLock);
        m_stateChanges.push_back(StateChangeInfo(prev, cur, false));
        LeaveCriticalSection(&m_stateChangeLock);
    }
}

void PcNetworkSession::TickFromThread()
{
    if (m_lanEnabled)
    {
        TickSendQueue();
        PollTransportEvents();
        PollReceiveData();
        TickVoice();
        m_transport.ReapClosedConnections();
        if (m_host && !m_localGame && m_externalData != 0)
        {
            PublishSessionData();
        }
        TickSearch();
    }

    if (m_internalState == eNQRNetworkManagerInternalState_JoinSessionFailed)
        SetState(eNQRNetworkManagerInternalState_Ending);
    if (m_internalState == eNQRNetworkManagerInternalState_JoinRoomReady && m_localPlayerCount > 0
        && m_localPlayersJoined >= m_localPlayerCount)
    {
        SetState(eNQRNetworkManagerInternalState_StartingGame);
        SetState(eNQRNetworkManagerInternalState_Playing);
    }
    if (m_internalState == eNQRNetworkManagerInternalState_Ending)
        ResetToIdle();

    EnterCriticalSection(&m_stateChangeLock);
    while (!m_stateChanges.empty())
    {
        StateChangeInfo c = m_stateChanges.front();
        m_stateChanges.pop_front();
        LeaveCriticalSection(&m_stateChangeLock);
        if (m_listener != 0)
        {
            m_listener->HandleStateChange(c.eOldState, c.eNewState, c.bError);
            if (c.eNewState == eNQRNetworkManagerState_Idle)
                m_receiving = false;
        }
        m_state = c.eNewState;
        EnterCriticalSection(&m_stateChangeLock);
    }
    LeaveCriticalSection(&m_stateChangeLock);
}

NQRNetworkManager::eNQRNetworkManagerState PcNetworkSession::GetState()
{
    return m_state;
}

bool PcNetworkSession::IsHost()
{
    return m_host;
}

bool PcNetworkSession::IsReadyToPlayOrIdle()
{
    return m_internalState == eNQRNetworkManagerInternalState_HostRoomReady
           || m_internalState == eNQRNetworkManagerInternalState_Playing
           || m_internalState == eNQRNetworkManagerInternalState_Idle;
}

bool PcNetworkSession::IsInSession()
{
    return m_receiving;
}

bool PcNetworkSession::IsMatchmakingEnabled()
{
    return m_matchmaking;
}

unsigned int PcNetworkSession::AllocateSmallId(const PlayerSyncData* fill)
{
    const unsigned int ID_COUNT = 255;
    for (unsigned int a = 0; a < ID_COUNT; ++a)
    {
        const unsigned int id = (m_nextSmallId + a) % ID_COUNT;
        bool used = false;
        for (int i = 0; i < kMaxRoomPlayers && !used; ++i)
        {
            const PlayerSyncData& p = m_roomPlayers[i];
            used = &p != fill && p.m_uiStationId != 0 && p.m_uiSmallId == id;
        }
        if (!used)
        {
            m_nextSmallId = (id + 1) % ID_COUNT;
            return id;
        }
    }
    return m_nextSmallId++ % ID_COUNT;
}

void PcNetworkSession::MakeNetworkNameUnique(PlayerSyncData& slot)
{
    char base[sizeof(slot.m_szNetworkName)];
    std::memset(base, 0, sizeof(base));
    std::strncpy(base, slot.m_szNetworkName, kMaxNetworkNameLength);
    for (int n = 1; n < 100; ++n)
    {
        char name[sizeof(slot.m_szNetworkName)];
        std::memset(name, 0, sizeof(name));
        if (n == 1)
            std::strncpy(name, base, kMaxNetworkNameLength);
        else
        {
            char tail[8];
            std::snprintf(tail, sizeof(tail), " (%d)", n);
            const size_t maxLen = kMaxNetworkNameLength - std::strlen(tail);
            std::strncpy(name, base, maxLen);
            name[maxLen < std::strlen(base) ? maxLen : std::strlen(base)] = 0;
            std::strcat(name, tail);
        }
        bool taken = false;
        for (int i = 0; i < kMaxRoomPlayers && !taken; ++i)
        {
            const PlayerSyncData& p = m_roomPlayers[i];
            taken = &p != &slot && p.m_uiStationId != 0
                    && strncasecmp(p.m_szNetworkName, name, kMaxNetworkNameLength) == 0;
        }
        if (!taken)
        {
            if (n != 1)
            {
                printf("[pc] lan: %s is already in this game; showing the new player as %s\n", base, name);
                fflush(stdout);
            }
            std::strncpy(slot.m_szNetworkName, name, kMaxNetworkNameLength);
            slot.m_szNetworkName[kMaxNetworkNameLength] = 0;
            return;
        }
    }
}

void PcNetworkSession::FillLocalSlot(PlayerSyncData& slot, int user)
{
    slot.m_uiLocalPlayerIndex = user;
    slot.m_uiStationId = 0;
    slot.m_uiSmallId = AllocateSmallId(&slot);
    slot.m_uiStationId = m_localStationId;
    gProfile->GetXUID(user, &slot, false);
    std::wstring mii = gProfile->GetMiiName(user);
    std::wstring name = gProfile->GetDisplayName(user);
    std::memset(slot.m_szMiiName, 0, sizeof(slot.m_szMiiName));
    std::wcsncpy(slot.m_szMiiName, mii.c_str(), sizeof(slot.m_szMiiName) / sizeof(wchar_t) - 1);
    std::memset(slot.m_szNetworkName, 0, sizeof(slot.m_szNetworkName));
    std::wcstombs(slot.m_szNetworkName, name.c_str(), kMaxNetworkNameLength);
}

void PcNetworkSession::CreateAndJoinRoom(int primary, int mask, void* extData, int extDataSize,
                                         int* attrs, int attrCount, bool local, unsigned char)
{
    m_roomAttributes = attrCount > 0 ? attrs : 0;
    m_roomAttributeCount = attrCount > 0 ? attrCount : 0;
    m_externalData = extData;
    m_externalDataSize = extDataSize;
    m_worldTitle = pendingTitle;
    pendingTitle.clear();
    m_receiving = true;
    m_host = true;
    m_localGame = local || !m_lanEnabled;
    m_matchmaking = attrCount > 0;
    m_nextSmallId = 0;
    m_hostStationId = m_localStationId;

    EnterCriticalSection(&m_playerMutex);
    std::memset(static_cast<void*>(m_roomPlayers), 0, sizeof(m_roomPlayers));
    FillLocalSlot(m_roomPlayers[0], primary);
    m_roomPlayers[0].m_uiPlayerCountMinusOne = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (i != primary && (mask & (1 << i)) != 0)
        {
            FillLocalSlot(m_roomPlayers[m_roomPlayers[0].m_uiPlayerCountMinusOne + 1], i);
            m_roomPlayers[0].m_uiPlayerCountMinusOne++;
        }
    }
    LeaveCriticalSection(&m_playerMutex);

    if (!m_localGame)
        StartLanHosting();
    SetState(eNQRNetworkManagerInternalState_HostSessionCreated);
    SetState(eNQRNetworkManagerInternalState_HostRoomReady);
    MapRoomSlotPlayers();
    SyncRoomData();
}

void PcNetworkSession::SetGamePort(int port)
{
    gamePort = port < 0 || port > 65535 ? kPcLanGamePort : port;
}

void PcNetworkSession::StartLanHosting()
{
    m_discovery.StopBrowsing();
    unsigned short port = (unsigned short)gamePort;
    if (!m_transport.Host(m_localStationId, port))
    {
        printf("[pc] lan: cannot listen on TCP port %u (in use?); the world is reachable only through a lobby\n",
               port);
        fflush(stdout);
        port = 0;
        m_transport.Host(m_localStationId, 0);
    }
    unsigned int id = (unsigned int)(m_localStationId ^ (m_localStationId >> 32));
    if (id == 0)
        id = 1;
    if (m_externalData != 0)
    {
        PublishSessionData();
    }
    if (port == 0)
    {
        printf("[pc] lan: hosting without listening (session %08x); guests come through a lobby\n", id);
        fflush(stdout);
        return;
    }
    m_discovery.StartAdvertising(id, port);
    printf("[pc] lan: hosting on TCP port %u (session %08x)\n", port, id);
    fflush(stdout);
}

void PcNetworkSession::StopLan()
{
    m_discovery.StopAdvertising();
    m_transport.Close();
    ClearSendQueue();
}

NQRNetworkPlayer* PcNetworkSession::FindPlayer(unsigned long long station, unsigned int idx)
{
    for (std::vector<NQRNetworkPlayer*>::iterator it = m_players.begin(); it != m_players.end(); ++it)
    {
        if ((*it)->GetStationId() == station && (*it)->GetLocalIndex() == idx)
            return *it;
    }
    return 0;
}

void PcNetworkSession::HandlePlayerJoined(NQRNetworkPlayer* player)
{
    if (m_listener != 0)
        m_listener->HandlePlayerJoined(player);
    if (!m_host && player->IsLocal())
        ++m_localPlayersJoined;
}

void PcNetworkSession::NetworkPlayerSmallIdAllocated(NQRNetworkPlayer* player, unsigned char smallId)
{
    bool wasReady = player->IsReady();
    player->SmallIdAllocated(smallId);
    if (!wasReady && player->IsReady())
        HandlePlayerJoined(player);
}

NQRNetworkPlayer* PcNetworkSession::CreateRoomPlayer(int i, NQRNetworkPlayer::eNQRNetworkPlayerType type)
{
    PlayerSyncData& slot = m_roomPlayers[i];
    PlayerUID uid = slot;
    if (type == NQRNetworkPlayer::eNQRNetworkPlayerType_Local
        || (m_host && type == NQRNetworkPlayer::eNQRNetworkPlayerType_Host))
        gProfile->GetXUID(slot.m_uiLocalPlayerIndex, &uid, false);
    NQRNetworkPlayer* p = new NQRNetworkPlayer(this, type, m_host, slot.m_uiStationId, slot.m_uiLocalPlayerIndex,
                                                    &uid, slot.m_szNetworkName, slot.m_szMiiName);
    p->ConnectionComplete();
    p->SmallIdAllocated((unsigned char)slot.m_uiSmallId);
    m_players.push_back(p);
    m_roomSlotPlayers[i] = p;
    HandlePlayerJoined(p);
    return p;
}

void PcNetworkSession::RemovePlayer(NQRNetworkPlayer* player)
{
    if (player != 0)
        PcVoiceForgetSpeaker(player->GetSmallId());
    for (std::vector<NQRNetworkPlayer*>::iterator it = m_players.begin(); it != m_players.end(); ++it)
    {
        if (*it == player)
        {
            m_players.erase(it);
            break;
        }
    }
}

void PcNetworkSession::MapRoomSlotPlayers()
{
    EnterCriticalSection(&m_playerMutex);
    std::vector<NQRNetworkPlayer*> unmapped(m_players);
    std::memset(m_roomSlotPlayers, 0, sizeof(m_roomSlotPlayers));
    int n = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
    if (m_roomPlayers[0].m_uiStationId == 0)
        n = 0;
    for (int i = 0; i < n; ++i)
    {
        PlayerSyncData& slot = m_roomPlayers[i];
        NQRNetworkPlayer::eNQRNetworkPlayerType type = NQRNetworkPlayer::eNQRNetworkPlayerType_Remote;
        if (i == 0)
            type = NQRNetworkPlayer::eNQRNetworkPlayerType_Host;
        else if (slot.m_uiStationId == m_localStationId)
            type = NQRNetworkPlayer::eNQRNetworkPlayerType_Local;
        if (!m_host && type == NQRNetworkPlayer::eNQRNetworkPlayerType_Local
            && !gProfile->IsSignedIn(slot.m_uiLocalPlayerIndex))
            continue;

        NQRNetworkPlayer* p = FindPlayer(slot.m_uiStationId, slot.m_uiLocalPlayerIndex);
        if (p == 0)
        {
            if (m_host && type == NQRNetworkPlayer::eNQRNetworkPlayerType_Remote)
                continue;
            p = CreateRoomPlayer(i, type);
        }
        else
        {
            m_roomSlotPlayers[i] = p;
            if (m_host && type == NQRNetworkPlayer::eNQRNetworkPlayerType_Remote)
                NetworkPlayerSmallIdAllocated(p, (unsigned char)slot.m_uiSmallId);
        }

        for (std::vector<NQRNetworkPlayer*>::iterator it = unmapped.begin(); it != unmapped.end(); ++it)
        {
            if (*it == p)
            {
                unmapped.erase(it);
                break;
            }
        }
        m_roomSlotPlayers[i] = p;
        if (m_host)
            static_cast<PlayerUID&>(slot) = p->GetUID();
        else if (type != NQRNetworkPlayer::eNQRNetworkPlayerType_Local)
            p->SetUID(slot);
    }

    for (std::vector<NQRNetworkPlayer*>::iterator it = unmapped.begin(); it != unmapped.end(); ++it)
    {
        if (m_host && !(*it)->IsReady())
            continue;
        if (m_listener != 0)
            m_listener->HandlePlayerLeaving(*it);
        RemovePlayer(*it);
        delete *it;
    }
    if (m_host && m_listener != 0)
        m_listener->HandleResyncPlayerRequest(m_roomSlotPlayers);
    LeaveCriticalSection(&m_playerMutex);
}

void PcNetworkSession::ResetToIdle()
{
    StopLan();
    EnterCriticalSection(&m_playerMutex);
    for (std::vector<NQRNetworkPlayer*>::iterator it = m_players.begin(); it != m_players.end(); ++it)
        delete *it;
    m_players.clear();
    std::memset(m_roomSlotPlayers, 0, sizeof(m_roomSlotPlayers));
    std::memset(static_cast<void*>(m_roomPlayers), 0, sizeof(m_roomPlayers));
    LeaveCriticalSection(&m_playerMutex);
    m_host = false;
    m_localGame = false;
    m_hostStationId = 0;
    m_nextSmallId = 0;
    m_roomPlayerMask = 0;
    m_localPlayerCount = 0;
    m_localPlayersJoined = 0;
    SetState(eNQRNetworkManagerInternalState_Idle);
}

void PcNetworkSession::QueueSend(unsigned long long station, unsigned char channel, const void* data,
                                 unsigned int size)
{
    SendQueueItem item;
    item.stationId = station;
    item.data.resize(size + 1);
    item.data[0] = channel;
    if (data != 0 && size != 0)
        std::memcpy(&item.data[1], data, size);
    EnterCriticalSection(&m_sendQueueMutex);
    m_sendQueue.push_back(SendQueueItem());
    m_sendQueue.back().stationId = item.stationId;
    m_sendQueue.back().data.swap(item.data);
    LeaveCriticalSection(&m_sendQueueMutex);
    if (m_tickEvent != 0)
        m_tickEvent->Set();
}

void PcNetworkSession::ClearSendQueue()
{
    EnterCriticalSection(&m_sendQueueMutex);
    m_sendQueue.clear();
    LeaveCriticalSection(&m_sendQueueMutex);
}

void PcNetworkSession::TickSendQueue()
{
    std::deque<SendQueueItem> pending;
    EnterCriticalSection(&m_sendQueueMutex);
    pending.swap(m_sendQueue);
    LeaveCriticalSection(&m_sendQueueMutex);
    for (std::deque<SendQueueItem>::iterator it = pending.begin(); it != pending.end(); ++it)
        m_transport.Send(it->stationId, &it->data[0], (unsigned int)it->data.size());
}

void PcNetworkSession::SyncRoomData()
{
    if (m_localGame || !m_host)
        return;
    EnterCriticalSection(&m_playerMutex);
    for (int i = 0; i < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++i)
    {
        if (m_roomSlotPlayers[i] != 0)
            static_cast<PlayerUID&>(m_roomPlayers[i]) = m_roomSlotPlayers[i]->GetUID();
    }
    QueueSend(0, CHANNEL_ROOM_SYNC, m_roomPlayers, sizeof(m_roomPlayers));
    LeaveCriticalSection(&m_playerMutex);
}

void PcNetworkSession::SendJoinData(unsigned int mask)
{
    ClientJoinData data;
    data.playerMask = mask;
    for (int i = 0; i < 4; ++i)
    {
        if ((mask & (1u << i)) != 0)
        {
            gProfile->GetXUID(i, &data.playerUIDs[i], false);
            std::wstring mii = gProfile->GetMiiName(i);
            std::wcsncpy(data.displayNames[i], mii.c_str(), 10);
            std::wstring name = gProfile->GetDisplayName(i);
            std::wcstombs(data.onlineIds[i], name.c_str(), kMaxNetworkNameLength);
        }
    }
    QueueSend(m_hostStationId, CHANNEL_JOIN, &data, sizeof(data));
}

void PcNetworkSession::SendLeaveData(unsigned int mask)
{
    QueueSend(m_hostStationId, CHANNEL_LEAVE, &mask, sizeof(mask));
}

void PcNetworkSession::SendJoinFailedData(unsigned long long station, unsigned int mask,
                                          EJoinFailedReason reason)
{
    unsigned int msg[2];
    msg[0] = mask;
    msg[1] = (unsigned int)reason;
    QueueSend(station, CHANNEL_JOIN_FAILED, msg, sizeof(msg));
}

bool PcNetworkSession::AddRemotePlayersAndSync(unsigned long long stationId, const ClientJoinData& joinData,
                                               EJoinFailedReason* reason)
{
    PcVerifiedIdentities verified;
    if (m_transport.GetVerifiedIdentities(stationId, &verified))
    {
        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            const PlayerUID& uid = joinData.playerUIDs[userIndex];
            if ((joinData.playerMask & (1u << userIndex)) == 0 || (uid.flags & PlayerUID::FlagUuid) == 0)
                continue;
            if (!verified.Contains(uid.uuid.data))
            {
                printf("[pc] lan: station %016llx joined through the lobby as another player's identity; refused\n",
                       stationId);
                fflush(stdout);
                *reason = eJoinFailedReason_MiiInUse;
                return false;
            }
        }
    }
    EnterCriticalSection(&m_playerMutex);
    int playerCount = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
    for (int userIndex = 0; userIndex < 4; ++userIndex)
    {
        if ((joinData.playerMask & (1u << userIndex)) == 0)
            continue;
        for (int index = 0; index < playerCount; ++index)
        {
            if (m_roomPlayers[index] == joinData.playerUIDs[userIndex])
            {
                *reason = eJoinFailedReason_MiiInUse;
                LeaveCriticalSection(&m_playerMutex);
                return false;
            }
        }
    }
    int joiningCount = CountPlayers(joinData.playerMask);
    if (joiningCount == 0 || playerCount + joiningCount > kMaxRoomPlayers
        || playerCount + joiningCount > static_cast<int>(CGameNetworkManager::sInstance.GetMaxPlayers()))
    {
        *reason = eJoinFailedReason_GameFull;
        LeaveCriticalSection(&m_playerMutex);
        return false;
    }

    int insertIndex = playerCount;
    for (int index = 0; index < playerCount; ++index)
    {
        if (m_roomPlayers[index].m_uiStationId != stationId)
            continue;
        insertIndex = index;
        while (insertIndex < playerCount && m_roomPlayers[insertIndex].m_uiStationId == stationId)
            ++insertIndex;
        break;
    }
    for (int userIndex = 0; userIndex < 4; ++userIndex)
    {
        if ((joinData.playerMask & (1u << userIndex)) == 0 || FindPlayer(stationId, userIndex) != 0)
            continue;
        int count = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
        for (int index = count; index > insertIndex; --index)
            m_roomPlayers[index] = m_roomPlayers[index - 1];
        PlayerSyncData& slot = m_roomPlayers[insertIndex];
        std::memset(static_cast<void*>(&slot), 0, sizeof(slot));
        static_cast<PlayerUID&>(slot) = joinData.playerUIDs[userIndex];
        slot.m_uiSmallId = AllocateSmallId(&slot);
        slot.m_uiStationId = stationId;
        slot.m_uiLocalPlayerIndex = userIndex;
        std::wcsncpy(slot.m_szMiiName, joinData.displayNames[userIndex], 10);
        std::strncpy(slot.m_szNetworkName, joinData.onlineIds[userIndex], kMaxNetworkNameLength);
        slot.m_uiStationId = stationId;
        MakeNetworkNameUnique(slot);
        m_roomPlayers[0].m_uiPlayerCountMinusOne = count;
        PlayerUID uid = joinData.playerUIDs[userIndex];
        NQRNetworkPlayer* player = new NQRNetworkPlayer(this, NQRNetworkPlayer::eNQRNetworkPlayerType_Remote, true,
                                                        stationId, userIndex, &uid, slot.m_szNetworkName,
                                                        slot.m_szMiiName);
        player->ConnectionComplete();
        m_players.push_back(player);
        ++insertIndex;
    }
    LeaveCriticalSection(&m_playerMutex);
    MapRoomSlotPlayers();
    SyncRoomData();
    return true;
}

void PcNetworkSession::RemoveRemotePlayersAndSync(unsigned long long station, unsigned int mask)
{
    EnterCriticalSection(&m_playerMutex);
    for (int i = 1; i < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;)
    {
        if (m_roomPlayers[i].m_uiStationId == station
            && ((1u << m_roomPlayers[i].m_uiLocalPlayerIndex) & mask) != 0)
        {
            int n = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
            for (int j = i; j < n - 1; ++j)
                m_roomPlayers[j] = m_roomPlayers[j + 1];
            std::memset(static_cast<void*>(&m_roomPlayers[n - 1]), 0, sizeof(PlayerSyncData));
            m_roomPlayers[0].m_uiPlayerCountMinusOne = n - 2;
        }
        else
            ++i;
    }
    for (std::vector<NQRNetworkPlayer*>::iterator it = m_players.begin(); it != m_players.end();)
    {
        if ((*it)->GetStationId() == station && ((1u << (*it)->GetLocalIndex()) & mask) != 0
            && !(*it)->IsReady())
        {
            delete *it;
            it = m_players.erase(it);
        }
        else
            ++it;
    }
    LeaveCriticalSection(&m_playerMutex);
    MapRoomSlotPlayers();
    SyncRoomData();
}

void PcNetworkSession::RemoveLocalClientPlayers(unsigned int mask)
{
    EnterCriticalSection(&m_playerMutex);
    for (std::vector<NQRNetworkPlayer*>::iterator it = m_players.begin(); it != m_players.end();)
    {
        NQRNetworkPlayer* p = *it;
        if (p->GetStationId() == m_localStationId && ((1u << p->GetLocalIndex()) & mask) != 0)
        {
            if (m_listener != 0)
                m_listener->HandlePlayerLeaving(p);
            for (int i = 0; i < kMaxRoomPlayers; ++i)
            {
                if (m_roomSlotPlayers[i] == p)
                    m_roomSlotPlayers[i] = 0;
            }
            it = m_players.erase(it);
            delete p;
        }
        else
            ++it;
    }
    LeaveCriticalSection(&m_playerMutex);
}

void PcNetworkSession::PollTransportEvents()
{
    PcLanTransport::Event ev;
    while (m_transport.PopEvent(&ev))
    {
        switch (ev.type)
        {
        case PcLanTransport::eEvent_Connected:
            if (!m_host && m_internalState == eNQRNetworkManagerInternalState_JoinJoiningSession)
            {
                if (!HostContentMatches(ev.stationId))
                    break;
                m_hostStationId = ev.stationId;
                SetState(eNQRNetworkManagerInternalState_JoinRoomReady);
                SendJoinData(m_roomPlayerMask);
                printf("[pc] lan: connected to host\n");
                fflush(stdout);
            }
            else if (m_host)
            {
                printf("[pc] lan: station %016llx connected\n", ev.stationId);
                fflush(stdout);
            }
            break;
        case PcLanTransport::eEvent_ConnectFailed:
            if (!m_host && m_internalState == eNQRNetworkManagerInternalState_JoinJoiningSession)
            {
                printf("[pc] lan: could not connect to the host\n");
                fflush(stdout);
                gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
                SetState(eNQRNetworkManagerInternalState_JoinSessionFailed);
            }
            break;
        case PcLanTransport::eEvent_Disconnected:
            if (m_host)
            {
                printf("[pc] lan: station %016llx disconnected\n", ev.stationId);
                fflush(stdout);
                RemoveRemotePlayersAndSync(ev.stationId, ALL_PLAYERS);
            }
            else if (ev.stationId == m_hostStationId
                     && m_internalState != eNQRNetworkManagerInternalState_LeavingRoom
                     && m_internalState != eNQRNetworkManagerInternalState_Idle)
            {
                printf("[pc] lan: lost the connection to the host\n");
                fflush(stdout);
                Minecraft* mc = Minecraft::GetInstance();
                int pad = gProfile->GetPrimaryPad();
                mc->mConnectionFailed[pad] = true;
                mc->mDisconnectReasons[pad] = DisconnectPacket::eDisconnectReason_NetworkError;
                gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
                SetState(eNQRNetworkManagerInternalState_LeavingRoom);
                SetState(eNQRNetworkManagerInternalState_Ending);
            }
            break;
        }
    }
}

void PcNetworkSession::HandleGameData(unsigned long long station, unsigned char channel,
                                      const unsigned char* packet, unsigned int size)
{
    if (size < 2)
        return;
    EnterCriticalSection(&m_playerMutex);
    NQRNetworkPlayer* host = m_roomSlotPlayers[0];
    NQRNetworkPlayer* p =
        m_host ? FindPlayer(station, channel) : FindPlayer(m_localStationId, channel);
    if (host != 0 && p != 0)
    {
        NQRNetworkPlayer::AckFlags flags = static_cast<NQRNetworkPlayer::AckFlags>(packet[1]);
        unsigned char* payload = const_cast<unsigned char*>(packet + 2);
        if (size > 2)
        {
            if (m_host)
            {
                host->ReadAckRequest(flags, p);
                m_listener->HandleDataReceived(p, host, payload, size - 2);
            }
            else
            {
                p->ReadAckRequest(flags, host);
                m_listener->HandleDataReceived(host, p, payload, size - 2);
            }
        }
        else
            p->ReadAckReturn(flags);
    }
    LeaveCriticalSection(&m_playerMutex);
}

bool PcNetworkSession::HostContentMatches(unsigned long long hostId)
{
    std::vector<unsigned char> hello;
    m_transport.GetPeerHelloData(hostId, &hello);
    PcContentIdentity info;
    PcContentParse(hello, sizeof(GameSessionData), &info);
    if (PcContentMatches(info))
        return true;
    std::vector<std::string> diffs = PcContentDifferences(info, true);
    printf("[pc] content: cannot join: the host has other gameplay mods (game version %s, yours %s)\n",
           info.present ? PcContentGameVersion(info).c_str() : "unknown",
           PcContentGameVersion(PcContentLocal()).c_str());
    for (size_t i = 0; i < diffs.size(); ++i)
        printf("[pc] content:   %s\n", diffs[i].c_str());
    fflush(stdout);
    PcContentSetMismatch(info);
    SetRefusalMessage(IDS_CANTJOIN_TITLE, ePcString_MismatchedContent);
    Minecraft* mc = Minecraft::GetInstance();
    mc->mDisconnectReasons[gProfile->GetPrimaryPad()] = DisconnectPacket::eDisconnectReason_PcMismatchedContent;
    gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_PcMismatchedContent);
    m_transport.Disconnect(hostId);
    SetState(eNQRNetworkManagerInternalState_JoinSessionFailed);
    return false;
}

bool PcNetworkSession::GuestContentMatches(unsigned long long station, unsigned int mask)
{
    std::vector<unsigned char> hello;
    m_transport.GetPeerHelloData(station, &hello);
    PcContentIdentity guest;
    PcContentParse(hello, 0, &guest);
    if (PcContentMatches(guest))
        return true;
    std::vector<std::string> diffs = PcContentDifferences(guest, false);
    printf("[pc] content: refused station %016llx: other gameplay mods (game version %s)\n", station,
           guest.present ? PcContentGameVersion(guest).c_str() : "unknown");
    for (size_t i = 0; i < diffs.size(); ++i)
        printf("[pc] content:   %s\n", diffs[i].c_str());
    if (guest.present)
        SendJoinFailedData(station, mask, eJoinFailedReason_MismatchedContent);
    else
        m_transport.Disconnect(station);
    return false;
}

void PcNetworkSession::SetRefusalMessage(int title, int msg)
{
    refusalFrames = 0;
    refusalTitle = title;
    refusalMessage = msg;
}

void PcNetworkSession::TickRefusalMessage()
{
    if (refusalMessage == -1)
        return;
    if (gApp.GetGameStarted() || gConsoleUIController.FindScene(EUIScene_MainMenu) == 0)
    {
        refusalFrames = 0;
        return;
    }
    if (++refusalFrames < REFUSAL_DELAY_FRAMES)
        return;
    int param = IDS_CONFIRM_OK;
    gConsoleUIController.RequestErrorMessage(refusalTitle, refusalMessage, &param, 1, 0xff, 0, 0, 0,
                                             true);
    refusalTitle = -1;
    refusalMessage = -1;
}

void PcNetworkSession::HandleJoinFailed(unsigned int mask, EJoinFailedReason reason)
{
    if (reason == eJoinFailedReason_MismatchedContent)
    {
        printf("[pc] lan: the host refused the join (mismatched content)\n");
        fflush(stdout);
        SetRefusalMessage(IDS_CANTJOIN_TITLE, ePcString_MismatchedContent);
        gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_PcMismatchedContent);
        LeaveRoom(true);
        return;
    }
    if ((mask & 1) == 0)
    {
        for (int i = 1; i < 4; ++i)
        {
            if ((mask & (1u << i)) != 0)
            {
                gApp.SetAction(i, eXuiAction_RemoveLocalPlayer, 0);
                RemoveLocalPlayerByUserIndex(i);
            }
        }
        int param = IDS_CONFIRM_OK;
        if (reason == eJoinFailedReason_GameFull)
            gConsoleUIController.RequestErrorMessage(GAME_FULL_TITLE, GAME_FULL_TEXT, &param, 1, ANY_PAD,
                                                     0, 0, 0, true);
        else
            gConsoleUIController.RequestErrorMessage(STRING_MII_IN_USE_TITLE, STRING_MII_ALREADY_IN_USE,
                                                     &param, 1, ANY_PAD, 0, 0, 0, true);
        return;
    }
    printf("[pc] lan: the host refused the join (%s)\n", reason == eJoinFailedReason_GameFull ? "full" : "player in use");
    fflush(stdout);
    if (reason == eJoinFailedReason_GameFull)
        SetRefusalMessage(GAME_FULL_TITLE, GAME_FULL_TEXT);
    else
        SetRefusalMessage(STRING_MII_IN_USE_TITLE, STRING_MII_ALREADY_IN_USE);
    gApp.SetDisconnectReason(reason == eJoinFailedReason_GameFull ? DisconnectPacket::eDisconnectReason_GameFull
                                                                  : DisconnectPacket::eDisconnectReason_MiiInUse);
    LeaveRoom(true);
}

void PcNetworkSession::PollReceiveData()
{
    unsigned long long station;
    std::vector<unsigned char> packet;
    while (m_transport.Receive(&station, &packet))
    {
        if (packet.empty())
            continue;
        const unsigned int len = (unsigned int)packet.size();
        const unsigned char ch = packet[0];
        if (ch <= LAST_PLAYER_CHANNEL)
        {
            HandleGameData(station, ch, &packet[0], len);
            continue;
        }
        switch (ch)
        {
        case CHANNEL_ROOM_SYNC:
            if (!m_host && len == sizeof(m_roomPlayers) + 1)
            {
                EnterCriticalSection(&m_playerMutex);
                std::memcpy(static_cast<void*>(m_roomPlayers), &packet[1], sizeof(m_roomPlayers));
                LeaveCriticalSection(&m_playerMutex);
                MapRoomSlotPlayers();
            }
            break;
        case CHANNEL_JOIN:
            if (m_host && len == sizeof(ClientJoinData) + 1)
            {
                ClientJoinData join;
                std::memcpy(&join, &packet[1], sizeof(join));
                EJoinFailedReason reason = eJoinFailedReason_GameFull;
                if (GuestContentMatches(station, join.playerMask))
                {
                    if (AddRemotePlayersAndSync(station, join, &reason))
                        printf("[pc] lan: station %016llx joined with %d player(s)\n", station,
                               CountPlayers(join.playerMask));
                    else
                    {
                        printf("[pc] lan: refused station %016llx\n", station);
                        SendJoinFailedData(station, join.playerMask, reason);
                    }
                }
                fflush(stdout);
            }
            break;
        case CHANNEL_LEAVE:
            if (len == sizeof(unsigned int) + 1)
            {
                unsigned int mask;
                std::memcpy(&mask, &packet[1], sizeof(mask));
                if (m_host)
                    RemoveRemotePlayersAndSync(station, mask);
            }
            break;
        case CHANNEL_VOICE:
            HandleVoice(station, &packet[1], len - 1);
            break;
        case CHANNEL_VOICE_PREFS:
            if (len == 3 && m_host)
            {
                VoicePreferences& prefs = voicePrefs[station];
                prefs.proximity = packet[1] != 0;
                prefs.range = packet[2] < kPcProximityRangeMinimum ? kPcProximityRangeMinimum
                                    : packet[2] > kPcProximityRangeMaximum ? kPcProximityRangeMaximum
                                                                         : packet[2];
            }
            else if (len == 3 && !m_host && station == m_hostStationId)
            {
                PcVoiceSetHostOverride(packet[1] < kPcHostProximityCount ? packet[1] : 0, packet[2]);
            }
            break;
        case CHANNEL_JOIN_FAILED:
            if (!m_host && len == 2 * sizeof(unsigned int) + 1)
            {
                unsigned int failed[2];
                std::memcpy(failed, &packet[1], sizeof(failed));
                HandleJoinFailed(failed[0], static_cast<EJoinFailedReason>(failed[1]));
            }
            break;
        default:
            break;
        }
    }
}

void PcNetworkSession::PublishSessionData()
{
    if (m_externalData == 0 || m_externalDataSize <= 0)
        return;
    const unsigned char* ext = static_cast<const unsigned char*>(m_externalData);
    std::vector<unsigned char> data(ext, ext + m_externalDataSize);
    if (!m_worldTitle.empty())
    {
        data.push_back('T');
        data.push_back((unsigned char)m_worldTitle.size());
        for (size_t i = 0; i < m_worldTitle.size(); ++i)
        {
            const unsigned int unit = (unsigned int)m_worldTitle[i] & 0xFFFFu;
            data.push_back((unsigned char)unit);
            data.push_back((unsigned char)(unit >> 8));
        }
    }
    PcContentAppendSummary(&data);
    m_discovery.SetAdvertisedData(&data[0], (unsigned int)data.size());
    if (m_externalDataSize >= (int)sizeof(GameSessionData)
        && static_cast<GameSessionData*>(m_externalData)->m_bGameStarted)
        m_transport.SetHelloData(&data[0], (unsigned int)data.size());
}

void PcNetworkSession::ReadSessionDataTail(const std::vector<unsigned char>& data, SearchResult* result)
{
    size_t pos = sizeof(GameSessionData);
    while (pos + 2 <= data.size())
    {
        const unsigned char tag = data[pos];
        const size_t n = data[pos + 1];
        const size_t unitLen = tag == 'M' ? 4 : tag == 'T' ? 2 : tag == 'C' ? 1 : 0;
        if (unitLen == 0 || pos + 2 + n * unitLen > data.size())
            break;
        const unsigned char* p = &data[pos + 2];
        if (tag == 'M')
        {
            result->attributes.clear();
            for (size_t a = 0; a < n; ++a, p += 4)
                result->attributes.push_back(p[0] | (p[1] << 8) | (p[2] << 16)
                                             | ((unsigned int)p[3] << 24));
        }
        else if (tag == 'T')
        {
            result->worldTitle.clear();
            for (size_t u = 0; u < n; ++u, p += 2)
                result->worldTitle.push_back((wchar_t)(p[0] | (p[1] << 8)));
        }
        pos += 2 + n * unitLen;
    }
    PcContentParse(data, sizeof(GameSessionData), &result->content);
}

void PcNetworkSession::UpdateExternalRoomData()
{
    if (m_host && !m_localGame && m_externalData != 0)
    {
        PublishSessionData();
    }
}

void PcNetworkSession::ClearSearchResults()
{
    EnterCriticalSection(&m_searchMutex);
    for (unsigned int i = 0; i < m_searchResults.size(); ++i)
        delete m_searchResults[i].sessionData;
    m_searchResults.clear();
    LeaveCriticalSection(&m_searchMutex);
}

bool PcNetworkSession::IsSameGameAsProbedSession(const PcLanDiscovery::Session& session,
                                                 const std::vector<PcLanDiscovery::Session>& sessions)
{
    GameSessionData announced;
    std::memcpy(static_cast<void*>(&announced), &session.data[0], sizeof(GameSessionData));
    for (unsigned int index = 0; index < sessions.size(); ++index)
    {
        if (!sessions[index].manual || sessions[index].data.size() < sizeof(GameSessionData))
            continue;
        GameSessionData probed;
        std::memcpy(static_cast<void*>(&probed), &sessions[index].data[0], sizeof(GameSessionData));
        if (IsSameGame(probed, announced))
            return true;
    }
    return false;
}

bool PcNetworkSession::IsSameGame(const GameSessionData& first, const GameSessionData& second)
{
    bool same = first.m_hostGamertag == second.m_hostGamertag;
    for (int player = 0; same && player < GameSessionData::MAX_SESSION_PLAYERS; ++player)
        same = first.m_playerGamertags[player] == second.m_playerGamertags[player];
    return same;
}

void PcNetworkSession::RefreshSearchResults()
{
    std::vector<PcLanDiscovery::Session> sessions = m_discovery.GetSessions();
    EnterCriticalSection(&m_searchMutex);
    for (unsigned int index = 0; index < m_searchResults.size(); ++index)
        delete m_searchResults[index].sessionData;
    m_searchResults.clear();
    for (unsigned int index = 0; index < sessions.size(); ++index)
    {
        const PcLanDiscovery::Session& session = sessions[index];
        if (session.data.size() < sizeof(GameSessionData))
            continue;
        if (!session.manual && IsSameGameAsProbedSession(session, sessions))
            continue;
        SearchResult result;
        result.sessionId = session.sessionId;
        result.address = session.address;
        result.port = session.port;
        result.lobbyWorld = 0;
        result.friendHost = true;
        result.sessionData = new GameSessionData;
        std::memcpy(static_cast<void*>(result.sessionData), &session.data[0], sizeof(GameSessionData));
        ReadSessionDataTail(session.data, &result);
        if (result.sessionData->m_version != kSessionDataVersion)
        {
            delete result.sessionData;
            continue;
        }
        m_searchResults.push_back(result);
    }
    m_lobbyRevision = PcLobbyClientRevision();
    unsigned int lobbyAddress = 0;
    unsigned short lobbyPort = 0;
    unsigned long long secret = 0;
    std::vector<PcLobbyWorld> worlds = PcLobbyClientWorlds(&lobbyAddress, &lobbyPort, &secret);
    const size_t directResults = m_searchResults.size();
    for (unsigned int index = 0; index < worlds.size(); ++index)
    {
        if (worlds[index].sessionData.size() < sizeof(GameSessionData) || worlds[index].id == 0)
            continue;
        SearchResult result;
        result.sessionId = 0x40000000u | (worlds[index].id & 0x3FFFFFFFu);
        result.address = lobbyAddress;
        result.port = lobbyPort;
        result.lobbyWorld = worlds[index].id;
        result.friendHost = (worlds[index].flags & kListedFriend) != 0;
        result.sessionData = new GameSessionData;
        std::memcpy(static_cast<void*>(result.sessionData), &worlds[index].sessionData[0], sizeof(GameSessionData));
        ReadSessionDataTail(worlds[index].sessionData, &result);
        bool listed = result.sessionData->m_version != kSessionDataVersion;
        for (size_t direct = 0; !listed && direct < directResults; ++direct)
            listed = IsSameGame(*m_searchResults[direct].sessionData, *result.sessionData);
        if (listed)
        {
            delete result.sessionData;
            continue;
        }
        m_searchResults.push_back(result);
    }
    m_searchRevision = m_discovery.GetRevision();
    LeaveCriticalSection(&m_searchMutex);
}

void PcNetworkSession::TickSearch()
{
    if (m_searching)
    {
        if (PcOsMonotonicNanoseconds() - m_searchStartTime >= SEARCH_TIME_NS)
        {
            RefreshSearchResults();
            m_searching = false;
        }
    }
    else if (m_internalState == eNQRNetworkManagerInternalState_Idle
             && (m_discovery.GetRevision() != m_searchRevision || PcLobbyClientRevision() != m_lobbyRevision))
    {
        RefreshSearchResults();
        m_searchResultsChanged = true;
    }

    if (m_extDataRequested)
    {
        bool found = false;
        EnterCriticalSection(&m_searchMutex);
        for (unsigned int i = 0; i < m_searchResults.size(); ++i)
        {
            if (m_searchResults[i].sessionId == m_extDataSessionId)
            {
                std::memcpy(m_extDataBuffer, m_searchResults[i].sessionData, sizeof(GameSessionData));
                found = true;
                break;
            }
        }
        LeaveCriticalSection(&m_searchMutex);
        m_extDataRequested = false;
        if (m_extDataCallback != 0)
            m_extDataCallback(found, m_extDataCallbackData);
    }
}

bool PcNetworkSession::FriendRoomManagerIsBusy()
{
    return m_searching;
}

bool PcNetworkSession::FriendRoomManagerSearch()
{
    if (!m_lanEnabled || m_searching || m_internalState != eNQRNetworkManagerInternalState_Idle)
        return false;
    m_discovery.StartBrowsing();
    m_discovery.ProbeServerList(kServerListPath, m_localStationId);
    m_discovery.ProbeTextServerList(kTextServerListPath, m_localStationId);
    m_searchStartTime = PcOsMonotonicNanoseconds();
    m_searchResultsChanged = false;
    m_searching = true;
    return true;
}

bool PcNetworkSession::FriendRoomManagerSearch2()
{
    return !m_searching;
}

int PcNetworkSession::FriendRoomManagerGetCount()
{
    EnterCriticalSection(&m_searchMutex);
    int count = static_cast<int>(m_searchResults.size());
    LeaveCriticalSection(&m_searchMutex);
    return count;
}

void PcNetworkSession::FriendRoomManagerGetRoomInfo(int index, SessionSearchResult* result)
{
    EnterCriticalSection(&m_searchMutex);
    const SearchResult& found = m_searchResults[index];
    result->m_uiSessionId = found.sessionId;
    result->m_uiOwnerPrincipalId = 0;
    result->m_uiOwnerLocalFriendIndex = 0;
    std::memset(result->m_szOwnerName, 0, sizeof(result->m_szOwnerName));
    std::strncpy(result->m_szOwnerName, found.sessionData->m_hostGamertag.m_name,
                 sizeof(result->m_szOwnerName) - 1);
    result->m_pExtData = found.sessionData;
    LeaveCriticalSection(&m_searchMutex);
}

bool PcNetworkSession::FriendPresenceUpdatedRoomList()
{
    bool changed = m_searchResultsChanged;
    m_searchResultsChanged = false;
    return changed;
}

NQRNetworkManager::SessionSearchResult* PcNetworkSession::GetFriendRoomSearchResults(int* count)
{
    EnterCriticalSection(&m_searchMutex);
    int resultCount = static_cast<int>(m_searchResults.size());
    SessionSearchResult* results = new SessionSearchResult[resultCount > 0 ? resultCount : 1];
    for (int index = 0; index < resultCount; ++index)
        FriendRoomManagerGetRoomInfo(index, &results[index]);
    *count = resultCount;
    LeaveCriticalSection(&m_searchMutex);
    return results;
}

void PcNetworkSession::GetExtDataForRoom(unsigned int id, void* buf, void (*cb)(bool, void*),
                                         void* cbData)
{
    m_extDataSessionId = id;
    m_extDataBuffer = buf;
    m_extDataCallback = cb;
    m_extDataCallbackData = cbData;
    m_extDataRequested = true;
}

bool PcNetworkSession::JoinRoom(SessionSearchResult* result, int mask)
{
    return JoinRoom(result->m_uiSessionId, mask, 0);
}

bool PcNetworkSession::JoinRoom(unsigned int id, int mask, const PresenceSyncInfo*)
{
    unsigned int addr = 0;
    unsigned short port = 0;
    unsigned int world = 0;
    bool dedicated = false;
    EnterCriticalSection(&m_searchMutex);
    for (unsigned int i = 0; i < m_searchResults.size(); ++i)
    {
        if (m_searchResults[i].sessionId == id)
        {
            addr = m_searchResults[i].address;
            port = m_searchResults[i].port;
            world = m_searchResults[i].lobbyWorld;
            dedicated = m_searchResults[i].sessionData != 0 && m_searchResults[i].sessionData->m_pcDedicated != 0;
        }
    }
    LeaveCriticalSection(&m_searchMutex);
    if (port == 0)
        return false;
    const bool ok = JoinRoomByAddress(addr, port, mask, world);
    m_joinedDedicated = ok && dedicated;
    return ok;
}

bool PcNetworkSession::JoinRoomByAddress(unsigned int addr, unsigned short port, int mask,
                                         unsigned int world)
{
    if (!m_lanEnabled || m_internalState != eNQRNetworkManagerInternalState_Idle)
        return false;
    char ip[32];
    PcOsFormatIPv4(addr, ip, sizeof(ip));
    printf("[pc] lan: joining %s:%u\n", ip, port);
    fflush(stdout);
    m_discovery.StopBrowsing();
    m_joinedDedicated = false;
    m_roomPlayerMask = (unsigned int)mask;
    m_localPlayerCount = CountPlayers(m_roomPlayerMask);
    m_localPlayersJoined = 0;
    m_host = false;
    m_receiving = true;
    m_localGame = false;
    m_hostStationId = 0;
    EnterCriticalSection(&m_playerMutex);
    std::memset(static_cast<void*>(m_roomPlayers), 0, sizeof(m_roomPlayers));
    LeaveCriticalSection(&m_playerMutex);
    if (gApp.GetDisconnectReason() == DisconnectPacket::eDisconnectReason_PcMismatchedContent)
        gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_None);
    SetState(eNQRNetworkManagerInternalState_JoinRequested);
    SetState(eNQRNetworkManagerInternalState_JoinJoiningSession);
    PcOsSocket sock = kPcOsInvalidSocket;
    unsigned long long probedId = 0;
    std::vector<unsigned char> probedHello;
    if (world != 0)
    {
        unsigned int lobbyAddr = 0;
        unsigned short lobbyPort = 0;
        unsigned long long secret = 0;
        PcLobbyClientWorlds(&lobbyAddr, &lobbyPort, &secret);
        printf("[pc] lan: joining world %u through the lobby\n", world);
        fflush(stdout);
        m_transport.ConnectRelay(m_localStationId, addr, port, world, secret);
    }
    else if (m_discovery.TakeProbedConnection(addr, port, &sock, &probedId, &probedHello))
        m_transport.Adopt(m_localStationId, sock, probedId, addr, port, probedHello);
    else
        m_transport.Connect(m_localStationId, addr, port);
    return true;
}

bool PcNetworkSession::IsLobbySession(unsigned int id)
{
    return (id & 0x40000000u) != 0;
}

int PcNetworkSession::VoicePeerCount()
{
    return voicePeerCount;
}

void PcNetworkSession::TickVoice()
{
    std::vector<std::pair<unsigned long long, int> > listeners;
    int speaker = -1;
    int others = 0;
    EnterCriticalSection(&m_playerMutex);
    const int n = m_roomPlayers[0].m_uiStationId == 0 ? 0 : m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
    for (int i = 0; i < n; ++i)
    {
        const PlayerSyncData& slot = m_roomPlayers[i];
        if (slot.m_uiStationId == m_localStationId)
        {
            if (speaker < 0)
                speaker = (int)slot.m_uiSmallId;
            continue;
        }
        if (i == 0 && m_joinedDedicated)
            continue;
        ++others;
        bool listed = false;
        for (size_t j = 0; j < listeners.size(); ++j)
            listed = listed || listeners[j].first == slot.m_uiStationId;
        if (!listed)
            listeners.push_back(std::make_pair(slot.m_uiStationId, (int)slot.m_uiSmallId));
    }
    LeaveCriticalSection(&m_playerMutex);
    voicePeerCount = m_lanEnabled && !m_localGame ? others : 0;
    const unsigned long long now = PcOsMonotonicNanoseconds();
    if (n == 0 || !m_lanEnabled || m_localGame)
    {
        voicePrefs.clear();
        PcVoiceClearHostOverride();
    }
    else if (now >= nextVoicePrefsTime)
    {
        nextVoicePrefsTime = now + VOICE_PREFS_INTERVAL_NS;
        unsigned char prefs[2];
        if (m_host)
        {
            prefs[0] = (unsigned char)PcSettingsHostProximity();
            prefs[1] = (unsigned char)PcSettingsHostProximityRange();
            PcVoiceSetHostOverride(prefs[0], prefs[1]);
            for (size_t i = 0; i < listeners.size(); ++i)
                QueueSend(listeners[i].first, CHANNEL_VOICE_PREFS, prefs, sizeof(prefs));
        }
        else if (m_hostStationId != 0)
        {
            prefs[0] = PcSettingsProximityChat() ? 1 : 0;
            prefs[1] = (unsigned char)PcSettingsProximityRange();
            QueueSend(m_hostStationId, CHANNEL_VOICE_PREFS, prefs, sizeof(prefs));
        }
    }
    std::vector<unsigned char> frame;
    while (PcVoiceTakeOutgoing(&frame))
    {
        if (voicePeerCount == 0 || speaker < 0 || frame.empty() || frame.size() > MAX_VOICE_BYTES)
            continue;
        std::vector<unsigned char> packet(frame.size() + VOICE_HEADER_SIZE);
        packet[0] = 0xFF;
        packet[1] = kPcVoiceFullGain;
        packet[2] = kPcVoiceCentrePan;
        std::memcpy(&packet[VOICE_HEADER_SIZE], &frame[0], frame.size());
        if (!m_host)
        {
            if (m_hostStationId != 0)
                QueueSend(m_hostStationId, CHANNEL_VOICE, &packet[0], (unsigned int)packet.size());
            continue;
        }
        packet[0] = (unsigned char)speaker;
        for (size_t i = 0; i < listeners.size(); ++i)
        {
            int range = 0;
            const bool proximity = ListenerProximity(listeners[i].first, false, &range);
            if (!PcVoiceRoute(speaker, listeners[i].second, proximity, range, &packet[1], &packet[2]))
                continue;
            QueueSend(listeners[i].first, CHANNEL_VOICE, &packet[0], (unsigned int)packet.size());
        }
    }
}

void PcNetworkSession::HandleVoice(unsigned long long station, const unsigned char* data, unsigned int len)
{
    if (len <= VOICE_HEADER_SIZE || len > MAX_VOICE_BYTES + VOICE_HEADER_SIZE || m_localGame)
        return;
    if (!m_host)
    {
        if (station == m_hostStationId)
            PcVoiceReceive(data[0], data[1], data[2], data + VOICE_HEADER_SIZE, len - VOICE_HEADER_SIZE);
        return;
    }
    int speaker = -1;
    int hostListener = -1;
    std::vector<std::pair<unsigned long long, int> > listeners;
    EnterCriticalSection(&m_playerMutex);
    const int n = m_roomPlayers[0].m_uiStationId == 0 ? 0 : m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
    for (int i = 0; i < n; ++i)
    {
        const PlayerSyncData& slot = m_roomPlayers[i];
        if (slot.m_uiStationId == station)
        {
            if (speaker < 0)
                speaker = (int)slot.m_uiSmallId;
            continue;
        }
        if (slot.m_uiStationId == m_localStationId)
        {
            if (hostListener < 0)
                hostListener = (int)slot.m_uiSmallId;
            continue;
        }
        bool listed = false;
        for (size_t j = 0; j < listeners.size(); ++j)
            listed = listed || listeners[j].first == slot.m_uiStationId;
        if (!listed)
            listeners.push_back(std::make_pair(slot.m_uiStationId, (int)slot.m_uiSmallId));
    }
    LeaveCriticalSection(&m_playerMutex);
    if (speaker < 0)
        return;
    std::vector<unsigned char> packet(data, data + len);
    packet[0] = (unsigned char)speaker;
    int range = 0;
    bool proximity = ListenerProximity(m_localStationId, true, &range);
    if (hostListener >= 0 && PcVoiceRoute(speaker, hostListener, proximity, range, &packet[1], &packet[2]))
        PcVoiceReceive(speaker, packet[1], packet[2], data + VOICE_HEADER_SIZE, len - VOICE_HEADER_SIZE);
    for (size_t i = 0; i < listeners.size(); ++i)
    {
        proximity = ListenerProximity(listeners[i].first, false, &range);
        if (!PcVoiceRoute(speaker, listeners[i].second, proximity, range, &packet[1], &packet[2]))
            continue;
        QueueSend(listeners[i].first, CHANNEL_VOICE, &packet[0], (unsigned int)packet.size());
    }
}

bool PcNetworkSession::JoinedDedicatedServer()
{
    PcNetworkSession* s = s_lanSession;
    return s != 0 && !s->m_host && s->m_joinedDedicated;
}

bool PcNetworkSession::IsFriendSession(unsigned int sessionId)
{
    PcNetworkSession* session = s_lanSession;
    if (session == 0)
        return false;
    bool friendHost = false;
    EnterCriticalSection(&session->m_searchMutex);
    for (size_t index = 0; index < session->m_searchResults.size(); ++index)
    {
        if (session->m_searchResults[index].sessionId == sessionId)
        {
            friendHost = session->m_searchResults[index].friendHost;
            break;
        }
    }
    LeaveCriticalSection(&session->m_searchMutex);
    return friendHost;
}

bool PcNetworkSession::HostedSessionData(std::vector<unsigned char>* data)
{
    data->clear();
    if (s_lanSession == 0 || !s_lanSession->m_host || s_lanSession->m_localGame
        || !s_lanSession->m_transport.IsHosting())
        return false;
    if (!s_lanSession->m_transport.GetHelloData(data))
        return false;
    std::vector<unsigned char> title;
    if (data->size() > sizeof(GameSessionData))
    {
        title.assign(data->begin() + sizeof(GameSessionData), data->end());
        data->resize(sizeof(GameSessionData));
    }
    if (s_lanSession->m_matchmaking && s_lanSession->m_roomAttributes != 0)
    {
        data->push_back('M');
        data->push_back(static_cast<unsigned char>(s_lanSession->m_roomAttributeCount));
        for (int index = 0; index < s_lanSession->m_roomAttributeCount; ++index)
        {
            const unsigned int value = static_cast<unsigned int>(s_lanSession->m_roomAttributes[index]);
            for (int byte = 0; byte < 4; ++byte)
                data->push_back(static_cast<unsigned char>(value >> (byte * 8)));
        }
    }
    data->insert(data->end(), title.begin(), title.end());
    return true;
}

void PcNetworkSession::SetPublicWorldTitle(const wchar_t* title)
{
    pendingTitle.assign(title != 0 ? title : L"");
    if (pendingTitle.size() > MAX_TITLE_LENGTH)
        pendingTitle.resize(MAX_TITLE_LENGTH);
}

std::wstring PcNetworkSession::WorldTitle(unsigned int id)
{
    std::wstring title;
    if (s_lanSession == 0)
        return title;
    EnterCriticalSection(&s_lanSession->m_searchMutex);
    for (size_t i = 0; i < s_lanSession->m_searchResults.size(); ++i)
    {
        if (s_lanSession->m_searchResults[i].sessionId == id)
        {
            title = s_lanSession->m_searchResults[i].worldTitle;
            break;
        }
    }
    LeaveCriticalSection(&s_lanSession->m_searchMutex);
    return title;
}

std::wstring PcNetworkSession::ContentLabel(unsigned int id)
{
    std::wstring label;
    if (s_lanSession == 0)
        return label;
    EnterCriticalSection(&s_lanSession->m_searchMutex);
    for (size_t i = 0; i < s_lanSession->m_searchResults.size(); ++i)
    {
        if (s_lanSession->m_searchResults[i].sessionId == id)
        {
            label = PcContentListLabel(s_lanSession->m_searchResults[i].content);
            break;
        }
    }
    LeaveCriticalSection(&s_lanSession->m_searchMutex);
    return label;
}

bool PcNetworkSession::MatchesSearch(const SearchResult& result, unsigned char mode, unsigned int numParams,
                                     const CMatchMakeSearchParam* params)
{
    const GameSessionData& data = *result.sessionData;
    if (!data.m_bPublicGame || data.m_miniGameId != (mode & 0xF) || !PcContentMatches(result.content))
        return false;
    const int slots = CGameNetworkManager::sInstance.GetMaxPlayers(const_cast<GameSessionData*>(&data))
                          - (int)data.m_numPlayers;
    for (unsigned int i = 0; i < numParams; ++i)
    {
        unsigned int v = 0;
        if (params[i].id < result.attributes.size())
            v = result.attributes[params[i].id];
        else if (params[i].id == 1)
            v = slots > 0 ? (unsigned int)slots : 0;
        else if (result.attributes.empty())
            continue;
        else
            return false;
        const unsigned int wanted = params[i].value;
        bool ok = true;
        switch (params[i].comparison)
        {
        case CMatchMakeSearchParam::eComparison_Equal:
            ok = v == wanted;
            break;
        case CMatchMakeSearchParam::eComparison_NotEqual:
            ok = v != wanted;
            break;
        case CMatchMakeSearchParam::eComparison_LessThan:
            ok = v < wanted;
            break;
        case CMatchMakeSearchParam::eComparison_LessThanOrEqual:
            ok = v <= wanted;
            break;
        case CMatchMakeSearchParam::eComparison_GreaterThan:
            ok = v > wanted;
            break;
        case CMatchMakeSearchParam::eComparison_GreaterThanOrEqual:
            ok = v >= wanted;
            break;
        }
        if (!ok)
            return false;
    }
    return slots > 0;
}

int PcNetworkSession::CountMatchmakingGames(unsigned char game)
{
    PcNetworkSession* s = s_lanSession;
    if (s == 0 || !s->m_lanEnabled || s->m_internalState != eNQRNetworkManagerInternalState_Idle)
        return 0;
    s->m_discovery.StartBrowsing();
    s->RefreshSearchResults();
    int n = 0;
    EnterCriticalSection(&s->m_searchMutex);
    for (unsigned int i = 0; i < s->m_searchResults.size(); ++i)
    {
        if (MatchesSearch(s->m_searchResults[i], game, 0, 0))
            ++n;
    }
    LeaveCriticalSection(&s->m_searchMutex);
    return n;
}

bool PcNetworkSession::AcceptRelayedConnection(PcOsSocket socket, unsigned int address,
                                               const PcVerifiedIdentities* verified)
{
    if (s_lanSession == 0 || !s_lanSession->m_host || s_lanSession->m_localGame)
        return false;
    return s_lanSession->m_transport.AcceptRelayed(socket, address, verified);
}

bool PcNetworkSession::VerifiedJoinUid(int smallId, PlayerUID* uid)
{
    PcNetworkSession* session = s_lanSession;
    if (session == 0)
        return false;
    NQRNetworkPlayer* player = session->GetPlayerBySmallId(smallId);
    PcVerifiedIdentities verified;
    if (player == 0 || !session->m_transport.GetVerifiedIdentities(player->GetStationId(), &verified))
        return false;
    *uid = player->GetUID();
    return true;
}

bool PcNetworkSession::VerifiedUuid(int smallId, unsigned char uuid[16])
{
    PcNetworkSession* session = s_lanSession;
    if (session == 0)
        return false;
    NQRNetworkPlayer* player = session->GetPlayerBySmallId(smallId);
    PcVerifiedIdentities verified;
    if (player == 0 || !session->m_transport.GetVerifiedIdentities(player->GetStationId(), &verified))
        return false;
    const PlayerUID uid = player->GetUID();
    if ((uid.flags & PlayerUID::FlagUuid) == 0 || !verified.Contains(uid.uuid.data))
        return false;
    memcpy(uuid, uid.uuid.data, 16);
    return true;
}

bool PcNetworkSession::AddDirectHost(unsigned int addr, unsigned short port)
{
    if (s_lanSession == 0)
        return false;
    s_lanSession->m_discovery.StartBrowsing();
    s_lanSession->m_discovery.ProbeAddress(addr, port, s_lanSession->m_localStationId);
    return true;
}

bool PcNetworkSession::WatchDirectHost(unsigned int addr, unsigned short port, bool any)
{
    if (s_lanSession == 0)
        return false;
    PcLanDiscovery::DropClosedProbes();
    if (any || s_lanSession->m_internalState == eNQRNetworkManagerInternalState_Idle)
        s_lanSession->m_discovery.ProbeAddress(addr, port, s_lanSession->m_localStationId);
    return true;
}

PcLanDiscovery::ProbeStatus PcNetworkSession::DirectHostStatus(unsigned int addr, unsigned short port)
{
    return PcLanDiscovery::GetProbeStatus(addr, port);
}

unsigned int PcNetworkSession::DirectHostSessionId(unsigned int addr, unsigned short port)
{
    return s_lanSession != 0 ? s_lanSession->m_discovery.FindSessionId(addr, port) : 0;
}

void PcNetworkSession::DescribeState(char* buf, unsigned int len)
{
    if (s_lanSession == 0)
    {
        snprintf(buf, len, "no session");
        return;
    }
    EnterCriticalSection(&s_lanSession->m_stateChangeLock);
    const unsigned int pending = (unsigned int)s_lanSession->m_stateChanges.size();
    LeaveCriticalSection(&s_lanSession->m_stateChangeLock);
    snprintf(buf, len, "state %d/%d host %d receiving %d players %u pending %u",
             (int)s_lanSession->m_internalState, (int)s_lanSession->m_state,
             s_lanSession->m_host ? 1 : 0, s_lanSession->m_receiving ? 1 : 0,
             (unsigned int)s_lanSession->m_players.size(), pending);
}

bool PcNetworkSession::JoinRoom(int, const nn::fp::FriendPresence*)
{
    return false;
}

bool PcNetworkSession::FindAndJoinRoom(int mask, unsigned int numParams, CMatchMakeSearchParam* params,
                                       unsigned char mode)
{
    if (!m_lanEnabled || m_internalState != eNQRNetworkManagerInternalState_Idle)
        return false;
    RefreshSearchResults();
    std::vector<unsigned int> matches;
    EnterCriticalSection(&m_searchMutex);
    for (unsigned int i = 0; i < m_searchResults.size(); ++i)
    {
        if (MatchesSearch(m_searchResults[i], mode, numParams, params))
            matches.push_back(m_searchResults[i].sessionId);
    }
    LeaveCriticalSection(&m_searchMutex);
    printf("[pc] lan: matchmaking for mini game %u found %u session(s)\n", (unsigned int)mode,
           (unsigned int)matches.size());
    fflush(stdout);
    if (matches.empty())
        return false;
    const unsigned int pick = matches[(unsigned int)(PcOsMonotonicNanoseconds() / 1000ULL) % matches.size()];
    if (!JoinRoom(pick, mask, 0))
        return false;
    m_matchmaking = true;
    return true;
}

void PcNetworkSession::StartGame()
{
    SetState(eNQRNetworkManagerInternalState_StartingGame);
    SetState(eNQRNetworkManagerInternalState_Playing);
}

void PcNetworkSession::LeaveRoom(bool reallyLeave)
{
    if (m_internalState == eNQRNetworkManagerInternalState_LeavingRoom)
        return;
    if (!reallyLeave)
    {
        SetState(eNQRNetworkManagerInternalState_Ending);
        return;
    }
    switch (m_internalState)
    {
    case eNQRNetworkManagerInternalState_HostSessionCreated:
    case eNQRNetworkManagerInternalState_HostRoomReady:
    case eNQRNetworkManagerInternalState_JoinJoiningSession:
    case eNQRNetworkManagerInternalState_JoinRoomReady:
    case eNQRNetworkManagerInternalState_StartingGame:
    case eNQRNetworkManagerInternalState_Playing:
        if (!m_host && !m_localGame)
            RemoveLocalClientPlayers(ALL_PLAYERS);
        SetState(eNQRNetworkManagerInternalState_LeavingRoom);
        SetState(eNQRNetworkManagerInternalState_Ending);
        break;
    default:
        SetState(eNQRNetworkManagerInternalState_Idle);
        break;
    }
}

void PcNetworkSession::EndGame()
{
}

bool PcNetworkSession::SessionHasSpace(int extra)
{
    return m_roomPlayers[0].m_uiPlayerCountMinusOne + extra + 1
           <= (int)CGameNetworkManager::sInstance.GetMaxPlayers();
}

bool PcNetworkSession::SessionHasMii(int user)
{
    PlayerUID uid(0);
    gProfile->GetXUID(user, &uid, false);
    for (int i = 0; i < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++i)
    {
        if (m_roomPlayers[i] == uid)
            return true;
    }
    return false;
}

void PcNetworkSession::UpdateRemotePlay()
{
    int n = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (GetLocalPlayerByUserIndex(i) != 0)
            ++n;
    }
    g_pInput->SetLocalMultiplayer(n > 1);
}

bool PcNetworkSession::AddLocalPlayerByUserIndex(int user)
{
    if (!m_host)
    {
        unsigned int mask = 1u << user;
        if (m_localGame || (m_roomPlayerMask & mask) != 0
            || m_roomPlayers[0].m_uiPlayerCountMinusOne + 1 >= (int)CGameNetworkManager::sInstance.GetMaxPlayers())
            return false;
        m_roomPlayerMask |= mask;
        ++m_localPlayerCount;
        SendJoinData(mask);
        return true;
    }
    EnterCriticalSection(&m_playerMutex);
    int count = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
    if (count >= kMaxRoomPlayers
        || count >= (int)CGameNetworkManager::sInstance.GetMaxPlayers())
    {
        LeaveCriticalSection(&m_playerMutex);
        return false;
    }
    int at;
    for (at = 0; at < count; ++at)
    {
        if (m_roomPlayers[at].m_uiStationId != m_localStationId)
            break;
        if ((int)m_roomPlayers[at].m_uiLocalPlayerIndex == user)
        {
            LeaveCriticalSection(&m_playerMutex);
            return false;
        }
    }
    for (int i = count; i > at; --i)
        m_roomPlayers[i] = m_roomPlayers[i - 1];
    unsigned int countMinusOne = m_roomPlayers[0].m_uiPlayerCountMinusOne;
    FillLocalSlot(m_roomPlayers[at], user);
    if (m_host)
        MakeNetworkNameUnique(m_roomPlayers[at]);
    m_roomPlayers[0].m_uiPlayerCountMinusOne = countMinusOne + 1;
    LeaveCriticalSection(&m_playerMutex);
    MapRoomSlotPlayers();
    SyncRoomData();
    UpdateRemotePlay();
    return true;
}

bool PcNetworkSession::RemoveLocalPlayerByUserIndex(int user)
{
    if (!m_host)
    {
        unsigned int mask = 1u << user;
        if ((m_roomPlayerMask & mask) == 0)
            return false;
        m_roomPlayerMask &= ~mask;
        SendLeaveData(mask);
        RemoveLocalClientPlayers(mask);
        UpdateRemotePlay();
        return true;
    }
    EnterCriticalSection(&m_playerMutex);
    int count = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
    for (int i = 1; i < count; ++i)
    {
        if (m_roomPlayers[i].m_uiStationId == m_localStationId
            && (int)m_roomPlayers[i].m_uiLocalPlayerIndex == user)
        {
            unsigned int countMinusOne = m_roomPlayers[0].m_uiPlayerCountMinusOne - 1;
            for (int j = i; j < count - 1; ++j)
                m_roomPlayers[j] = m_roomPlayers[j + 1];
            std::memset(static_cast<void*>(&m_roomPlayers[count - 1]), 0, sizeof(PlayerSyncData));
            m_roomPlayers[0].m_uiPlayerCountMinusOne = countMinusOne;
            LeaveCriticalSection(&m_playerMutex);
            MapRoomSlotPlayers();
            SyncRoomData();
            UpdateRemotePlay();
            return true;
        }
    }
    LeaveCriticalSection(&m_playerMutex);
    return false;
}

void PcNetworkSession::SendInviteGUI()
{
}

int PcNetworkSession::GetPlayerCount()
{
    return m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
}

int PcNetworkSession::GetOnlinePlayerCount()
{
    int n = 0;
    for (int i = 0; i < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++i)
    {
        if (m_roomPlayers[i].m_uiStationId != m_localStationId)
            ++n;
    }
    return n;
}

NQRNetworkPlayer* PcNetworkSession::GetPlayerByIndex(int i)
{
    if (i < 0 || i >= kMaxRoomPlayers)
        return 0;
    NQRNetworkPlayer* p = m_roomSlotPlayers[i];
    return p != 0 && p->IsReady() ? p : 0;
}

NQRNetworkPlayer* PcNetworkSession::GetPlayerBySmallId(int smallId)
{
    EnterCriticalSection(&m_playerMutex);
    NQRNetworkPlayer* p = 0;
    for (int i = 0; i < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++i)
    {
        if ((int)m_roomPlayers[i].m_uiSmallId == smallId)
        {
            p = GetPlayerByIndex(i);
            break;
        }
    }
    LeaveCriticalSection(&m_playerMutex);
    return p;
}

NQRNetworkPlayer* PcNetworkSession::GetPlayerByXuid(PlayerUID uid)
{
    EnterCriticalSection(&m_playerMutex);
    NQRNetworkPlayer* p = 0;
    for (int i = 0; i < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++i)
    {
        if (m_roomPlayers[i] == uid)
        {
            p = GetPlayerByIndex(i);
            break;
        }
    }
    LeaveCriticalSection(&m_playerMutex);
    return p;
}

std::wstring PcNetworkSession::GetDisplayNameByGamertag(std::wstring gamertag)
{
    return gamertag;
}

NQRNetworkPlayer* PcNetworkSession::GetLocalPlayerByUserIndex(int user)
{
    EnterCriticalSection(&m_playerMutex);
    NQRNetworkPlayer* p = 0;
    for (int i = 0; i < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++i)
    {
        if (m_roomPlayers[i].m_uiStationId == m_localStationId
            && (int)m_roomPlayers[i].m_uiLocalPlayerIndex == user)
        {
            p = GetPlayerByIndex(i);
            break;
        }
    }
    LeaveCriticalSection(&m_playerMutex);
    return p;
}

NQRNetworkPlayer* PcNetworkSession::GetHostPlayer()
{
    EnterCriticalSection(&m_playerMutex);
    NQRNetworkPlayer* p = m_roomSlotPlayers[0];
    LeaveCriticalSection(&m_playerMutex);
    return p;
}

void PcNetworkSession::SetPresenceDataStartHostingGame()
{
}

int PcNetworkSession::GetJoiningReadyPercentage()
{
    return 100;
}

void PcNetworkSession::LocalDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data, unsigned int size)
{
    if (m_listener != 0)
        m_listener->HandleDataReceived(from, to, static_cast<unsigned char*>(const_cast<void*>(data)), size);
}

void PcNetworkSession::RemoteDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data,
                                      unsigned int size)
{
    if (m_localGame)
        return;
    QueueSend(to->GetStationId(), (unsigned char)from->GetLocalPlayerIndex(), data, size);
    Tick();
}

int PcNetworkSession::GetSessionIndex(NQRNetworkPlayer* player)
{
    for (int i = 0; i < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++i)
    {
        if (m_roomSlotPlayers[i] == player)
            return i;
    }
    return 0;
}

void PcNetworkSession::TryToConnectOnline(int (*cb)(void*, bool, int), void* cbData, bool)
{
    bool online = m_lanEnabled && PcOsNetInitialise();
    if (online)
        gProfile->SetNetworkStatus(true);
    if (cb != 0)
        cb(cbData, online, 0);
}

bool PcNetworkSession::isAdhocConnected()
{
    return false;
}

bool PcNetworkSession::connectToAdhoc()
{
    return false;
}

bool PcNetworkSession::isLanConnected()
{
    return true;
}
