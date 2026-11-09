#include "game/network/CGameNetworkManager.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcDedicated.hpp"
#include "platform/pc/PcTutorials.hpp"
#endif
#include "game/core/StringIds.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/Socket.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/Connection.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Gui.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/core/GameSessionData.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/MasterGameMode.hpp"
#include "platform/wiiu/CInput.hpp"
#include "vendor/_ConnectionProgressParams.hpp"
#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"
#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"
#include "platform/wiiu/cCommerceNintendo.hpp"
#include "platform/wiiu/C4JThread.hpp"
#include "game/network/ServerboundPreLoginPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/ui/UIScene_PauseMenu.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/core/Compression.hpp"
#include "game/core/IntCache.hpp"
#include "game/world/chunk/OldChunkStorage.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/block/Block.hpp"
#include "game/item/FireworksRecipe.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/core/GameRuleManager.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/util/Random.hpp"
#include "game/core/System.hpp"
#include "game/core/File.hpp"
#include "game/core/StringConversion.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/data/SparseLightStorage.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"
#include "vendor/_NetworkGameInitData.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "game/ui/UIScene_QuadrantSignin.hpp"
#include "game/core/User.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "platform/wiiu/CPlatformNetworkManagerNintendo.hpp"
#if defined(RECON_NATIVE)
#include <cerrno>
#include <cstring>
#include <nn/fp/fp_cpp.h>
#include <cstdio>
#include <cstdlib>
#endif

#include <sysapp/args.h>

extern "C" int shutdown(int socket, int how);
extern ConsoleUIController gConsoleUIController;
extern "C" int close(int socket);
void SetThreadName(unsigned int threadId, const char* name);
void closeSocketForLockedProfile();

CPlatformNetworkManager* g_pPlatformNetworkManager;
CGameNetworkManager CGameNetworkManager::sInstance;

CGameNetworkManager::CGameNetworkManager()
    : m_initialised(false),
      m_pendingInviteUserIndex(0),
      m_pendingInviteUserMask(0),
      m_pendingInvitePresence(0),
      m_deferredInviteUserIndex(-1),
      m_disconnectError(false),
      m_unknownState(false),
      m_publicJoinable(false),
      m_inviteInfo(0),
      m_connections(),
      m_joiningMiniGameId(eMiniGameId_Lobby)
{
    InitializeCriticalSection(&m_connectionsMutex);
}
CGameNetworkManager::~CGameNetworkManager()
{
    DeleteCriticalSection(&m_connectionsMutex);
}

