#include "platform/pc/PcWorldLauncher.hpp"

#include "platform/pc/PcNetworkSession.hpp"
#include "platform/pc/PcOs.hpp"

#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/User.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/LevelData.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/CPlatformNetworkManager.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/pc/PcDedicated.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "vendor/_NetworkGameInitData.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <vector>

extern C4JStorage* gStorage;
extern C_4JProfile* gProfile;
extern CConsoleMinecraftApp gApp;

namespace
{
const int kWorldWidths[] = {54, 64, 192, 320, 640};
const int kHellScales[] = {3, 3, 6, 8, 8};
const int PROGRESS_CLOSE_MODE = 3;
const int CONFIRM_CONTROL_ID = 0;
const int GAME_PAD_CONTROLLER_ID = 4;
const int SCENE_SETTLE_FRAMES = 30;
const unsigned long long NANOSECONDS_PER_MILLISECOND = 1000000ULL;
const unsigned long long REPORT_INTERVAL_NANOSECONDS = 5000ULL * NANOSECONDS_PER_MILLISECOND;
const unsigned long long SAVE_TIMEOUT_NANOSECONDS = 60000ULL * NANOSECONDS_PER_MILLISECOND;
const char SAVE_LIST_FILTER[] = "save";
const unsigned long long JOIN_SEARCH_TIMEOUT_NANOSECONDS = 30000ULL * NANOSECONDS_PER_MILLISECOND;
const unsigned long long INTERACTIVE_JOIN_TIMEOUT_NANOSECONDS = 15000ULL * NANOSECONDS_PER_MILLISECOND;
const unsigned long long EXIT_REPORT_AFTER_NANOSECONDS = 15000ULL * NANOSECONDS_PER_MILLISECOND;
const unsigned long long EXIT_REPORT_INTERVAL_NANOSECONDS = 10000ULL * NANOSECONDS_PER_MILLISECOND;

enum LaunchState
{
    eLaunch_Idle,
    eLaunch_WaitForMenu,
    eLaunch_FindSession,
    eLaunch_ListSaves,
    eLaunch_LoadThumbnail,
    eLaunch_LoadSave,
    eLaunch_Starting,
    eLaunch_InGame,
    eLaunch_Saving,
    eLaunch_Done
};

enum CallbackResult
{
    eCallback_Pending,
    eCallback_Succeeded,
    eCallback_Failed
};

struct WorldRequest
{
    WorldRequest()
        : hasSeed(false), seed(0), worldSize(1), roamStep(0), host(false), join(false), joinSessionId(0), joinAddress(0), joinPort(0),
          quitOnDisconnect(false), interactive(false), resetNether(false), resetEnd(false)
    {
    }
    std::wstring name;
    bool hasSeed;
    bool creative;
    int visibility;
    long long seed;
    int worldSize;
    int roamStep;
    bool host;
    bool join;
    std::wstring joinHost;
    unsigned int joinSessionId;
    unsigned int joinAddress;
    unsigned short joinPort;
    bool quitOnDisconnect;
    bool interactive;
    bool resetNether;
    bool resetEnd;
};

WorldRequest g_request;
LaunchState g_state = eLaunch_Idle;
bool g_acceptEula = false;
bool g_eulaReported = false;
bool g_quitRequested = false;
bool g_wasPlaying = false;
bool g_exiting = false;
unsigned long long g_exitStart = 0;
unsigned long long g_exitReport = 0;
unsigned long long g_processStart = 0;
double g_leaveAtSeconds = -1.0;
UIScene* g_bootScene = 0;
int g_bootSceneFrames = 0;
int g_menuFrames = 0;
unsigned long long g_startTime = 0;
unsigned long long g_lastReport = 0;
unsigned long long g_saveStart = 0;
int g_saveIndex = -1;
unsigned int g_savedTexturePack = 0;
std::vector<unsigned char> g_thumbnail;
volatile CallbackResult g_callbackResult = eCallback_Pending;

unsigned long long MillisecondsSinceStart()
{
    return (PcOsMonotonicNanoseconds() - g_startTime) / NANOSECONDS_PER_MILLISECOND;
}

bool ConfirmBootScene()
{
    UIScene* top = gConsoleUIController.GetTopScene(0, EUILayer_Scene, EUIGroup_PlayerDefault);
    if (top == 0)
        return true;
    if (top != g_bootScene)
    {
        g_bootScene = top;
        g_bootSceneFrames = 0;
    }
    if (++g_bootSceneFrames < SCENE_SETTLE_FRAMES)
        return true;

    const int sceneType = top->getSceneType();
    if (sceneType == EUIScene_EULA)
    {
        if (!g_acceptEula)
        {
            if (!g_eulaReported)
                printf("[pc] world: waiting at the EULA; accept it or pass --accept-eula\n");
            g_eulaReported = true;
            fflush(stdout);
            g_bootSceneFrames = 0;
            return true;
        }
        printf("[pc] world: accepting the EULA (--accept-eula)\n");
        top->handlePress(CONFIRM_CONTROL_ID, 0);
    }
    else if (sceneType == EUIScene_SaveMessage)
    {
        top->handlePress(CONFIRM_CONTROL_ID, 0);
    }
    g_bootSceneFrames = 0;
    return true;
}

void LockPrimaryPad()
{
    if (!g_pInputInstance->IsPadLocked(0))
        gProfile->SetPrimaryPad(GAME_PAD_CONTROLLER_ID);
}

void PrepareToPlay(Minecraft* mc)
{
    LockPrimaryPad();
    const int pad = gProfile->GetPrimaryPad();
    gProfile->SetLockedProfile(pad);
    gProfile->QuerySigninStatus();
    mc->mUser->name = convStringToWstring(gProfile->GetGamertag(pad));
    GameSettings::ApplyGameSettingsChanged(pad);
    gApp.setLevelGenerationOptions(0);
    gApp.SetTutorialMode(false, boost::shared_ptr<Player>());
    gApp.SetCorruptSaveDeleted(false);
}

void ShowLoadingScene(_NetworkGameInitData* initData)
{
    CGameNetworkManager::sInstance.FakeLocalPlayerJoined();
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_threadProc = &CGameNetworkManager::RunNetworkGameThreadProc;
    params->m_threadParam = initData;
    gApp.SetAutosaveTimerTime();
    FullscreenProgressState* state = new FullscreenProgressState;
    state->closeMode = PROGRESS_CLOSE_MODE;
    state->playerId = gProfile->GetPrimaryPad();
    params->m_completionData = state;
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                         EUILayer_Scene, EUIGroup_PlayerDefault);
}