INetworkPlayer* CGameNetworkManager::GetLocalPlayerByUserIndex(int userIndex)
{
    return g_pPlatformNetworkManager->GetLocalPlayerByUserIndex(userIndex);
}
void CGameNetworkManager::ServerStoppedCreate(bool create)
{
    m_serverStoppedEvent = create ? new C4JEventImpl(C4JEvent::Mode0) : 0;
}
void CGameNetworkManager::ServerReadyCreate(bool create)
{
    m_serverReadyEvent = create ? new C4JEventImpl(C4JEvent::Mode0) : 0;
}
void CGameNetworkManager::Initialise()
{
    ServerStoppedCreate(false);
    ServerReadyCreate(false);
    g_pPlatformNetworkManager = CPlatformNetworkManager::Create();
    g_pPlatformNetworkManager->Initialise(this, LevelRenderer::getGlobalChunkCount() / 16);
    m_networkThreadRunning = false;
    m_initialised = true;
}
void CGameNetworkManager::Terminate()
{
    if (m_initialised)
        g_pPlatformNetworkManager->Terminate();
}
int CGameNetworkManager::GetLocalPlayerMask(int userIndex)
{
    return g_pPlatformNetworkManager ? g_pPlatformNetworkManager->GetLocalPlayerMask(userIndex) : 0;
}
bool CGameNetworkManager::IsInSession()
{
    return g_pPlatformNetworkManager->IsInSession();
}
bool CGameNetworkManager::JoinGameFromInviteInfo(int userIndex, int mask, const nn::fp::FriendPresence* presence)
{
    bool joining = g_pPlatformNetworkManager->JoinGameFromInviteInfo(userIndex, mask, presence);
    if (joining)
    {
        telemetryManagerInstance->PrepareMiniGameHostOptions(userIndex, NULL, eTelemetryJoinMethod_Invite);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(true);
    }
    return joining;
}
bool CGameNetworkManager::SetLocalGame(bool localGame)
{
    return g_pPlatformNetworkManager->SetLocalGame(localGame);
}
int CGameNetworkManager::JoinFromInvite_SignInReturned(void* data, bool result, int userIndex)
{
    if (result && gProfile->IsSignedIn(userIndex) && gProfile->IsSignedInLive(userIndex))
    {
        unsigned int mask = 0;
        bool notAllowed = false;
        if (gProfile->IsSignedIn(0))
        {
            if (!gProfile->AllowedToPlayMultiplayer(0))
                notAllowed = true;
            mask = GetLocalPlayerMask(0);
        }
        if (gProfile->IsSignedIn(1))
        {
            if (!gProfile->AllowedToPlayMultiplayer(1))
                notAllowed = true;
            mask |= GetLocalPlayerMask(1);
        }
        if (gProfile->IsSignedIn(2))
        {
            if (!gProfile->AllowedToPlayMultiplayer(2))
                notAllowed = true;
            mask |= GetLocalPlayerMask(2);
        }
        if (gProfile->IsSignedIn(3))
        {
            if (!gProfile->AllowedToPlayMultiplayer(3))
                notAllowed = true;
            mask |= GetLocalPlayerMask(3);
        }
        if (notAllowed)
        {
            int messageParams = IDS_CONFIRM_OK;
            gConsoleUIController.RequestErrorMessage(0x252ab13f, 0xcc5a590f, &messageParams, 1,
                                                     gProfile->GetPrimaryPad(), 0, 0, 0, true);
        }
        else
        {
            gProfile->SetLockedProfile(userIndex);
            gProfile->SetPrimaryPad(userIndex);
            sInstance.SetLocalGame(false);
            gProfile->QuerySigninStatus();
            Minecraft::GetInstance()->clearConnectionFailed();
            Minecraft::GetInstance()->mUser->name = convStringToWstring(
                gProfile->GetGamertag(gProfile->GetPrimaryPad()));
            sInstance.JoinGameFromInviteInfo(userIndex, mask, static_cast<const nn::fp::FriendPresence*>(data));
        }
    }
    return 0;
}
void CGameNetworkManager::HandleInviteWhenInMenus(int userIndex, const nn::fp::FriendPresence* presence)
{
    bool fullVersion = gProfile->IsFullVersion();
    gProfile->SetPrimaryPad(userIndex);
    if (!fullVersion)
    {
        gApp.SetAction(userIndex, eXuiAction_UpsellForInvite, 0);
        return;
    }
    gApp.StartInstallDLCProcess(userIndex);
    if (!gApp.IsLocalMultiplayerAvailable() || gApp.IsDrcPreventingSplitscreen())
    {
        if (!gProfile->AllowedToPlayMultiplayer(userIndex))
        {
            int messageParams = IDS_CONFIRM_OK;
            gConsoleUIController.RequestErrorMessage(0x252ab13f, 0xcc5a590f, &messageParams, 1,
                                                     gProfile->GetPrimaryPad(), 0, 0, 0, true);
        }
        else
        {
            gProfile->SetLockedProfile(userIndex);
            gProfile->SetPrimaryPad(userIndex);
            unsigned int mask = GetLocalPlayerMask(userIndex);
            gProfile->QuerySigninStatus();
            Minecraft::GetInstance()->clearConnectionFailed();
            sInstance.SetLocalGame(false);
            Minecraft::GetInstance()->mUser->name = convStringToWstring(
                gProfile->GetGamertag(gProfile->GetPrimaryPad()));
            sInstance.JoinGameFromInviteInfo(userIndex, mask, presence);
        }
    }
    else
    {
        UIScene_QuadrantSignin::SignInInfo signInInfo;
        signInInfo.callback = &CGameNetworkManager::JoinFromInvite_SignInReturned;
        signInInfo.context = const_cast<nn::fp::FriendPresence*>(presence);
        signInInfo.onlineGame = true;
        signInInfo.returnToHome = false;
        gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_QuadrantSignin, &signInInfo,
                                             EUILayer_Fullscreen, EUIGroup_Fullscreen);
    }
}
void CGameNetworkManager::ProcessInvite(unsigned int userIndex, unsigned int flags, nn::fp::FriendPresence* presence)
{
    m_pendingInvitePresence = presence;
    m_pendingInviteUserIndex = (int)userIndex;
    m_pendingInviteUserMask = flags;
    gApp.SetAction((int)userIndex, eXuiAction_JoinFromInvite, 0);
}
void CGameNetworkManager::GameInviteReceived(int userIndex, nn::fp::FriendPresence* presence)
{
    if (gConsoleUIController.IsShowingErrorMessage(userIndex))
    {
        if (presence != NULL)
        {
            m_deferredInviteUserIndex = userIndex;
            memcpy(m_deferredInvitePresence, presence, sizeof(nn::fp::FriendPresence));
        }
        return;
    }

    unsigned int mask = 0;
    m_inviteInfo = presence;
    Minecraft* minecraft = Minecraft::GetInstance();
    bool notAllowed = false;
    if (gProfile->IsSignedIn(0) && (userIndex == 0 || minecraft->mLocalPlayers[0] != NULL))
    {
        if (!gProfile->AllowedToPlayMultiplayer(0))
            notAllowed = true;
        mask = GetLocalPlayerMask(0);
    }
    if (gProfile->IsSignedIn(1) && (userIndex == 1 || minecraft->mLocalPlayers[1] != NULL))
    {
        if (!gProfile->AllowedToPlayMultiplayer(1))
            notAllowed = true;
        mask |= GetLocalPlayerMask(1);
    }
    if (gProfile->IsSignedIn(2) && (userIndex == 2 || minecraft->mLocalPlayers[2] != NULL))
    {
        if (!gProfile->AllowedToPlayMultiplayer(2))
            notAllowed = true;
        mask |= GetLocalPlayerMask(2);
    }
    if (gProfile->IsSignedIn(3) && (userIndex == 3 || minecraft->mLocalPlayers[3] != NULL))
    {
        if (!gProfile->AllowedToPlayMultiplayer(3))
            notAllowed = true;
        mask |= GetLocalPlayerMask(3);
    }
    if (notAllowed)
    {
        int messageParams = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(0x252ab13f, 0xcc5a590f, &messageParams, 1, 0xff, 0, 0, 0, true);
        return;
    }
    if (!sInstance.IsInSession())
    {
        HandleInviteWhenInMenus(userIndex, presence);
        return;
    }
    if (g_pPlatformNetworkManager->checkValidInviteData(presence))
    {
        ProcessInvite(userIndex, mask, presence);
    }
}
void CGameNetworkManager::TickInvitePending()
{
    if (m_deferredInviteUserIndex >= 0 && !gConsoleUIController.IsShowingErrorMessage(m_deferredInviteUserIndex))
    {
        GameInviteReceived(m_deferredInviteUserIndex,
                           reinterpret_cast<nn::fp::FriendPresence*>(m_deferredInvitePresence));
        m_deferredInviteUserIndex = -1;
    }
}
void CGameNetworkManager::DoWork()
{
    if (g_pPlatformNetworkManager)
        g_pPlatformNetworkManager->DoWork();
    TickInvitePending();
}
bool CGameNetworkManager::IsHost()
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->IsHost();
}
int CGameNetworkManager::ServerThreadProc(void* data)
{
    _NetworkGameInitData* initData = static_cast<_NetworkGameInitData*>(data);
    long long seed = 0;
    if (initData != 0)
    {
        seed = initData->m_seed;
        gApp.SetGameHostOption(eGameHostOption_All, initData->m_hostOptions);
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        miniGame->SetHostOption(eCustomGameModeHostOption_All, initData->m_miniGameHostOptions);
        if (initData->m_levelGenerationOptions != 0
            && initData->m_levelGenerationOptions->getParentPackId() != initData->m_texturePackId)
        {
            while (Minecraft::GetInstance()->mTexturePackRepository->needsUIUpdate()
                   || gConsoleUIController.IsReloadingSkin())
                C4JThread::Sleep(1);
            initData->m_levelGenerationOptions->loadBaseSaveData();
        }
    }
    SetThreadName(0xffffffff, "Minecraft Server thread");
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    phys::Vec2::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    Compression::UseDefaultThreadStorage();
    OldChunkStorage::UseDefaultThreadStorage();
    Entity::useSmallIds();
    Level::enableLightingCache();
    Block::CreateNewThreadStorage();
    FireworksRecipe::CreateNewThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_Server);
    LevelChunk::CreateNewThreadStorage();
    MinecraftServer::main(seed, initData);
    AABB::ReleaseThreadStorage();
    Vec3::ReleaseThreadStorage();
    phys::Vec2::ReleaseThreadStorage();
    IntCache::ReleaseThreadStorage();
    Level::destroyLightingCache();
    Block::ReleaseThreadStorage();
    FireworksRecipe::ReleaseThreadStorage();
    ElementStorage::ReleaseThreadStorage();
    LevelChunk::ReleaseThreadStorage();
    if (data != 0)
        free(data);
    return 0;
}
void CGameNetworkManager::ServerReadyWait()
{
    if (m_serverReadyEvent)
        m_serverReadyEvent->WaitForSignal(-1);
}
void CGameNetworkManager::ServerReadyDestroy()
{
    delete m_serverReadyEvent;
    m_serverReadyEvent = 0;
}
bool CGameNetworkManager::IsReadyToPlayOrIdle()
{
    return g_pPlatformNetworkManager->IsReadyToPlayOrIdle();
}
int CGameNetworkManager::CorrectErrorIDS(int ids)
{
    return g_pPlatformNetworkManager->CorrectErrorIDS(ids);
}
int CGameNetworkManager::GetJoiningReadyPercentage()
{
    return g_pPlatformNetworkManager->GetJoiningReadyPercentage();
}
bool CGameNetworkManager::IsLeavingGame()
{
    return g_pPlatformNetworkManager->IsLeavingGame();
}
int CGameNetworkManager::GetPlayerCount()
{
    return g_pPlatformNetworkManager->GetPlayerCount();
}
bool CGameNetworkManager::IsLocalGame()
{
    return g_pPlatformNetworkManager->IsLocalGame();
}
bool CGameNetworkManager::StartNetworkGame(Minecraft* minecraft, void* data)
{
    _NetworkGameInitData* initData = static_cast<_NetworkGameInitData*>(data);
    if (initData != 0)
    {
        gApp.setLevelGenerationOptions(initData->m_levelGenerationOptions);
        if (initData->m_levelGenerationOptions != 0)
        {
            if (gApp.getLevelGenerationOptions() == 0)
            {
                closeSocketForLockedProfile();
                return false;
            }
            if (!gApp.getLevelGenerationOptions()->hasLevelSeed())
            {
                Random random(System::processTimeInNanoSecs());
                initData->m_seed = random.nextLong();
#if defined(RECON_NATIVE)
                const char* fixture = std::getenv("MC_WORLD_SEED");
                if (fixture != NULL && *fixture != '\0')
                {
                    char* end = NULL;
                    errno = 0;
                    const long long fixtureSeed = std::strtoll(fixture, &end, 10);
                    if (errno == 0 && end != fixture && *end == '\0')
                        initData->m_seed = fixtureSeed;
                    else
                        std::fprintf(stderr, "[native-world] invalid MC_WORLD_SEED: %s\n", fixture);
                }
#endif
            }
            else
            {
                initData->m_seed = gApp.getLevelGenerationOptions()->getLevelSeed();
            }
            if (initData->m_levelGenerationOptions->isLocal())
            {
                bool hasBaseSave = false;
                if (initData->m_levelGenerationOptions->requiresBaseSave()
                    && !initData->m_levelGenerationOptions->getBaseSavePath().empty())
                {
                    hasBaseSave = true;
                }
                if (hasBaseSave)
                {
                    const unsigned int texturePackId = initData->m_levelGenerationOptions->getTexturePackId();
                    if (!initData->m_levelGenerationOptions->isTutorial())
                    {
                        if (texturePackId == 0)
                        {
                            std::wstring root = initData->m_levelGenerationOptions->getBaseSaveRoot();
                            std::wstring path = initData->m_levelGenerationOptions->getBaseSavePath();
                            std::wstring combined = root;
                            combined.append(path);
                            initData->m_levelGenerationOptions->setBaseSaveData(File(combined));
                        }
                        else
                        {
                            std::wstring path = initData->m_levelGenerationOptions->getBaseSavePath();
                            std::wstring root = initData->m_levelGenerationOptions->getBaseSaveRoot();
                            std::wstring filePath = gApp.getFilePath(
                                initData->m_levelGenerationOptions->getTexturePackId(), path, true, root);
                            initData->m_levelGenerationOptions->setBaseSaveData(File(filePath));
                        }
                    }
                    else
                    {
                        std::wstring path = initData->m_levelGenerationOptions->getBaseSavePath();
                        std::wstring root = initData->m_levelGenerationOptions->getBaseSaveRoot();
                        std::wstring filePath = gApp.getFilePath(4, path, true, root);
#ifdef MINECRAFT_PC
                        const std::wstring tutorialPath = PcTutorialBaseSavePath(initData->m_levelGenerationOptions);
                        if (!tutorialPath.empty())
                            filePath = tutorialPath;
#endif
                        initData->m_levelGenerationOptions->setBaseSaveData(File(filePath));
                    }
                }
            }
        }
    }

    ServerStoppedCreate(false);
    if (!sInstance.IsHost())
    {
        Socket::Initialise(0);
    }
    else
    {
        ServerStoppedCreate(true);
        ServerReadyCreate(true);
        C4JThreadImpl* serverThread = new C4JThreadImpl(ServerThreadProc, initData, "Server", 0x100000);
        serverThread->SetProcessor(0);
        serverThread->Run();
        serverThread->SetDeleteOnExit(true);
        ServerReadyWait();
        ServerReadyDestroy();
        if (MinecraftServer::s_haltRequested)
        {
            closeSocketForLockedProfile();
            return false;
        }
    }

    Minecraft* pMinecraft = Minecraft::GetInstance();
    if (!IsReadyToPlayOrIdle())
    {
        do
        {
            pMinecraft->mProgressRenderer->progressStage(sInstance.CorrectErrorIDS(0xaebf0574));
            pMinecraft->mProgressRenderer->progressStagePercentage(sInstance.GetJoiningReadyPercentage());
            Sleep(10);
        } while (!IsReadyToPlayOrIdle());
        pMinecraft->mProgressRenderer->progressStagePercentage(100);
    }

    if (!IsInSession())
    {
        MinecraftServer::HaltServer(false);
        closeSocketForLockedProfile();
        return false;
    }

    while (!gApp.DLCInstallProcessCompleted() && gApp.DLCInstallPending() && !sInstance.IsLeavingGame())
    {
        Sleep(10);
    }
    if (sInstance.IsLeavingGame())
    {
        MinecraftServer::HaltServer(false);
        closeSocketForLockedProfile();
        return false;
    }

    EnterCriticalSection(&m_connectionsMutex);
    m_connections.clear();
    LeaveCriticalSection(&m_connectionsMutex);

    ClientPacketListener* primaryListener = 0;
    if (!sInstance.IsHost())
    {
        INetworkPlayer* localPlayer = sInstance.GetLocalPlayerByUserIndex(gProfile->GetLockedProfile());
        Socket* socket = localPlayer != 0 ? localPlayer->GetSocket() : 0;
        if (socket == 0)
        {
            MinecraftServer::HaltServer(false);
            return false;
        }
        primaryListener = new ClientPacketListener(minecraft, socket, -1);
    }
    else
    {
        primaryListener = new ClientPacketListener(minecraft, 0, -1);
    }
    if (!primaryListener->m_createdOk)
    {
        delete primaryListener;
        MinecraftServer::HaltServer(false);
        return false;
    }

    const int primaryPad = gProfile->GetPrimaryPad();
    std::wstring onlineName = convStringToWstring(gProfile->GetGamertag(primaryPad));
    boost::shared_ptr<ServerboundPreLoginPacket> preLoginPacket(new ServerboundPreLoginPacket(onlineName));
    primaryListener->send(preLoginPacket);

    TexturePack* selectedTexturePack = 0;
    if (!sInstance.IsHost())
    {
        Minecraft::GetInstance()->mProgressRenderer->progressStart(IDS_PROGRESS_CONNECTING);
        selectedTexturePack = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    }
    else
    {
        telemetryManagerInstance->SetMultiplayerInstanceId(telemetryManagerInstance->GenerateMultiplayerInstanceId());
        selectedTexturePack = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    }

    while (true)
    {
        primaryListener->tick();
        CompressedBlockStorage::tick();
        SparseLightStorage::tick();
        SparseDataStorage::tick();
        Sleep(50);
        if (IsInSession() && !primaryListener->isStarted() && !primaryListener->isClosed()
            && !sInstance.IsLeavingGame())
        {
            continue;
        }
        if (selectedTexturePack->isLoadingData())
            continue;

        if (!Minecraft::GetInstance()->mTexturePackRepository->needsUIUpdate()
            && !gConsoleUIController.IsReloadingSkin())
        {
            break;
        }
    }
    gConsoleUIController.CleanUpSkinReload();

    if (sInstance.IsLeavingGame() || !IsInSession())
    {
        primaryListener->close();
    }

    if (!primaryListener->isStarted() || primaryListener->isClosed())
    {
        if (primaryListener->isClosed() || !IsInSession())
        {
            MinecraftServer::HaltServer(false);
            return false;
        }
    }
    else
    {
        EnterCriticalSection(&m_connectionsMutex);
        m_connections.push_back(primaryListener);
        LeaveCriticalSection(&m_connectionsMutex);

        const int hostPad = gProfile->GetPrimaryPad();
        const int playerCount = GetPlayerCount();
        const bool localGame = IsLocalGame();
        if (playerCount < 2)
            gProfile->SetCurrentGameActivity(hostPad, localGame ? 5 : 4, false);
        else
            gProfile->SetCurrentGameActivity(hostPad, localGame ? 3 : 2, false);

        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            if (userIndex == gProfile->GetPrimaryPad())
            {
                continue;
            }
            if (GetLocalPlayerByUserIndex(userIndex) != 0 && !gProfile->IsSignedIn(userIndex))
            {
                Socket* socket = sInstance.GetLocalPlayerByUserIndex(userIndex)->GetSocket();
                if (socket != 0 && !socket->close(false))
                    socket->close(true);
                continue;
            }

            INetworkPlayer* player = sInstance.GetLocalPlayerByUserIndex(userIndex);
            Socket* socket = player != 0 ? player->GetSocket() : 0;
            if (player == 0 || socket == 0)
            {
                continue;
            }

            ClientPacketListener* listener = new ClientPacketListener(minecraft, socket, userIndex);
            if (!listener->m_createdOk)
            {
                continue;
            }

            minecraft->addPendingLocalConnection(userIndex, listener);
            Socket::addIncomingSocket(socket);
            std::wstring playerName = convStringToWstring(gProfile->GetGamertag(userIndex));
            boost::shared_ptr<ServerboundPreLoginPacket> playerPreLogin(new ServerboundPreLoginPacket(playerName));
            listener->send(playerPreLogin);

            EnterCriticalSection(&m_connectionsMutex);
            m_connections.push_back(listener);
            LeaveCriticalSection(&m_connectionsMutex);

            while (true)
            {
                EnterCriticalSection(&m_connectionsMutex);
                for (unsigned int i = 0; i < m_connections.size(); ++i)
                {
                    m_connections[i]->tick();
                }
                LeaveCriticalSection(&m_connectionsMutex);
                CompressedBlockStorage::tick();
                SparseLightStorage::tick();
                SparseDataStorage::tick();
                Sleep(50);
                if (!IsInSession() || listener->isStarted() || listener->isClosed() || sInstance.IsLeavingGame())
                {
                    break;
                }
            }

            if (sInstance.IsLeavingGame() || !IsInSession())
            {
                break;
            }

            if (!gProfile->IsSignedIn(userIndex) || listener->isClosed())
            {
                EnterCriticalSection(&m_connectionsMutex);
                listener->close();
                std::vector<ClientPacketListener*>::iterator it = std::find(m_connections.begin(), m_connections.end(),
                                                                            listener);
                if (it != m_connections.end())
                {
                    delete *it;
                    *it = NULL;
                    m_connections.erase(it);
                }
                LeaveCriticalSection(&m_connectionsMutex);
            }
            else
            {
                gApp.SetRichPresenceContext(userIndex, 0);
                gProfile->SetCurrentGameActivity(userIndex, IsLocalGame() ? 3 : 2, false);
            }
        }

        gApp.SetGameMode(static_cast<eGameMode>(1));
    }

    if (!sInstance.IsLeavingGame() && IsInSession())
    {
        return !MinecraftServer::s_haltRequested;
    }

    EnterCriticalSection(&m_connectionsMutex);
    for (unsigned int i = 0; i < m_connections.size(); ++i)
    {
        m_connections[i]->close();
    }
    MinecraftServer::HaltServer(false);
    LeaveCriticalSection(&m_connectionsMutex);
    return false;
}
bool CGameNetworkManager::_RunNetworkGame(void* data)
{
    const bool isHost = sInstance.IsHost();
    Minecraft* minecraft = Minecraft::GetInstance();
    bool result = StartNetworkGame(minecraft, data);
    if (!result)
    {
        return false;
    }

    if (isHost)
    {
        result = g_pPlatformNetworkManager->_RunNetworkGame();
        if (!result)
        {
            gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_ExitWorld, reinterpret_cast<void*>(1));
            return true;
        }
    }

    if (sInstance.IsLeavingGame())
    {
        return false;
    }

    gApp.SetGameStarted(true);
    if (!gProfile->IsFullVersion())
    {
        gConsoleUIController.SetTrialTimerLimitSecs(0x960u);
        gApp.SetTrialTimerStart();
    }
    return result;
}
int CGameNetworkManager::GetOnlinePlayerCount()
{
    return g_pPlatformNetworkManager->GetOnlinePlayerCount();
}
bool CGameNetworkManager::AddLocalPlayerByUserIndex(int index)
{
    return g_pPlatformNetworkManager->AddLocalPlayerByUserIndex(index);
}
bool CGameNetworkManager::RemoveLocalPlayerByUserIndex(int index)
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->RemoveLocalPlayerByUserIndex(index);
}
INetworkPlayer* CGameNetworkManager::GetPlayerByIndex(int index)
{
    return g_pPlatformNetworkManager->GetPlayerByIndex(index);
}
INetworkPlayer* CGameNetworkManager::GetPlayerByXuid(PlayerUID uid)
{
    return g_pPlatformNetworkManager ? g_pPlatformNetworkManager->GetPlayerByXuid(uid) : 0;
}
INetworkPlayer* CGameNetworkManager::GetPlayerBySmallId(unsigned char id)
{
    return g_pPlatformNetworkManager->GetPlayerBySmallId(id);
}
std::wstring CGameNetworkManager::GetDisplayNameByGamertag(std::wstring name)
{
    return g_pPlatformNetworkManager ? g_pPlatformNetworkManager->GetDisplayNameByGamertag(name) : name;
}
INetworkPlayer* CGameNetworkManager::GetHostPlayer()
{
    return g_pPlatformNetworkManager->GetHostPlayer();
}
void CGameNetworkManager::RegisterPlayerChangedCallback(int i, void (*cb)(void*, INetworkPlayer*, bool), void* d)
{
    if (g_pPlatformNetworkManager)
    {
        g_pPlatformNetworkManager->RegisterPlayerChangedCallback(i, cb, d);
    }
}
void CGameNetworkManager::UnRegisterPlayerChangedCallback(int i, void (*cb)(void*, INetworkPlayer*, bool), void* d)
{
    if (g_pPlatformNetworkManager)
    {
        g_pPlatformNetworkManager->UnRegisterPlayerChangedCallback(i, cb, d);
    }
}
void CGameNetworkManager::HandleSignInChange()
{
    g_pPlatformNetworkManager->HandleSignInChange();
}
bool CGameNetworkManager::ShouldMessageForFullSession()
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->ShouldMessageForFullSession();
}
bool CGameNetworkManager::IsInGameplay()
{
    return g_pPlatformNetworkManager->IsInGameplay();
}
void CGameNetworkManager::SetPrivateGame(bool value)
{
    g_pPlatformNetworkManager->SetPrivateGame(value);
}
bool CGameNetworkManager::IsPrivateGame()
{
    return g_pPlatformNetworkManager->IsPrivateGame();
}
void CGameNetworkManager::HostGame(int a, bool b, bool c, unsigned char d, EMiniGameId e, unsigned int f,
                                   unsigned int* g)
{
    Minecraft::GetInstance()->clearConnectionFailed();
    if (g_pPlatformNetworkManager)
        g_pPlatformNetworkManager->HostGame(a, b, c, d, e, f, g);
}
bool CGameNetworkManager::IsInStatsEnabledSession()
{
    return g_pPlatformNetworkManager->IsInStatsEnabledSession();
}
bool CGameNetworkManager::IsMatchmakingEnabled()
{
    return g_pPlatformNetworkManager->IsMatchmakingEnabled();
}
bool CGameNetworkManager::IsInPublicJoinableGame()
{
    return (m_publicJoinable || g_pPlatformNetworkManager->IsMatchmakingEnabled()) && !IsLocalGame();
}
void CGameNetworkManager::SetIsInPublicJoinableGame(bool value)
{
    m_publicJoinable = value;
}
bool CGameNetworkManager::IsInActiveCoopOrCompetitive(int)
{
    ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame)
        && (!mode->IsInActiveRoundState() || mode->IsRoundEnding() || mode->IsRoundStarting()
            || mode->IsRoundRestarting()))
    {
        return false;
    }
    return true;
}
bool CGameNetworkManager::SessionHasSpace(unsigned int n)
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->SessionHasSpace(n);
}
bool CGameNetworkManager::SessionHasMii(int n)
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->SessionHasMii(n);
}
std::vector<FriendSessionInfo*>* CGameNetworkManager::GetSessionList(int a, int b, bool c)
{
    return g_pPlatformNetworkManager->GetSessionList(a, b, c);
}
bool CGameNetworkManager::GetGameSessionInfo(int a, unsigned int b, FriendSessionInfo* c)
{
    return g_pPlatformNetworkManager->GetGameSessionInfo(a, b, c);
}
void CGameNetworkManager::SetSessionsUpdatedCallback(void (*cb)(void*), void* d)
{
    if (g_pPlatformNetworkManager)
        g_pPlatformNetworkManager->SetSessionsUpdatedCallback(cb, d);
}
void CGameNetworkManager::GetFullFriendSessionInfo(FriendSessionInfo* a, void (*cb)(bool, void*), void* d)
{
    g_pPlatformNetworkManager->GetFullFriendSessionInfo(a, cb, d);
}
void CGameNetworkManager::ForceFriendsSessionRefresh()
{
    g_pPlatformNetworkManager->ForceFriendsSessionRefresh();
}
void CGameNetworkManager::ClearFriendsSessions()
{
    g_pPlatformNetworkManager->ClearFriendsSessions();
}
int CGameNetworkManager::JoinGame(FriendSessionInfo* info, int userIndex, bool matchmaking)
{
    gApp.SetTutorialMode(false, boost::shared_ptr<Player>());
    SetLocalGame(false);
    int lockedProfile = gProfile->GetLockedProfile();
    Minecraft::GetInstance()->clearConnectionFailed();
    unsigned int localMask = GetLocalPlayerMask(gProfile->GetPrimaryPad());
    if (g_pPlatformNetworkManager == 0)
    {
        return 0;
    }
    g_pPlatformNetworkManager->JoinGame(info, userIndex | localMask, lockedProfile, matchmaking);
    return 0;
}
int CGameNetworkManager::FindAndJoinGame(int userIndexMask, EMiniGameId miniGameId, unsigned int attributeCount,
                                         unsigned int* attributeValues, unsigned int* attributeIds)
{
    if (!gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
    {
        return 3;
    }
    gApp.SetTutorialMode(false, boost::shared_ptr<Player>());
    sInstance.SetLocalGame(false);
    int lockedProfile = gProfile->GetLockedProfile();
    Minecraft::GetInstance()->clearConnectionFailed();
    unsigned int localMask = GetLocalPlayerMask(gProfile->GetPrimaryPad());
    return g_pPlatformNetworkManager->FindAndJoinGame(userIndexMask | localMask, lockedProfile, miniGameId,
                                                      attributeCount, attributeValues, attributeIds);
}
void CGameNetworkManager::CancelJoinGame(void*)
{
    return;
}
bool CGameNetworkManager::LeaveGame(bool e)
{
    Minecraft::GetInstance()->mGui->clearMessages(-1);
    return g_pPlatformNetworkManager->LeaveGame(e);
}
void CGameNetworkManager::UpdateAndSetGameSessionData(INetworkPlayer* player)
{
    TexturePack* texturePack = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    g_pPlatformNetworkManager->SetSessionTexturePackParentId(texturePack->getDLCParentPackId());
    g_pPlatformNetworkManager->SetSessionSubTexturePackId(texturePack->getDLCSubPackId());
    g_pPlatformNetworkManager->UpdateAndSetGameSessionData(player);
}
void CGameNetworkManager::SendInviteGUI(int i)
{
    if (g_pPlatformNetworkManager)
        g_pPlatformNetworkManager->SendInviteGUI(i);
}
void CGameNetworkManager::ResetLeavingGame()
{
    g_pPlatformNetworkManager->ResetLeavingGame();
}
bool CGameNetworkManager::AlreadyInSession(const nn::fp::FriendPresence* p)
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->AlreadyInSession(p);
}
bool CGameNetworkManager::IsNetworkThreadRunning()
{
    return m_networkThreadRunning;
}
int CGameNetworkManager::RunNetworkGameThreadProc(void* data)
{
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    Block::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    FireworksRecipe::UseDefaultThreadStorage();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_NetworkGame);
    LevelChunk::CreateNewThreadStorage();

    sInstance.m_networkThreadRunning = true;
    const bool runResult = sInstance._RunNetworkGame(data);
    sInstance.m_networkThreadRunning = false;

    if (!runResult)
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        TexturePack* selected = minecraft->mTexturePackRepository->getSelected();
        while (true)
        {
            if (!selected->isLoadingData())
            {
                if (!minecraft->mTexturePackRepository->needsUIUpdate() && !gConsoleUIController.IsReloadingSkin())
                {
                    gConsoleUIController.CleanUpSkinReload();
                    if (gApp.GetDisconnectReason() == 0)
                    {
                        gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_Banned);
                    }
                    if (gMinecraftServer == 0)
                    {
                        gApp.mGameRuleManager.unloadCurrentGameRules();
                    }
                    Block::ReleaseThreadStorage();
                    return -1;
                }
            }
            C4JThread::Sleep(1);
        }
    }

    while (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        ClientMasterGameMode* mode = Minecraft::GetInstance()->GetClientMasterGameMode();
        if (!mode->IsInTrasitionFromLobby())
        {
            break;
        }
        C4JThread::Sleep(5);
    }

    Block::ReleaseThreadStorage();
    IntCache::ReleaseThreadStorage();
    ElementStorage::ReleaseThreadStorage();
    LevelChunk::ReleaseThreadStorage();
    return 0;
}
int CGameNetworkManager::ExitAndJoinFromInviteThreadProc(void* data)
{
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    UIScene_PauseMenu::_ExitWorld(NULL);
    while (sInstance.IsInSession())
        Sleep(1);

    Minecraft* minecraft = Minecraft::GetInstance();
    TexturePack* previousSelected = Minecraft::GetInstance()->mTexturePackRepository->getSelected();
    previousSelected->hasAudio();
    minecraft->mTexturePackRepository->selectTexturePackById(0, false);
    const bool hasAudio = previousSelected->hasAudio();
    if (!hasAudio)
    {
        gStorage->UnmountInstalledDLC("TPACK");
        minecraft->mSoundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x1c, 0x1f,
                                                    0x20, 0x29, 0x2a, 0x2a, 0x3e, 0x44, 0x57);
        minecraft->mSoundEngine->playStreaming(NULL, 0.f, 0.f, 0.f, 1.f, 1.f, false, false,
                                               ConsoleSoundEngine::eAudioDelayType_Random);
    }
    else
    {
        minecraft->mSoundEngine->SetStreamingSounds(0, 0xb, 0xc, 0xf, 0x10, 0x11, 0x12, 0x17, 0x18, 0x1b, 0x1c, 0x1f,
                                                    0x20, 0x29, 0x2a, 0x2a, 0x3e, 0x44, 0x57);
        minecraft->mSoundEngine->playStreaming(NULL, 0.f, 0.f, 0.f, 1.f, 1.f, false, false,
                                               ConsoleSoundEngine::eAudioDelayType_Random);
        gStorage->UnmountInstalledDLC("TPACK");
    }

    while (gApp.GetXuiAction(gProfile->GetPrimaryPad()) != eXuiAction_Idle)
        Sleep(1);

    const int player = *static_cast<int*>(data);
    gApp.SetAction(player, eXuiAction_JoinInvitedGame, data);
    return 0;
}
int CGameNetworkManager::TryToConnectOnline(int (*cb)(void*, bool, int), void* d, bool p)
{
    if (g_pPlatformNetworkManager)
        g_pPlatformNetworkManager->TryToConnectOnline(cb, d, p);
    return 0;
}
bool CGameNetworkManager::_LeaveGame()
{
    return g_pPlatformNetworkManager->_LeaveGame(false, true);
}
int CGameNetworkManager::ChangeSessionTypeThreadProc(void*)
{
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();

    Minecraft* minecraft = Minecraft::GetInstance();
    MinecraftServer* server = gMinecraftServer;
    minecraft->mProgressRenderer->progressStartNoAbort(sInstance.CorrectErrorIDS(IDS_CONNECTION_LOST_LIVE_NO_EXIT));
    minecraft->mProgressRenderer->progressStage(0x13b73b12);

    while (gApp.GetXuiServerAction(gProfile->GetPrimaryPad()) != 0 && !MinecraftServer::s_haltRequested)
        Sleep(10);

    gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_PauseServer, reinterpret_cast<void*>(1));

    server->serverPausedEvent->WaitForSignal(-1);

    minecraft->mProgressRenderer->progressStartNoAbort(sInstance.CorrectErrorIDS(IDS_CONNECTION_LOST_LIVE_NO_EXIT));
    minecraft->mProgressRenderer->progressStage(0x13b73b12);
    minecraft->mProgressRenderer->progressStagePercentage(0x19);

    std::vector<boost::shared_ptr<ServerPlayer> >* currentPlayers = server->getPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = currentPlayers->begin();
         it < currentPlayers->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> serverPlayer = *it;
        if (serverPlayer->mConnection->isLocal() && !serverPlayer->mConnection->isGuest())
            serverPlayer->mConnection->mConnection->m_socket->setPlayer(0);
    }

    if (!sInstance.m_disconnectError)
        g_pPlatformNetworkManager->_LeaveGame(false, true);
    else
        g_pPlatformNetworkManager->_LeaveGame(false, false);

    while (sInstance.IsInSession())
        Sleep(1);

    sInstance.m_disconnectError = false;
    sInstance.m_unknownState = false;
    minecraft->mProgressRenderer->progressStagePercentage(0x32);
    sInstance.SetLocalGame(true);

    unsigned int localMask = 0;
    char localPlayerCount = 0;
    for (int userIndex = 0; userIndex < 4; ++userIndex)
    {
        if (gProfile->IsSignedIn(userIndex) && minecraft->mLocalPlayers[userIndex] != 0)
        {
            ++localPlayerCount;
            localMask |= GetLocalPlayerMask(userIndex);
        }
    }

    g_pPlatformNetworkManager->_HostGame(localMask, 0, eMiniGameId_Any, 0, 0);
    minecraft->mProgressRenderer->progressStagePercentage(0x4b);

    while (sInstance.GetPlayerCount() < localPlayerCount)
        Sleep(1);

    if (server != 0)
    {
        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            if (gProfile->IsSignedIn(userIndex) && minecraft->mLocalPlayers[userIndex] != 0)
            {
                PlayerUID localUid = minecraft->mLocalPlayers[userIndex]->getXuid();
                std::vector<boost::shared_ptr<ServerPlayer> >* rebindPlayers = server->getPlayers()->getPlayers();
                for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = rebindPlayers->begin();
                     it < rebindPlayers->end(); ++it)
                {
                    boost::shared_ptr<ServerPlayer> serverPlayer = *it;
                    if (serverPlayer->getXuid() == localUid)
                        serverPlayer->mConnection->mConnection->m_socket->setPlayer(
                            sInstance.GetLocalPlayerByUserIndex(userIndex));
                }

                if (minecraft->mPendingConnections[userIndex] != 0)
                {
                    minecraft->mPendingConnections[userIndex]->getConnection()->m_socket->setPlayer(
                        sInstance.GetLocalPlayerByUserIndex(userIndex));
                    continue;
                }

                if (minecraft->mConnectionFailed[userIndex]
                    && minecraft->mDisconnectReasons[userIndex] == DisconnectPacket::eDisconnectReason_Banned)
                {
                    minecraft->removeLocalPlayerIdx(userIndex);
                    gProfile->SignOutUser(userIndex);
                }
            }
        }
    }

    minecraft->mProgressRenderer->progressStagePercentage(100);

    while (!g_pPlatformNetworkManager->IsReadyToPlayOrIdle())
        Sleep(10);

    if (server != 0)
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* players = server->getPlayers()->getPlayers();
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin(); it != players->end(); ++it)
        {
            boost::shared_ptr<ServerPlayer> serverPlayer = *it;
            serverPlayer->mLoadedChunks.insert(serverPlayer->mLoadedChunks.end(), serverPlayer->mTrackedChunks.begin(),
                                               serverPlayer->mTrackedChunks.end());
        }
    }

    g_pPlatformNetworkManager->_StartGame();

    while (gConsoleUIController.IsSceneInStack(0xff, EUIScene_MessageBox))
        Sleep(10);

    gApp.SetGameStarted(true);
    gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_PauseServer, 0);
    gApp.SetChangingSessionType(false);
    gApp.SetReallyChangingSessionType(false);
    return 0;
}
void CGameNetworkManager::SystemFlagSet(INetworkPlayer* p, int f)
{
    if (g_pPlatformNetworkManager)
        g_pPlatformNetworkManager->SystemFlagSet(p, f);
}
bool CGameNetworkManager::SystemFlagGet(INetworkPlayer* p, int f)
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->SystemFlagGet(p, f);
}
void CGameNetworkManager::SystemFlagClearAll(INetworkPlayer* p)
{
    if (g_pPlatformNetworkManager)
        g_pPlatformNetworkManager->SystemFlagClearAll(p);
}
void CGameNetworkManager::ServerReady()
{
    m_serverReadyEvent->Set();
}
void CGameNetworkManager::ServerStopped()
{
    m_serverStoppedEvent->Set();
}
void CGameNetworkManager::ServerStoppedWait()
{
    if (!C4JThread::isMainThread())
    {
        m_serverStoppedEvent->WaitForSignal(-1);
        return;
    }
    unsigned int result;
    do
    {
        gRenderer->StartFrame();
        result = m_serverStoppedEvent->WaitForSignal(20);
        gProfile->Tick();
        gStorage->Tick();
        g_pInput->Tick();
        gRenderer->Tick();
        gConsoleUIController.tick();
        gConsoleUIController.render();
        gRenderer->Present();
    } while (result == C4JEvent::WaitTimeout);
}
void CGameNetworkManager::ServerStoppedDestroy()
{
    delete m_serverStoppedEvent;
    m_serverStoppedEvent = 0;
}
bool CGameNetworkManager::ServerStoppedValid()
{
    return m_serverStoppedEvent != 0;
}
void CGameNetworkManager::RemoveCreatedConnection(ClientPacketListener* p)
{
    EnterCriticalSection(&m_connectionsMutex);
    std::vector<ClientPacketListener*>::iterator i = m_connections.begin();
    while (i != m_connections.end() && *i != p)
    {
        ++i;
    }
    if (i != m_connections.end())
    {
        m_connections.erase(i);
    }
    LeaveCriticalSection(&m_connectionsMutex);
}
bool CGameNetworkManager::usingLanMode()
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->usingLanMode();
}
bool CGameNetworkManager::setLanMode(bool v)
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->setLanMode(v);
}
bool CGameNetworkManager::isLanConnected()
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->isLanConnected();
}
bool CGameNetworkManager::isLanModeChangeInProgress()
{
    return g_pPlatformNetworkManager && g_pPlatformNetworkManager->isLanModeChangeInProgress();
}
std::wstring CGameNetworkManager::GatherStats()
{
    return g_pPlatformNetworkManager->GatherStats();
}
void CGameNetworkManager::renderQueueMeter()
{
    return;
}
std::wstring CGameNetworkManager::GatherRTTStats()
{
    return g_pPlatformNetworkManager->GatherRTTStats();
}
void CGameNetworkManager::SetSessionGameModeId(int id)
{
    g_pPlatformNetworkManager->SetSessionGameModeId(id);
}
void CGameNetworkManager::StateChange_AnyToHosting()
{
    XEnableGuestSignin(0);
    Minecraft::GetInstance()->mPendingClientTextureRequests.clear();
}
void CGameNetworkManager::StateChange_AnyToJoining(bool)
{
    XEnableGuestSignin(0);
    Minecraft::GetInstance()->mPendingClientTextureRequests.clear();

    _ConnectionProgressParams* params = new _ConnectionProgressParams();
    params->m_stringId = -1;
    params->m_playerId = gProfile->GetPrimaryPad();
    params->m_showTooltips = false;
    params->m_setFailTimer = true;
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame))
        params->m_miniGameId = gApp.GetFirstMiniGameType();
    params->m_timerTime = 500;
    gConsoleUIController.CloseUIScenes(gProfile->GetPrimaryPad(), false);
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_ConnectingProgress, params, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
}
void CGameNetworkManager::StateChange_JoiningToIdle(CPlatformNetworkManager::eJoinFailedReason reason)
{
    DisconnectPacket::eDisconnectReason disconnectReason;
    switch (reason)
    {
    case CPlatformNetworkManager::eJoinFailedReason_SessionFull:
        disconnectReason = DisconnectPacket::eDisconnectReason_GameFull;
        break;
    case CPlatformNetworkManager::eJoinFailedReason_NotAllowedOnline:
        gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_MultiplayerNotAllowed, 0);
        disconnectReason = DisconnectPacket::eDisconnectReason_NoMultiplayerPrivilegeJoin;
        break;
    case CPlatformNetworkManager::eJoinFailedReason_ConnectionLost:
        disconnectReason = DisconnectPacket::eDisconnectReason_NoSuitableGame;
        break;
    default:
        disconnectReason = DisconnectPacket::eDisconnectReason_Banned;
        break;
    }
#ifdef MINECRAFT_PC
    if (gApp.GetDisconnectReason() == DisconnectPacket::eDisconnectReason_PcMismatchedContent)
        disconnectReason = DisconnectPacket::eDisconnectReason_PcMismatchedContent;
#endif
    Minecraft* minecraft = Minecraft::GetInstance();
    int primaryPad = gProfile->GetPrimaryPad();
    minecraft->mConnectionFailed[primaryPad] = true;
    minecraft->mDisconnectReasons[primaryPad] = disconnectReason;
}
void CGameNetworkManager::StateChange_AnyToStarting()
{
    if (IsHost())
    {
        return;
    }
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_threadProc = &CGameNetworkManager::RunNetworkGameThreadProc;
    params->m_threadParam = 0;
    FullscreenProgressState* progressState = new FullscreenProgressState();
    progressState->reservedFlag1 = true;
    progressState->reservedFlag2 = true;
    progressState->closeMode = 3;
    progressState->playerId = gProfile->GetPrimaryPad();
    params->m_completionData = progressState;
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame))
    {
        params->m_miniGameId = gApp.GetFirstMiniGameType();
    }
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
}
void CGameNetworkManager::StateChange_AnyToEnding(bool wasPlaying, bool)
{
    if (wasPlaying && sInstance.IsLocalGame())
    {
        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            INetworkPlayer* player = sInstance.GetLocalPlayerByUserIndex(userIndex);
            if (player != 0 && gProfile->IsSignedIn(userIndex))
            {
                Minecraft::GetInstance()->forceStatsSave(player->GetUserIndex());
            }
        }
    }
    Minecraft::GetInstance()->mGui->clearMessages(-1);
    if (!sInstance.IsHost() && !sInstance.IsLeavingGame())
    {
        if (gApp.GetDisconnectReason() == 0)
        {
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_Quit);
        }
        gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_ExitWorld, reinterpret_cast<void*>(1));
    }
}
void CGameNetworkManager::CreateSocket(INetworkPlayer* player, bool localPlayer)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> mpPlayer = minecraft->mLocalPlayers[player->GetUserIndex()];
    if (localPlayer && mpPlayer != NULL && mpPlayer->mPacketListener != NULL)
    {
        Socket* socket = mpPlayer->mPacketListener->getSocket();
        player->SetSocket(socket);
        if (socket != NULL)
        {
            socket->setPlayer(player);
        }
    }
    else
    {
        Socket* socket = new Socket(player, sInstance.IsHost(), sInstance.IsHost() && localPlayer);
        player->SetSocket(socket);
        if (sInstance.IsHost() && sInstance.IsInGameplay())
        {
            Socket::addIncomingSocket(socket);
        }
        if (localPlayer && sInstance.IsInGameplay())
        {
            int userIndex = player->GetUserIndex();
            ClientPacketListener* connection = new ClientPacketListener(minecraft, socket, userIndex);
            if (connection->m_createdOk)
            {
                connection->send(boost::shared_ptr<ServerboundPreLoginPacket>(
                    new ServerboundPreLoginPacket(player->GetOnlineName())));
                minecraft->addPendingLocalConnection(userIndex, connection);
            }
            else
            {
                minecraft->mConnectionFailed[userIndex] = true;
                minecraft->mDisconnectReasons[userIndex] = DisconnectPacket::eDisconnectReason_Banned;
                delete connection;
            }
        }
    }
}
void CGameNetworkManager::CloseConnection(INetworkPlayer* player)
{
    if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
    {
        gMinecraftServer->getPlayers()->closeServerGamePacketListenerImplBySmallId(player->GetSmallId());
    }
}
void CGameNetworkManager::PlayerJoining(INetworkPlayer* player)
{
    if (sInstance.IsInGameplay())
    {
        bool few = sInstance.GetPlayerCount() < 2;
        bool localGame = sInstance.IsLocalGame();
        int activityCode = few ? (localGame ? 5 : 4) : (localGame ? 3 : 2);
        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            if (sInstance.GetLocalPlayerByUserIndex(userIndex) != 0)
            {
                gApp.SetRichPresenceContext(userIndex, 0);
                gProfile->SetCurrentGameActivity(userIndex, activityCode, false);
            }
        }
    }
    if (player->IsLocal())
    {
        telemetryManagerInstance->RecordPlayerSessionStart(player->GetUserIndex());
        telemetryManagerInstance->RecordPlayerSettings(player->GetUserIndex());
    }
}
void CGameNetworkManager::PlayerLeaving(INetworkPlayer* player)
{
    if (player->IsLocal())
    {
        gProfile->SetCurrentGameActivity(player->GetUserIndex(), 0, false);
        telemetryManagerInstance->RecordPlayerSessionExit(player->GetUserIndex(), gApp.GetDisconnectReason());
    }
}
void CGameNetworkManager::HostChanged()
{
    gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_ExitWorld, (void*)1);
}
void CGameNetworkManager::WriteStats(INetworkPlayer* player)
{
    Minecraft::GetInstance()->forceStatsSave(player->GetUserIndex());
}
void CGameNetworkManager::AddLocalPlayerFailed(int userIndex, bool full)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mConnectionFailed[userIndex] = true;
    minecraft->mDisconnectReasons[userIndex] = full ? DisconnectPacket::eDisconnectReason_GameFull
                                                    : DisconnectPacket::eDisconnectReason_Banned;
}
int CGameNetworkManager::GetPrimaryPad()
{
    return gProfile->GetPrimaryPad();
}
int CGameNetworkManager::GetLockedProfile()
{
    return gProfile->GetLockedProfile();
}
void CGameNetworkManager::HandleDisconnect(bool error)
{
    int primaryPad = GetPrimaryPad();
    if (GetLockedProfile() != -1 && primaryPad != -1 && IsInSession())
    {
        m_disconnectError = error;
        gApp.SetAction(primaryPad, eXuiAction_ConnectionLost, 0);
    }
    else
    {
        m_disconnectError = false;
    }
}
bool CGameNetworkManager::IsSignedInLive(int userIndex)
{
    return gProfile->IsSignedInLive(userIndex);
}
bool CGameNetworkManager::AllowedToPlayMultiplayer(int userIndex)
{
    return gProfile->AllowedToPlayMultiplayer(userIndex);
}
void CGameNetworkManager::FakeLocalPlayerJoined()
{
    g_pPlatformNetworkManager->FakeLocalPlayerJoined();
}
bool CGameNetworkManager::IsHandlingBootInvite()
{
    return g_pPlatformNetworkManager->IsHandlingBootInvite();
}
void CGameNetworkManager::WiiU_ClearIsHandlingBootInvite()
{
    NQRNetworkManager_WiiU::ClearIsHandlingBootInvite();
}
void CGameNetworkManager::WiiU_ShowErrorViewer()
{
    NQRNetworkManager_WiiU::ShowErrorViewer();
}
void CGameNetworkManager::ShowConnectOnlineNintendoProgress(int i, int (*cb)(void*, bool, int, int), void* d)
{
    NQRNetworkManager_WiiU::ShowConnectOnlineNintendoProgress(i, cb, d);
}
void CGameNetworkManager::WiiU_ReleaseForeground()
{
    NintendoVoiceChat_WiiU::ReleaseForeground();
    NQRNetworkManager_WiiU::SetAppSuspended(true);
}
void CGameNetworkManager::WiiU_AcquiredForeground()
{
    NintendoVoiceChat_WiiU::AcquiredForeground();
    NQRNetworkManager_WiiU::SetAppSuspended(false);
    MasterGameMode::ResetLobbyTimerAfterSuspend();
}
void CGameNetworkManager::WiiU_ReleaseCallback()
{
    NintendoVoiceChat_WiiU::ReleaseCallback();
}
void CGameNetworkManager::WiiU_SetBootFromFriendList()
{
    SYSArgDataBlock args[2] = {{SYS_ARG_ID_JOIN_PID}, {SYS_ARG_ID_END}};
    SYSCallerInfo callerInfo;
    if (SYSGetArguments(args, &callerInfo) == 0 && args[0].type == SYS_ARG_TYPE_UINT32)
    {
        g_bGameBootInvitePending = true;
        g_uiGameBootInvitePrincipalId = args[0].uint32;
        g_bHandlingGameBootInvite = true;
        gApp.setIgnoreMainMenuInputDuringInvite(true);
    }
}
bool CGameNetworkManager::IsUGCRestricted(int userIndex)
{
    bool explicitUser = userIndex != 0xff;
    if (!explicitUser)
    {
        userIndex = gProfile->GetPrimaryPad();
    }
    int allowed = 1;
    int friendsOnly = 1;
    gProfile->AllowedPlayerCreatedContent(userIndex, explicitUser, &allowed, &friendsOnly);
    return !allowed && !friendsOnly;
}
bool CGameNetworkManager::ModeAllowsUGC(EMiniGameId miniGameId)
{
    MiniGameDef* mode = MiniGameDef::GetCustomGameModeById(miniGameId, true);
    return mode != 0 && mode->AllowsUGC();
}
bool CGameNetworkManager::IsUGCRestrictedMode(int i, EMiniGameId m)
{
    return IsUGCRestricted(i) && ModeAllowsUGC(m);
}
bool CGameNetworkManager::IsFriendsOnlyUGC(int userIndex)
{
    int allowed = 0;
    int friendsOnly = 0;
    gProfile->AllowedPlayerCreatedContent(userIndex, true, &allowed, &friendsOnly);
    return allowed != 1 && friendsOnly == 1;
}
void CGameNetworkManager::CheckFriendsCanPlay(int userIndex, int& friends,
                                              boost::shared_ptr<ClientboundPreLoginPacket> packet, int titleVersion,
                                              int& can, int& cannot, int& blocked, int& unknown)
{
    if (g_pPlatformNetworkManager)
    {
        g_pPlatformNetworkManager->CheckFriendsCanPlay(userIndex, friends, packet, titleVersion, can, cannot, blocked,
                                                       unknown);
    }
}
unsigned int CGameNetworkManager::GetMaxPlayers(EMiniGameId miniGameId, unsigned int)
{
    if (miniGameId != eMiniGameId_Any)
    {
        MiniGameDef::GetCustomGameModeById(miniGameId, true);
#ifdef MINECRAFT_PC
        return GameSessionData::MAX_MINI_GAME_PLAYERS;
#endif
    }
#ifdef MINECRAFT_PC
    return GameSessionData::MAX_HUMAN_PLAYERS;
#else
    return GameSessionData::MAX_SESSION_PLAYERS;
#endif
}
int CGameNetworkManager::GetMaxPlayers(GameSessionData* data)
{
    if (data->m_miniGameId != eMiniGameId_Any)
    {
        MiniGameDef::GetCustomGameModeById((EMiniGameId)data->m_miniGameId, true);
#ifdef MINECRAFT_PC
        return GameSessionData::MAX_MINI_GAME_PLAYERS;
#endif
    }
#ifdef MINECRAFT_PC
    return GameSessionData::MAX_HUMAN_PLAYERS;
#else
    return GameSessionData::MAX_SESSION_PLAYERS;
#endif
}
unsigned int CGameNetworkManager::GetMaxPlayers()
{
#ifdef MINECRAFT_PC
    unsigned int players = GameSessionData::MAX_HUMAN_PLAYERS + (PcIsDedicatedServer() ? 1 : 0);
#else
    unsigned int players = GameSessionData::MAX_SESSION_PLAYERS;
#endif
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        EMiniGameId id = (EMiniGameId)gApp.GetFirstMiniGameType();
        const MiniGameDef* game = Minecraft::GetInstance()->GetMiniGame();
        players = GetMaxPlayers(id, game->GetHostOption(eCustomGameModeHostOption_All));
    }
    return players;
}
void CGameNetworkManager::AttemptPSNSignIn(int (*cb)(void*, bool, int), void* d, bool p, int i)
{
    g_pPlatformNetworkManager->AttemptPSNSignIn(cb, d, p, i);
}
void CGameNetworkManager::RecvInviteGUI()
{
    g_pPlatformNetworkManager->RecvInviteGUI();
}
void CGameNetworkManager::ClearInviteInfo()
{
    m_joiningMiniGameId = eMiniGameId_Lobby;
    m_inviteInfo = 0;
}