void SetDefaultHostOptions(int pad, int gameType)
{
    gApp.SetGameHostOption(eGameHostOption_Difficulty, GameSettings::GetGameSettings(pad, eGameSetting_Difficulty));
    gApp.SetGameHostOption(eGameHostOption_FriendsOfFriends, 1);
    gApp.SetGameHostOption(eGameHostOption_Gamertags, GameSettings::GetGameSettings(pad, eGameSetting_DisplayMii));
    gApp.SetGameHostOption(eGameHostOption_GameType, gameType);
    gApp.SetGameHostOption(eGameHostOption_LevelType, 0);
    gApp.SetGameHostOption(eGameHostOption_WorldSize, g_request.worldSize);
    gApp.SetGameHostOption(eGameHostOption_Structures, 1);
    gApp.SetGameHostOption(eGameHostOption_BonusChest, 0);
    gApp.SetGameHostOption(eGameHostOption_PvP, 1);
    gApp.SetGameHostOption(eGameHostOption_TrustPlayers, 1);
    gApp.SetGameHostOption(eGameHostOption_FireSpreads, 1);
    gApp.SetGameHostOption(eGameHostOption_TNT, 1);
    gApp.SetGameHostOption(eGameHostOption_HostCanFly, 0);
    gApp.SetGameHostOption(eGameHostOption_HostCanChangeHunger, 0);
    gApp.SetGameHostOption(eGameHostOption_HostCanBeInvisible, 0);
    gApp.SetGameHostOption(eGameHostOption_MobGriefing, 1);
    gApp.SetGameHostOption(eGameHostOption_KeepInventory, 0);
    gApp.SetGameHostOption(eGameHostOption_DoMobSpawning, 1);
    gApp.SetGameHostOption(eGameHostOption_DoMobLoot, 1);
    gApp.SetGameHostOption(eGameHostOption_DoTileDrops, 1);
    gApp.SetGameHostOption(eGameHostOption_NaturalRegeneration, 1);
    gApp.SetGameHostOption(eGameHostOption_DoDaylightCycle, 1);
    gApp.SetGameHostOption(eGameHostOption_DoWeatherCycle, 1);
}

void HostNewWorld(Minecraft* mc)
{
    const int pad = gProfile->GetPrimaryPad();
    const int gameType = (g_request.creative ? GameType::CREATIVE : GameType::SURVIVAL)->getId();

    gStorage->ResetSaveData();
    gStorage->SetSaveTitle(g_request.name.c_str());
    gApp.ClearTerrainFeaturePosition();

    _NetworkGameInitData* initData = new _NetworkGameInitData();
    initData->m_seed = g_request.seed;
    initData->m_findSeed = !g_request.hasSeed;
    initData->m_levelGenerationOptions = 0;
    initData->m_texturePackId = 0;
    initData->m_saveData = 0;
    initData->m_gameType = gameType;
    initData->m_xzSize = kWorldWidths[g_request.worldSize - 1];
    initData->m_hellScale = kHellScales[g_request.worldSize - 1];
    mc->mTexturePackRepository->selectTexturePackById(0, false);

    SetDefaultHostOptions(pad, gameType);
    gApp.SetResetNether(false);
    gApp.SetResetEnd(false);

    if (g_request.visibility != 0)
        gApp.SetGameHostOption(eGameHostOption_FriendsOfFriends, g_request.visibility == 2 ? 1 : 0);
    if (PcIsDedicatedServer())
        gApp.SetGameHostOption(eGameHostOption_CheatsEnabled, 1);
    CGameNetworkManager::sInstance.HostGame(0, g_request.host, g_request.visibility != 0,
                                            (unsigned char)gameType, eMiniGameId_Any, 0, 0);
    initData->m_hostOptions = gApp.GetGameHostOption(eGameHostOption_All);
    initData->m_miniGameHostOptions =
        MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true)->GetHostOption(eCustomGameModeHostOption_All);
    ShowLoadingScene(initData);
}

void HostSavedWorld(Minecraft* mc)
{
    gApp.ClearTerrainFeaturePosition();
    const int gameType = gApp.GetGameHostOption(eGameHostOption_GameType);

    _NetworkGameInitData* initData = new _NetworkGameInitData();
    initData->m_recordLevelResume = 1;
    initData->m_levelGenerationOptions = 0;
    initData->m_texturePackId = g_savedTexturePack;
    initData->m_saveData = 0;
    initData->m_gameType = gameType;
    mc->mTexturePackRepository->selectTexturePackById(initData->m_texturePackId, false);
    gApp.SetResetNether(g_request.resetNether);
    gApp.SetResetEnd(g_request.resetEnd);
    if (g_request.resetNether)
        printf("[pc] world: resetting the Nether\n");
    if (g_request.resetEnd)
        printf("[pc] world: resetting the End\n");

    if (g_request.visibility != 0)
        gApp.SetGameHostOption(eGameHostOption_FriendsOfFriends, g_request.visibility == 2 ? 1 : 0);
    if (PcIsDedicatedServer())
        gApp.SetGameHostOption(eGameHostOption_CheatsEnabled, 1);
    CGameNetworkManager::sInstance.HostGame(0, g_request.host, g_request.visibility != 0,
                                            (unsigned char)gameType, eMiniGameId_Any, 0, 0);
    initData->m_hostOptions = gApp.GetGameHostOption(eGameHostOption_All);
    initData->m_miniGameHostOptions =
        MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true)->GetHostOption(eCustomGameModeHostOption_All);
    ShowLoadingScene(initData);
}