namespace nn
{
namespace erreula
{
struct HomeNixSignArg
{
    int controller;
};

void AppearHomeNixSign(const HomeNixSignArg& arg);
}
}

unsigned int CallbackAcquiredForeground(void*)
{
    gRenderer->AcquiredForeground();
    g_pInput->AcquiredForeground();
    if (CommerceAppletIsExecuting())
    {
        CommerceAppletFinish();
        CommerceAppletGetReturnCode();
        cCommerceNintendo* commerce = static_cast<cCommerceNintendo*>(
            reinterpret_cast<void*>(gApp.GetCommerceInstance()));
        commerce->SetProcessState(cCommerceNintendo::eCommerce_Process_State_ForegroundAcquired);
    }
    CGameNetworkManager::sInstance.WiiU_AcquiredForeground();
    return 0;
}

unsigned int CallbackReleaseForeground(void*)
{
    gRenderer->ReleaseForeground();
    CGameNetworkManager::sInstance.WiiU_ReleaseForeground();
    return 0;
}

void closeSocketForLockedProfile()
{
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(gProfile->GetLockedProfile());
    if (player != 0)
    {
        Socket* socket = player->GetSocket();
        if (socket != 0 && !socket->mClosed[0] && !socket->mClosed[1])
        {
            socket->close(true);
        }
    }
}