int SavesListed(void*, SAVE_DETAILS* info, bool)
{
    g_saveIndex = -1;
    for (int i = 0; info != 0 && i < info->count; ++i)
    {
        if (wcscmp(info->saves[i].title, g_request.name.c_str()) == 0)
        {
            g_saveIndex = i;
            break;
        }
    }
    g_callbackResult = eCallback_Succeeded;
    return 0;
}

int ThumbnailLoaded(void*, unsigned char* data, unsigned int len)
{
    if (data != 0 && len != 0)
        g_thumbnail.assign(data, data + len);
    else
        g_thumbnail.clear();
    g_callbackResult = eCallback_Succeeded;
    return 0;
}

int SaveLoaded(void*, bool corrupt, bool complete)
{
    g_callbackResult = complete && !corrupt ? eCallback_Succeeded : eCallback_Failed;
    return 0;
}

void RestoreHostOptionsFromThumbnail()
{
    CMinecraftApp::PngTagData tagData;
    memset(&tagData, 0, sizeof(tagData));
    if (!g_thumbnail.empty())
        gApp.GetImageTextData(&g_thumbnail[0], (unsigned int)g_thumbnail.size(), &tagData);
    g_savedTexturePack = (tagData.flags & CMinecraftApp::PngTagData::eFlag_TexturePack) != 0 ? tagData.texturePack : 0;
    tagData.flags |= CMinecraftApp::PngTagData::eFlag_NumLoads;
    ++tagData.numLoads;
    gApp.SetCurrentImageTextData(&tagData);
    if ((tagData.flags & CMinecraftApp::PngTagData::eFlag_HostOptions) == 0)
    {
        printf("[pc] world: the save has no host options; using the defaults\n");
        fflush(stdout);
        SetDefaultHostOptions(gProfile->GetPrimaryPad(), GameType::SURVIVAL->getId());
    }
}

void SessionsUpdated(void*)
{
}

void AddDirectHostFromRequest()
{
    std::string text(g_request.joinHost.begin(), g_request.joinHost.end());
    unsigned short port = kPcLanGamePort;
    std::string::size_type colon = text.find(':');
    if (colon != std::string::npos)
    {
        port = static_cast<unsigned short>(atoi(text.c_str() + colon + 1));
        text.erase(colon);
    }
    unsigned int address = 0;
    if (text.empty() || !PcOsResolveIPv4(text.c_str(), &address))
        return;
    char formatted[32];
    PcOsFormatIPv4(address, formatted, sizeof(formatted));
    g_request.joinHost = convStringToWstring(std::string(formatted));
    g_request.joinSessionId = PcLanDiscovery::ManualSessionId(address, port);
    g_request.joinAddress = address;
    g_request.joinPort = port;
    PcNetworkSession::AddDirectHost(address, port);
}

bool JoinFirstMatchingSession()
{
    const int pad = gProfile->GetPrimaryPad();
    std::vector<FriendSessionInfo*>* sessions = CGameNetworkManager::sInstance.GetSessionList(pad, 0, false);
    FriendSessionInfo* chosen = 0;
    for (unsigned int index = 0; sessions != 0 && index < sessions->size(); ++index)
    {
        FriendSessionInfo* info = (*sessions)[index];
        if (chosen != 0)
            continue;
        std::wstring hostName = convStringToWstring(std::string(info->m_gameSessionData.m_hostGamertag.m_name));
        if (g_request.joinSessionId != 0 ? info->m_sessionId == g_request.joinSessionId
                                         : (g_request.joinHost.empty() || hostName == g_request.joinHost))
            chosen = info;
    }
    if (chosen == 0)
        return false;
    printf("[pc] world: joining the LAN game hosted by \"%s\" (listed as \"%ls\")\n",
           chosen->m_gameSessionData.m_hostGamertag.m_name, chosen->m_displayName != 0 ? chosen->m_displayName : L"");
    LockPrimaryPad();
    gProfile->SetLockedProfile(0);
    CGameNetworkManager::sInstance.m_joiningMiniGameId =
        static_cast<EMiniGameId>(chosen->m_gameSessionData.m_miniGameId);
    if (chosen->m_gameSessionData.m_miniGameId != eMiniGameId_Any)
    {
        gApp.SetGameHostOption(eGameHostOption_MiniGame, 1);
        gApp.SetFirstMiniGameType(static_cast<EMiniGameId>(chosen->m_gameSessionData.m_miniGameId));
    }
    CGameNetworkManager::sInstance.JoinGame(chosen, CGameNetworkManager::GetLocalPlayerMask(pad), false);
    fflush(stdout);
    return true;
}

void ReportWorld(Minecraft* mc)
{
    Level* lvl = mc->mLevel;
    if (lvl == 0 || mc->mPlayer == 0)
    {
        printf("[pc] world: t=%llums no level yet\n", MillisecondsSinceStart());
        return;
    }

    printf("[pc] world: t=%llums seed=%lld time=%lld player=(%.1f, %.1f, %.1f) players=%d\n",
           MillisecondsSinceStart(), lvl->getSeed(), lvl->getGameTime(), mc->mPlayer->mX,
           mc->mPlayer->mY, mc->mPlayer->mZ, CGameNetworkManager::sInstance.GetPlayerCount());
}

void Roam()
{
    if (g_request.roamStep <= 0 || gMinecraftServer == 0 || gMinecraftServer->getPlayers() == 0)
        return;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
    ServerLevel* overworld = gMinecraftServer->getLevel(0);
    if (players->empty() || overworld == 0)
        return;
    boost::shared_ptr<ServerPlayer> p = players->at(0);
    const double edge = overworld->getLevelData()->getXZSize() * 8.0 - 64.0;
    if (p->mX + g_request.roamStep < edge)
    {
        p->mAbilities.mMayfly = true;
        p->mAbilities.mFlying = true;
        p->mAbilities.mInvulnerable = true;
        p->onUpdateAbilities();
        p->teleportTo(p->mX + g_request.roamStep, 150.0, p->mZ);
    }
    printf("[pc] roam: x=%.0f loaded overworld chunks=%d\n", p->mX,
           static_cast<ServerChunkCache*>(overworld->getChunkSource())->getLoadedChunks());
}

bool TakeCallbackResult(CallbackResult* res)
{
    if (g_callbackResult == eCallback_Pending)
        return false;
    *res = g_callbackResult;
    g_callbackResult = eCallback_Pending;
    return true;
}

void LeaveIfScheduled()
{
    if (g_leaveAtSeconds < 0.0 || gApp.GetGameStarted() == 0
        || (double)(PcOsMonotonicNanoseconds() - g_processStart) / 1e9 < g_leaveAtSeconds)
        return;
    g_leaveAtSeconds = -1.0;
    const int pad = gProfile->GetPrimaryPad();
    printf("[pc] world: leaving the game (--leave)\n");
    fflush(stdout);
    if (gMinecraftServer != 0)
    {
        gMinecraftServer->saveOnExit = true;
        MinecraftServer::s_saveOnExitAnswered = true;
    }
    gApp.SetAction(pad, eXuiAction_ExitWorld, NULL);
}

void WatchExit()
{
    const bool playing = gApp.GetGameStarted() != 0;
    const unsigned long long now = PcOsMonotonicNanoseconds();
    if (playing)
    {
        g_wasPlaying = true;
        g_exiting = false;
        return;
    }
    if (!g_wasPlaying)
        return;
    if (!g_exiting && gApp.GetXuiServerAction(gProfile->GetPrimaryPad()) != eXuiServerAction_Idle)
        return;
    if (!g_exiting)
    {
        g_exiting = true;
        g_exitStart = now;
        g_exitReport = now;
        printf("[pc] exit: leaving the game\n");
        fflush(stdout);
    }
    if (gConsoleUIController.FindScene(EUIScene_FullscreenProgress) == 0
        && gConsoleUIController.FindScene(EUIScene_MainMenu) != 0)
    {
        printf("[pc] exit: done after %llums\n", (now - g_exitStart) / NANOSECONDS_PER_MILLISECOND);
        fflush(stdout);
        g_wasPlaying = false;
        g_exiting = false;
        return;
    }
    if (now - g_exitStart < EXIT_REPORT_AFTER_NANOSECONDS || now - g_exitReport < EXIT_REPORT_INTERVAL_NANOSECONDS)
        return;
    g_exitReport = now;
    char session[128];
    PcNetworkSession::DescribeState(session, sizeof(session));
    printf("[pc] exit: still waiting after %llums: network thread %d, in session %d, leaving %d, server %d, "
           "halt %d; %s\n",
           (now - g_exitStart) / NANOSECONDS_PER_MILLISECOND,
           CGameNetworkManager::sInstance.IsNetworkThreadRunning() ? 1 : 0,
           CGameNetworkManager::sInstance.IsInSession() ? 1 : 0,
           CGameNetworkManager::sInstance.IsLeavingGame() ? 1 : 0, gMinecraftServer != 0 ? 1 : 0,
           MinecraftServer::s_haltRequested ? 1 : 0, session);
    fflush(stdout);
}

void Fail(const char* s)
{
    printf("[pc] world: %s failed; staying on the main menu\n", s);
    fflush(stdout);
    g_state = eLaunch_Done;
    if (g_request.interactive)
    {
        static wchar_t s_message[256];
        std::wstring host = g_request.joinHost;
        for (std::wstring::size_type i = 0; i < host.size(); ++i)
        {
            if (host[i] == L'%')
                host[i] = L' ';
        }
        swprintf(s_message, sizeof(s_message) / sizeof(s_message[0]),
                 L"No game answered at %ls. Check the address, that the host has loaded a world with Online Game "
                 L"on, and that port 25565 is reachable.",
                 host.c_str());
        int parameters[1] = {(int)IDS_CONFIRM_OK};
        gConsoleUIController.RequestErrorMessage(IDS_ONLINE_GAME, IDS_ONLINE_GAME, parameters, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, s_message, true);
    }
}
}

void PcRequestWorld(const std::string& name)
{
    g_request.name = convStringToWstring(name);
    g_state = eLaunch_WaitForMenu;
}

void PcRequestWorldVisibility(int visibility)
{
    g_request.visibility = visibility;
}

void PcRequestWorldCreative()
{
    g_request.creative = true;
}

void PcRequestWorldSeed(long long seed)
{
    g_request.hasSeed = true;
    g_request.seed = seed;
}

void PcRequestResetNether()
{
    g_request.resetNether = true;
}

void PcRequestResetEnd()
{
    g_request.resetEnd = true;
}

void PcRequestWorldRoam(int n)
{
    g_request.roamStep = n;
}

bool PcRequestWorldSize(const std::string& size)
{
    static const char* const kNames[] = {"classic", "small", "medium", "large", "massive"};
    for (int i = 0; i < 5; ++i)
    {
        if (size == kNames[i])
        {
            g_request.worldSize = i + 1;
            return true;
        }
    }
    return false;
}

void PcRequestHost()
{
    g_request.host = true;
}

void PcRequestJoin(const std::string& hostName)
{
    g_request.join = true;
    g_request.joinHost = convStringToWstring(hostName);
    g_state = eLaunch_WaitForMenu;
}

void PcRequestLeaveAt(double secs)
{
    g_processStart = PcOsMonotonicNanoseconds();
    g_leaveAtSeconds = secs;
}

void PcRequestQuitOnDisconnect()
{
    g_request.quitOnDisconnect = true;
}

bool PcWorldLauncherQuitRequested()
{
    return g_quitRequested;
}

void PcRequestInteractiveJoin(const std::string& address)
{
    g_request.join = true;
    g_request.host = false;
    g_request.interactive = true;
    g_request.quitOnDisconnect = false;
    g_request.joinHost = convStringToWstring(address);
    g_request.joinSessionId = 0;
    g_request.joinAddress = 0;
    g_request.joinPort = 0;
    g_menuFrames = 0;
    g_state = eLaunch_WaitForMenu;
}

void PcAcceptEula()
{
    g_acceptEula = true;
}

bool PcWorldRequested()
{
    return g_state != eLaunch_Idle;
}

void PcWorldLauncherTick(Minecraft* mc)
{
    PcDedicatedTick();
    WatchExit();
    LeaveIfScheduled();
    if (g_state > eLaunch_WaitForMenu && g_state < eLaunch_InGame)
        LockPrimaryPad();
    CallbackResult res;
    switch (g_state)
    {
    case eLaunch_Idle:
    case eLaunch_Done:
        return;
    case eLaunch_WaitForMenu:
        if (gConsoleUIController.FindScene(EUIScene_MainMenu) == 0)
        {
            g_menuFrames = 0;
            if (!ConfirmBootScene())
                g_state = eLaunch_Done;
            return;
        }
        if (++g_menuFrames < SCENE_SETTLE_FRAMES)
            return;
        g_startTime = PcOsMonotonicNanoseconds();
        PrepareToPlay(mc);
        if (g_request.join)
        {
            printf("[pc] world: looking for a LAN game\n");
            fflush(stdout);
            AddDirectHostFromRequest();
            CGameNetworkManager::sInstance.SetSessionsUpdatedCallback(SessionsUpdated, 0);
            CGameNetworkManager::sInstance.ForceFriendsSessionRefresh();
            g_state = eLaunch_FindSession;
            return;
        }
        g_callbackResult = eCallback_Pending;
        gStorage->GetSavesInfo(gProfile->GetPrimaryPad(), SavesListed, 0, SAVE_LIST_FILTER);
        g_state = eLaunch_ListSaves;
        return;
    case eLaunch_FindSession:
        if (g_request.joinPort != 0)
        {
            PcNetworkSession::AddDirectHost(g_request.joinAddress, g_request.joinPort);
            const unsigned int sessionId =
                PcNetworkSession::DirectHostSessionId(g_request.joinAddress, g_request.joinPort);
            if (sessionId != 0)
                g_request.joinSessionId = sessionId;
        }
        if (JoinFirstMatchingSession())
        {
            CGameNetworkManager::sInstance.SetSessionsUpdatedCallback(0, 0);
            g_state = eLaunch_Starting;
            return;
        }
        if (PcOsMonotonicNanoseconds() - g_startTime
            > (g_request.interactive ? INTERACTIVE_JOIN_TIMEOUT_NANOSECONDS : JOIN_SEARCH_TIMEOUT_NANOSECONDS))
        {
            CGameNetworkManager::sInstance.SetSessionsUpdatedCallback(0, 0);
            Fail("finding a LAN game");
        }
        return;
    case eLaunch_ListSaves:
        if (!TakeCallbackResult(&res))
            return;
        if (g_saveIndex < 0)
        {
            printf("[pc] world: creating \"%ls\" on pad %d\n", g_request.name.c_str(), gProfile->GetPrimaryPad());
            fflush(stdout);
            HostNewWorld(mc);
            g_state = eLaunch_Starting;
            return;
        }
        printf("[pc] world: loading \"%ls\" (save %d) on pad %d\n", g_request.name.c_str(), g_saveIndex,
               gProfile->GetPrimaryPad());
        fflush(stdout);
        gStorage->LoadSaveDataThumbnail(&gStorage->ReturnSavesInfo()->saves[g_saveIndex], ThumbnailLoaded, 0);
        g_state = eLaunch_LoadThumbnail;
        return;
    case eLaunch_LoadThumbnail:
        if (!TakeCallbackResult(&res))
            return;
        RestoreHostOptionsFromThumbnail();
        if (gStorage->LoadSaveData(&gStorage->ReturnSavesInfo()->saves[g_saveIndex], SaveLoaded, 0)
            == C4JStorage::ESaveGame_LoadFailed)
        {
            Fail("loading the save");
            return;
        }
        g_state = eLaunch_LoadSave;
        return;
    case eLaunch_LoadSave:
        if (!TakeCallbackResult(&res))
            return;
        if (res != eCallback_Succeeded)
        {
            Fail("loading the save");
            return;
        }
        HostSavedWorld(mc);
        g_state = eLaunch_Starting;
        return;
    case eLaunch_Starting:
        if (gApp.GetGameStarted() == 0)
            return;
        printf("[pc] world: game started after %llums\n", MillisecondsSinceStart());
        g_lastReport = PcOsMonotonicNanoseconds();
        ReportWorld(mc);
        fflush(stdout);
        g_state = eLaunch_InGame;
        return;
    case eLaunch_InGame:
        if (g_request.quitOnDisconnect && !g_quitRequested && gApp.GetGameStarted() == 0
            && gConsoleUIController.FindScene(EUIScene_MainMenu) != 0)
        {
            printf("[pc] world: left the game; quitting (-quitondisconnect)\n");
            fflush(stdout);
            g_quitRequested = true;
            return;
        }
    case eLaunch_Saving:
        if (PcOsMonotonicNanoseconds() - g_lastReport < REPORT_INTERVAL_NANOSECONDS)
            return;
        g_lastReport = PcOsMonotonicNanoseconds();
        ReportWorld(mc);
        if (g_state == eLaunch_InGame)
            Roam();
        fflush(stdout);
        return;
    }
}

bool PcWorldLauncherFinish()
{
    const int pad = gProfile->GetPrimaryPad();
    if (g_state == eLaunch_InGame && !CGameNetworkManager::sInstance.IsHost())
    {
        g_state = eLaunch_Done;
        return true;
    }
    if (g_state == eLaunch_InGame)
    {
        printf("[pc] world: saving before exit\n");
        fflush(stdout);
        g_saveStart = PcOsMonotonicNanoseconds();
        gApp.SetAction(pad, eXuiAction_AutoSave, 0);
        g_state = eLaunch_Saving;
        return false;
    }
    if (g_state != eLaunch_Saving)
        return true;

    const bool busy = gApp.GetXuiAction(pad) != eXuiAction_Idle
                      || gConsoleUIController.FindScene(EUIScene_FullscreenProgress) != 0
                      || gStorage->GetSaveState() == C4JStorage::ESaveGame_Save;
    const unsigned long long t = PcOsMonotonicNanoseconds() - g_saveStart;
    if (busy && t < SAVE_TIMEOUT_NANOSECONDS)
        return false;

    if (busy)
        printf("[pc] world: save did not finish within %llums\n", t / NANOSECONDS_PER_MILLISECOND);
    else
        printf("[pc] world: saved in %llums\n", t / NANOSECONDS_PER_MILLISECOND);
    fflush(stdout);
    g_state = eLaunch_Done;
    return true;
}
