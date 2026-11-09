#include "game/ui/UIScene_PauseMenu.hpp"
#include "game/core/StringIds.hpp"

#include <string>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/GameType.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/L10N.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/MiniGamePlayerSettings.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/DLCPack.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/core/cCommerce.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/core/Compression.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "game/core/GameSettings.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

extern ConsoleUIController gConsoleUIController;

void UIScene_PauseMenu::Init()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    bool primaryPlayer = gProfile->GetPrimaryPad() == m_playerId;
    gApp.GetLocalPlayerCount();

    m_buttonList.AddNewMenuButton(L10N::GetString(IDS_RESUME_GAME), 0);
    m_buttonList.AddNewMenuButton(L10N::GetString(IDS_HELP_AND_OPTIONS), 1);

    PlayerUID xuid(0);
    gProfile->GetXUID(m_playerId, &xuid, false);
    if ((xuid.flags & PlayerUID::FlagUuid) != 0
        && (gProfile->GetPrimaryPad() == m_playerId || gProfile->IsHasAchievementsFile(m_playerId)))
    {
        m_buttonList.AddNewMenuButton(L10N::GetString(IDS_ACHIEVEMENTS), 4);
    }

    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0)
    {
        ClientMasterGameMode* masterGameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
        const MiniGameDef* miniGame = CommonMasterGameMode::GetMiniGame();
        MiniGamePlayerSettings settings;
        MiniGameDef::GetPlayerSettings(settings);
        std::vector<ClientMasterGameMode::MapSelectInfo> playlist;
        masterGameMode->GetUniquePlaylist(playlist);

        bool multipleMaps = playlist.size() > 1;
        bool multipleVariants = false;
        if (Minecraft::InMiniGame(eMiniGameId_Lobby, true) && minecraft->GetMiniGame()->HasMultipleVariants()
            && Minecraft::GetInstance()->GetFirstMiniGame()->GetId() != eMiniGameId_Battle)
        {
            multipleVariants = true;
        }
        bool buildGame = Minecraft::InMiniGame(eMiniGameId_Build, true);
        bool customGame = MiniGameDef::IsCustomGame(
            static_cast<EMiniGameId>(Minecraft::GetInstance()->GetFirstMiniGame()->GetId()), settings);

        if (multipleMaps || multipleVariants || buildGame)
        {
            if (CGameNetworkManager::sInstance.IsInPublicJoinableGame())
            {
                if (multipleMaps)
                {
                    m_buttonList.AddNewMenuButton(L10N::GetString(IDS_MG_OPTION_MAP_VOTE), 8);
                }
                else if (!customGame && multipleVariants
                         && ((CGameNetworkManager::sInstance.IsHost()
                              && !CGameNetworkManager::sInstance.IsInPublicJoinableGame())
                             || miniGame->GetHostOption(eCustomGameModeHostOption_EnableVoting)))
                {
                    m_buttonList.AddNewMenuButton(L10N::GetString(IDS_MG_OPTION_VARIANT_VOTE), 8);
                }
                else if (buildGame)
                {
                    m_buttonList.AddNewMenuButton(L10N::GetString(IDS_MG_OPTION_VARIANT_VOTE), 8);
                }
            }
            else if (primaryPlayer && CGameNetworkManager::sInstance.IsHost() && minecraft != NULL
                     && minecraft->mLocalPlayers[m_playerId].get()->GetGameType() == GameType::LOBBY)
            {
                if (multipleMaps)
                    m_buttonList.AddNewMenuButton(L10N::GetString(IDS_MG_SELECT_MAP), 8);
                else if (!customGame && multipleVariants
                         && ((CGameNetworkManager::sInstance.IsHost()
                              && !CGameNetworkManager::sInstance.IsInPublicJoinableGame())
                             || miniGame->GetHostOption(eCustomGameModeHostOption_EnableVoting)))
                    m_buttonList.AddNewMenuButton(L10N::GetString(IDS_MG_SELECT_VARIANT), 8);
                else if (buildGame)
                    m_buttonList.AddNewMenuButton(L10N::GetString(IDS_MG_SELECT_VARIANT), 8);
            }
        }

        if (customGame)
        {
            if (Minecraft::GetInstance()->GetFirstMiniGame()->GetId() == eMiniGameId_Battle)
                m_buttonList.AddNewMenuButton(L10N::GetString(IDS_MG01_SETTINGS), 7);
            else if (Minecraft::GetInstance()->GetFirstMiniGame()->GetId() == eMiniGameId_Tumble)
                m_buttonList.AddNewMenuButton(L10N::GetString(IDS_MG02_SETTINGS), 7);
        }
    }
    else if (primaryPlayer && CGameNetworkManager::sInstance.IsHost())
    {
        m_buttonList.AddNewMenuButton(L10N::GetString(IDS_SAVE_GAME), 5);
        if (gApp.HasNoSpaceForSaveQuota())
            m_buttonList.EnableItem(5, false, true);
    }

    m_buttonList.AddNewMenuButton(L10N::GetString(IDS_EXIT_GAME), 6);
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) == 0
        || !CGameNetworkManager::sInstance.IsInPublicJoinableGame() || gProfile->GetPrimaryPad() != m_playerId
        || !CGameNetworkManager::sInstance.IsHost() || minecraft == NULL
        || minecraft->mLocalPlayers[m_playerId].get()->GetGameType() == GameType::LOBBY
        || CGameNetworkManager::sInstance.GetOnlinePlayerCount() == 0)
    {
        m_buttonList.EnableItem(6, true, true);
        m_buttonList.SetItemOpacity(6, 1.0f, true);
    }
    else
    {
        m_buttonList.EnableItem(6, false, true);
        m_buttonList.SetItemOpacity(6, 0.3f, true);
    }
    if (m_lastIndex >= 0)
        m_buttonList.setCurrentSelection(m_lastIndex, false, true);

    if (CGameNetworkManager::sInstance.IsLocalGame() && CGameNetworkManager::sInstance.GetPlayerCount() == 1)
        gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_PauseServer, reinterpret_cast<void*>(1));
}

void UIScene_PauseMenu::tick()
{
    UIScene::tick();
    if (gConsoleUIController.GetTopScene(m_playerId, EUILayer_Scene, EUIGroup_PlayerDefault) != this)
        return;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0 && CGameNetworkManager::sInstance.IsInPublicJoinableGame()
        && gProfile->GetPrimaryPad() == m_playerId && CGameNetworkManager::sInstance.IsHost() && minecraft != NULL
        && minecraft->mLocalPlayers[m_playerId].get()->GetGameType() != GameType::LOBBY
        && CGameNetworkManager::sInstance.GetOnlinePlayerCount() != 0)
    {
        if (m_buttonList.IsEnabled(6))
        {
            m_buttonList.EnableItem(6, false, true);
            m_buttonList.SetItemOpacity(6, 0.3f, true);
        }
    }
    else
    {
        if (!m_buttonList.IsEnabled(6))
        {
            m_buttonList.EnableItem(6, true, true);
            m_buttonList.SetItemOpacity(6, 1.0f, true);
        }
    }
}

void UIScene_PauseMenu::updateTooltips()
{
    bool host = CGameNetworkManager::sInstance.IsHost() && gProfile->GetPrimaryPad() == m_playerId;
    int saveTooltip = -1;
    if (gProfile->IsFullVersion())
    {
        if (gStorage->GetSaveDisabled())
        {
            if (host)
                saveTooltip = IDS_TOOLTIPS_SELECTDEVICE;
            else
                saveTooltip = -1;
        }
        else if (host)
            saveTooltip = IDS_TOOLTIPS_CHANGEDEVICE;
        else
            saveTooltip = -1;
    }
    if (gConsoleUIController.IsSceneInStack(m_playerId, EUIScene_FullscreenProgress))
        gConsoleUIController.SetTooltips(m_playerId, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         false, false);
    else
        gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, saveTooltip, -1, -1, -1,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_PauseMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
    m_layer->showComponent(m_playerId, EUIScene_MenuBackground, true);
    bool showLogo = false;
    if (gApp.GetLocalPlayerCount() == 1)
        showLogo = true;
    m_layer->showComponent(m_playerId, EUIScene_Logo, showLogo);
}

void UIScene_PauseMenu::handlePreReload()
{
}

void UIScene_PauseMenu::handleReload()
{
    updateTooltips();
    int focusIndex = m_focusIndex;
    Init();
    m_focusIndex = focusIndex;
    doHorizontalResizeCheck();
    gConsoleUIController.TouchBoxRebuild(this);
    m_buttonList.setCurrentSelection(m_focusIndex, false, true);
}

std::wstring UIScene_PauseMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"PauseMenuSplit") : std::wstring(L"PauseMenu");
}

void UIScene_PauseMenu::handleFocusChange(int, int childFocus)
{
    m_focusIndex = m_buttonList.getListIndex(childFocus);
}

void UIScene_PauseMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool&)
{
    if (m_ignoreInput)
        return;
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
            navigateBack();
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuOk:
        if (pressed)
            sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_PauseMenu::handlePress(int, int childId)
{
    if (m_ignoreInput)
        return;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    m_lastIndex = m_buttonList.getListIndex(childId);
    switch (childId)
    {
    case 0:
        navigateBack();
        return;
    case 2:
    {
        int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
        if (gProfile->IsGuest(m_playerId))
        {
            gConsoleUIController.RequestAlertMessage(0x0e6bf122, static_cast<int>(0x8ba9896fU), parameters, 1,
                                                     gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
            return;
        }
        if (!gProfile->IsSignedInLive(m_playerId))
        {
            int errorParameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
            gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_PRO_NOTONLINE_TITLE),
                                                     static_cast<int>(0xc34c0b77U), errorParameters, 1, m_playerId,
                                                     NULL, NULL, NULL, true);
            return;
        }
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_LeaderboardsMenu, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        return;
    }
    case 4:
    {
        if (gProfile->IsGuest(m_playerId))
        {
            int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
            gConsoleUIController.RequestAlertMessage(0x0e6bf122, static_cast<int>(0x8ba9896fU), parameters, 1,
                                                     gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
            return;
        }
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_AchievementsMenu, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        return;
    }
    case 1:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_HelpAndOptionsMenu, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        return;
    case 5:
        PerformActionSaveGame();
        return;
    case 8:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_MapVoteMenu,
                                             reinterpret_cast<void*>(static_cast<unsigned char>(
                                                 !CGameNetworkManager::sInstance.IsInPublicJoinableGame())),
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    case 7:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_InGameSettingsListMenu, NULL, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        return;
    case 6:
        OpenExitGameDialog(m_playerId, reinterpret_cast<void*>(GetCallbackUniqueId()), ExitGameDialogReturned,
                           ExitGameSaveDialogReturned, EOpenExitGameDialog_ExitGame);
        break;
    case 3:
        break;
    }
}

void UIScene_PauseMenu::ShowScene(bool)
{
}
void UIScene_PauseMenu::SetIgnoreInput(bool ignore)
{
    m_ignoreInput = ignore;
}

unsigned int UIScene_PauseMenu::BuyTexturePack_SignInReturned(void*, bool signedIn, int userIndex, int)
{
    if (signedIn && gProfile->IsSignedInLive(userIndex))
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        TexturePackRepository* repository = minecraft->mTexturePackRepository;
        DLCPack* pack = static_cast<DLCTexturePack*>(repository->getSelected())->getDLCInfoParentPack();
        gApp.GetCommerceInstance()->PurchaseDLCByPackID(pack->GetPackId());
    }
    return 0;
}

bool UIScene_PauseMenu::IsBanEnabled()
{
    bool host = CGameNetworkManager::sInstance.IsHost() && gProfile->GetPrimaryPad() == m_playerId;
    return !CGameNetworkManager::sInstance.IsLocalGame() && !host && !gProfile->IsGuest(m_playerId)
           && !Minecraft::InMiniGame(eMiniGameId_Any, true);
}

unsigned int UIScene_PauseMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_buttonList.setupControl(this, currentNode, std::string("ButtonList"));
        m_controls.push_back(&m_buttonList);
    }
    return 1;
}

int UIScene_PauseMenu::getSceneType()
{
    return EUIScene_PauseMenu;
}

UIScene_PauseMenu::UIScene_PauseMenu(int sceneType, void*, UILayer* layer) : UIScene(sceneType, layer), m_buttonList()
{
    initialiseMovie();
    m_ignoreInput = false;
    m_lastIndex = -1;
    m_unusedValue = 0;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (!gProfile->IsFullVersion())
        gConsoleUIController.ShowTrialTimer(false);
    Init();
    doHorizontalResizeCheck();
    gConsoleUIController.HidePressStart();
    telemetryManagerInstance->RecordPauseOrInactive(m_playerId);
    if (minecraft != NULL && minecraft->mGameModes[sceneType] != NULL)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[sceneType]->getTutorial();
        if (tutorial != NULL)
            tutorial->showTutorialPopup(false);
    }
    m_unusedFlag2 = false;
    m_focusIndex = 0;
    gConsoleUIController.TouchBoxRebuild(this);
}

UIScene_PauseMenu::~UIScene_PauseMenu()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft != NULL && minecraft->mGameModes[m_playerId] != NULL)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[m_playerId]->getTutorial();
        if (tutorial != NULL)
            tutorial->showTutorialPopup(true);
    }
    if (!gProfile->IsFullVersion())
        gConsoleUIController.ShowTrialTimer(true);
    if (CGameNetworkManager::sInstance.IsLocalGame() && minecraft->mGameModes[m_playerId] != NULL)
        gApp.SetXuiServerAction(gProfile->GetPrimaryPad(), eXuiServerAction_PauseServer, NULL);
    m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
    m_layer->showComponent(m_playerId, EUIScene_MenuBackground, false);
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

int UIScene_PauseMenu::SaveWorldThreadProc(void* context)
{
    bool autosave = context != NULL;
    int primaryPad = gProfile->GetPrimaryPad();
    gApp.SetXuiServerAction(primaryPad, autosave ? eXuiServerAction_AutoSaveGame : eXuiServerAction_SaveGame, NULL);
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    if (gProfile->IsFullVersion())
    {
        gApp.SetGameStarted(false);
        while (gApp.GetXuiServerAction(gProfile->GetPrimaryPad()) != eXuiServerAction_Idle
               && !MinecraftServer::s_haltRequested)
            Sleep(10);
        if (!MinecraftServer::s_haltRequested && !gApp.GetChangingSessionType())
            gApp.SetGameStarted(true);
    }
    if (gApp.GetChangingSessionType())
        return ERROR_CANCELLED;
    return 0;
}

int UIScene_PauseMenu::NetworkErrorHandledDuringInvite(void*, int, C4JStorage::EMessageResult)
{
    gProfile->ClearGameUsers();
    gProfile->SetLockedProfile(-1);
    CGameNetworkManager::sInstance.WiiU_ClearIsHandlingBootInvite();
    return 0;
}

void UIScene_PauseMenu::_ExitWorld(void* context)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    int exitTitle = minecraft->mProgressRenderer->getCurrentTitle();
    int errorTitle = static_cast<int>(IDS_CONNECTION_LOST);
    if (Minecraft::GetInstance()->isClientSide())
        Minecraft::GetInstance()->SetupMiniGameInstance(*MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true), 0);

    bool saveStats = true;
    int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
    if (minecraft->isClientSide() || CGameNetworkManager::sInstance.IsInSession())
    {
        if (context == NULL)
        {
            exitTitle = IDS_EXITING_GAME;
            minecraft->mProgressRenderer->progressStartNoAbort(IDS_EXITING_GAME);
            for (int i = 0; i < 3; ++i)
            {
                if (minecraft->mLevels[i] != NULL)
                    minecraft->mLevels[i]->disconnect(true);
            }
        }
        else
        {
            int errorMessage;
            if (gProfile->GetLiveConnectionStatus() != 0)
            {
                errorMessage = static_cast<int>(IDS_CONNECTION_LOST_LIVE);
            }
            else
            {
                switch (gApp.GetDisconnectReason())
                {
                case DisconnectPacket::eDisconnectReason_Quit:
                    errorMessage = IDS_DISCONNECTED_SERVER_QUIT;
                    break;
                case DisconnectPacket::eDisconnectReason_NoFlying:
                    errorMessage = static_cast<int>(IDS_DISCONNECTED_FLYING);
                    break;
                case DisconnectPacket::eDisconnectReason_Kicked:
                    errorMessage = static_cast<int>(IDS_DISCONNECTED_KICKED);
                    break;
                case DisconnectPacket::eDisconnectReason_GameFull:
                    errorMessage = static_cast<int>(IDS_DISCONNECTED_SERVER_FULL);
                    errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
                    break;
                case DisconnectPacket::eDisconnectReason_HostOutOfDate:
                    errorMessage = static_cast<int>(IDS_DISCONNECTED_SERVER_OLD);
                    errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
                    break;
                case DisconnectPacket::eDisconnectReason_ClientOutOfDate:
                    errorMessage = static_cast<int>(IDS_DISCONNECTED_CLIENT_OLD);
                    errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
                    break;
                case DisconnectPacket::eDisconnectReason_ContentRestrictedSingleLocal:
                    errorMessage = static_cast<int>(STRING_JOIN_LOCAL_PLAYER_CONTENT_RESTRICTED);
                    errorTitle = IDS_CONNECTION_FAILED;
                    break;
                case DisconnectPacket::eDisconnectReason_ContentRestrictedAllLocal:
                    errorMessage = STRING_JOIN_CONTENT_PRIVILEGE_TOO_RESTRICTIVE;
                    errorTitle = IDS_CONNECTION_FAILED;
                    break;
                case DisconnectPacket::eDisconnectReason_NoFriendsInGame:
                    errorMessage = IDS_DISCONNECTED_NO_FRIENDS_IN_GAME;
                    errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
                    break;
                case DisconnectPacket::eDisconnectReason_BannedByHost:
                    errorMessage = IDS_DISCONNECTED_BANNED;
                    errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
                    break;
                case DisconnectPacket::eDisconnectReason_FriendsOfHostOnly:
                    errorMessage = static_cast<int>(STRING_JOIN_FRIENDS_OF_HOST_ONLY);
                    errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
                    break;
                case DisconnectPacket::eDisconnectReason_MiiInUse:
                    errorMessage = STRING_MII_ALREADY_IN_USE;
                    errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
                    break;
                case DisconnectPacket::eDisconnectReason_CannotJoinGame:
                    errorMessage = static_cast<int>(STRING_CANNOT_JOIN_GAME);
                    errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
                    break;
                case DisconnectPacket::eDisconnectReason_NoSuitableGame:
                    errorMessage = static_cast<int>(STRING_NO_SUITABLE_GAME_TO_JOIN);
                    errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
                    break;
#ifdef MINECRAFT_PC
                case DisconnectPacket::eDisconnectReason_PcMismatchedContent:
                    errorMessage = static_cast<int>(ePcString_MismatchedContent);
                    errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
                    break;
#endif
                default:
                    errorMessage = IDS_CONNECTION_LOST_SERVER;
                    break;
                }
            }
            if (gProfile->IsSignedIn(gProfile->GetPrimaryPad()))
                gConsoleUIController.RequestErrorMessage(errorTitle, errorMessage, parameters, 1,
                                                         gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
            for (int i = 0; i < 3; ++i)
            {
                if (minecraft->mLevels[i] != NULL)
                    minecraft->mLevels[i]->disconnect(false);
            }
            exitTitle = -1;
        }
        MinecraftServer::HaltServer(false);
        saveStats = false;
        CGameNetworkManager::sInstance.LeaveGame(false);
    }
    else if (context != NULL && gProfile->IsSignedIn(gProfile->GetPrimaryPad()))
    {
        int errorMessage;
        switch (gApp.GetDisconnectReason())
        {
        case DisconnectPacket::eDisconnectReason_Quit:
            errorMessage = IDS_DISCONNECTED_SERVER_QUIT;
            break;
        case DisconnectPacket::eDisconnectReason_Kicked:
            errorMessage = static_cast<int>(IDS_DISCONNECTED_KICKED);
            break;
        case DisconnectPacket::eDisconnectReason_GameFull:
            errorMessage = static_cast<int>(IDS_DISCONNECTED_SERVER_FULL);
            errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
            break;
        case DisconnectPacket::eDisconnectReason_HostOutOfDate:
            errorMessage = static_cast<int>(IDS_DISCONNECTED_SERVER_OLD);
            errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
            break;
        case DisconnectPacket::eDisconnectReason_ClientOutOfDate:
            errorMessage = static_cast<int>(IDS_DISCONNECTED_CLIENT_OLD);
            errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
            break;
        case DisconnectPacket::eDisconnectReason_NoMultiplayerPrivilegeJoin:
            errorMessage = static_cast<int>(IDS_NO_MULTIPLAYER_PRIVILEGE_JOIN_TEXT);
            break;
        case DisconnectPacket::eDisconnectReason_ContentRestrictedSingleLocal:
            errorMessage = static_cast<int>(STRING_JOIN_LOCAL_PLAYER_CONTENT_RESTRICTED);
            errorTitle = IDS_CONNECTION_FAILED;
            break;
        case DisconnectPacket::eDisconnectReason_ContentRestrictedAllLocal:
            errorMessage = STRING_JOIN_CONTENT_PRIVILEGE_TOO_RESTRICTIVE;
            errorTitle = IDS_CONNECTION_FAILED;
            break;
        case DisconnectPacket::eDisconnectReason_MiiInUse:
            errorMessage = STRING_MII_ALREADY_IN_USE;
            break;
        case DisconnectPacket::eDisconnectReason_CannotJoinGame:
            errorMessage = static_cast<int>(STRING_CANNOT_JOIN_GAME);
            errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
            break;
        case DisconnectPacket::eDisconnectReason_NoSuitableGame:
            errorMessage = static_cast<int>(STRING_NO_SUITABLE_GAME_TO_JOIN);
            errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
            break;
#ifdef MINECRAFT_PC
        case DisconnectPacket::eDisconnectReason_PcMismatchedContent:
            errorMessage = static_cast<int>(ePcString_MismatchedContent);
            errorTitle = static_cast<int>(IDS_CANTJOIN_TITLE);
            break;
#endif
        default:
            errorMessage = IDS_DISCONNECTED;
            break;
        }
        gConsoleUIController.RequestErrorMessage(
            errorTitle, errorMessage, parameters, 1, 0xff,
            CGameNetworkManager::sInstance.IsHandlingBootInvite() ? NetworkErrorHandledDuringInvite : NULL, NULL, NULL,
            true);
        exitTitle = -1;
    }

    while (CGameNetworkManager::sInstance.IsNetworkThreadRunning())
        Sleep(1);
    minecraft->setLevel(NULL, exitTitle, boost::shared_ptr<Player>(), saveStats, false);
    telemetryManagerInstance->Flush();
    gApp.mGameRuleManager.unloadCurrentGameRules();
    MinecraftServer::s_haltRequested = false;
    MinecraftServer::s_saveOnExitAnswered = false;
    minecraft->mProgressRenderer->progressStart(IDS_EXITING_GAME);
    while (CGameNetworkManager::sInstance.IsInSession())
        Sleep(1);
    gApp.SetChangingSessionType(false);
    gApp.SetReallyChangingSessionType(false);
    gApp.SetGameHostOption(eGameHostOption_MiniGame, 0);
    Minecraft::GetInstance()->SetupMiniGameInstance(*MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true), 0);
}

int UIScene_PauseMenu::ExitWorldThreadProc(void* context)
{
    AABB::UseDefaultThreadStorage();
    Vec3::UseDefaultThreadStorage();
    Compression::UseDefaultThreadStorage();
    _ExitWorld(context);
    return 0;
}

void UIScene_PauseMenu::OpenExitGameDialog(int player, void* callbackData,
                                           int (*callback)(void*, int, C4JStorage::EMessageResult),
                                           int (*saveCallback)(void*, int, C4JStorage::EMessageResult),
                                           EOpenExitGameDialog action)
{
    if (action != EOpenExitGameDialog_JoinInvite && player != gProfile->GetPrimaryPad())
    {
        telemetryManagerInstance->RecordLevelExit(player, eSen_LevelExitStatus_Completed);
        gApp.SetAction(player, eXuiAction_RemoveLocalPlayer, NULL);
        return;
    }
    int parameters[3] = {static_cast<int>(IDS_CONFIRM_CANCEL), static_cast<int>(IDS_CONFIRM_OK), IDS_EXIT_GAME_NO_SAVE};
    int message = 0xd5e33637;
    unsigned int parameterCount = 2;
    int (*selectedCallback)(void*, int, C4JStorage::EMessageResult) = callback;
    if (!gProfile->IsFullVersion())
    {
        gConsoleUIController.ReduceTrialTimerValue();
        message = IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST;
    }
    else if (CGameNetworkManager::sInstance.IsHost())
    {
        if (gApp.GetGameHostOption(eGameHostOption_MiniGame) == 0)
        {
            if (gStorage->GetSaveDisabled() || gApp.HasNoSpaceForSaveQuota())
            {
                message = IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST;
            }
            else
            {
                message = CGameNetworkManager::sInstance.GetPlayerCount() > 1
                              ? static_cast<int>(IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE)
                              : IDS_CONFIRM_EXIT_GAME;
                parameters[1] = IDS_EXIT_GAME_SAVE;
                parameterCount = 3;
                selectedCallback = saveCallback;
            }
        }
        else if (CGameNetworkManager::sInstance.GetPlayerCount() > 1)
        {
            message = IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT;
        }
    }
    if (action == EOpenExitGameDialog_JoinInvite)
        message = IDS_CONFIRM_LEAVE_VIA_INVITE;
    gConsoleUIController.RequestAlertMessage(IDS_EXIT_GAME, message, parameters, parameterCount, player,
                                             selectedCallback, callbackData, NULL, true);
}

int UIScene_PauseMenu::WarningTrialTexturePackReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene* scene = gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context)));
    UIScene_PauseMenu* pauseMenu = scene != NULL ? dynamic_cast<UIScene_PauseMenu*>(scene) : NULL;
    if (result == C4JStorage::EMessageResult_Accept && gProfile->IsSignedIn(pad))
    {
        if (gProfile->IsSignedInLive(pad))
        {
            DLCPack* pack = static_cast<DLCTexturePack*>(
                                Minecraft::GetInstance()->mTexturePackRepository->getSelected())
                                ->getDLCInfoParentPack();
            if (!gApp.GetCommerceInstance()->PurchaseDLCByPackID(pack->GetPackId()))
            {
                int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
                gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_DLC_ALREADY_INSTALLED_TITLE),
                                                         static_cast<int>(IDS_DLC_ALREADY_INSTALLED_TEXT), parameters,
                                                         1, pad, NULL, NULL, NULL, true);
            }
        }
        else
        {
            int parameters[1] = {static_cast<int>(IDS_CONFIRM_OK)};
            gConsoleUIController.RequestErrorMessage(static_cast<int>(IDS_PRO_NOTONLINE_TITLE),
                                                     static_cast<int>(0xc34c0b77U), parameters, 1, pad, NULL, NULL,
                                                     NULL, true);
        }
    }
    return 0;
}

int UIScene_PauseMenu::ExitGameDialogReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene_PauseMenu* pauseMenu = dynamic_cast<UIScene_PauseMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context))));
    if (result == C4JStorage::EMessageResult_Decline)
    {
        if (pauseMenu != NULL && pauseMenu->getSceneType() == EUIScene_PauseMenu)
            pauseMenu->SetIgnoreInput(true);
        gApp.SetAction(pad, eXuiAction_ExitWorld, NULL);
    }
    return 0;
}

int UIScene_PauseMenu::ExitGameAndSaveReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene* scene = gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context)));
    UIScene_PauseMenu* pauseMenu = scene != NULL ? dynamic_cast<UIScene_PauseMenu*>(scene) : NULL;
    if (result == C4JStorage::EMessageResult_Decline)
    {
        if (pauseMenu != NULL && pauseMenu->getSceneType() == EUIScene_PauseMenu)
            pauseMenu->SetIgnoreInput(true);
        gMinecraftServer->saveOnExit = true;
        MinecraftServer::s_saveOnExitAnswered = true;
        gApp.SetAction(pad, eXuiAction_ExitWorld, NULL);
    }
    else if (gConsoleUIController.IsPauseMenuDisplayed(gProfile->GetPrimaryPad()))
    {
        int parameters[3] = {IDS_CONFIRM_CANCEL, IDS_EXIT_GAME_SAVE, IDS_EXIT_GAME_NO_SAVE};
        if (CGameNetworkManager::sInstance.GetPlayerCount() > 1)
        {
            gConsoleUIController.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE,
                                                     parameters, 3, gProfile->GetPrimaryPad(),
                                                     ExitGameSaveDialogReturned, context, NULL, true);
        }
        else
        {
            gConsoleUIController.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, parameters, 3,
                                                     gProfile->GetPrimaryPad(), ExitGameSaveDialogReturned, context,
                                                     NULL, true);
        }
    }
    return 0;
}

int UIScene_PauseMenu::ExitGameDeclineSaveReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene* scene = gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context)));
    UIScene_PauseMenu* pauseMenu = scene != NULL ? dynamic_cast<UIScene_PauseMenu*>(scene) : NULL;
    if (result == C4JStorage::EMessageResult_Decline)
    {
        if (pauseMenu != NULL && pauseMenu->getSceneType() == EUIScene_PauseMenu)
            pauseMenu->SetIgnoreInput(true);
        gMinecraftServer->saveOnExit = false;
        MinecraftServer::s_saveOnExitAnswered = true;
        gApp.SetAction(pad, eXuiAction_ExitWorld, NULL);
    }
    else if (gConsoleUIController.IsPauseMenuDisplayed(gProfile->GetPrimaryPad()))
    {
        int parameters[3] = {IDS_CONFIRM_CANCEL, IDS_EXIT_GAME_SAVE, IDS_EXIT_GAME_NO_SAVE};
        if (CGameNetworkManager::sInstance.GetPlayerCount() > 1)
        {
            gConsoleUIController.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE,
                                                     parameters, 3, gProfile->GetPrimaryPad(),
                                                     ExitGameSaveDialogReturned, context, NULL, true);
        }
        else
        {
            gConsoleUIController.RequestAlertMessage(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, parameters, 3,
                                                     gProfile->GetPrimaryPad(), ExitGameSaveDialogReturned, context,
                                                     NULL, true);
        }
    }
    return 0;
}

int UIScene_PauseMenu::ExitGameSaveDialogReturned(void* context, int pad, C4JStorage::EMessageResult result)
{
    UIScene_PauseMenu* pauseMenu = dynamic_cast<UIScene_PauseMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context))));
    if (result != C4JStorage::EMessageResult_Decline)
    {
        if (result == C4JStorage::EMessageResult_ThirdOption)
        {
            int parameters[2] = {IDS_CONFIRM_CANCEL, static_cast<int>(IDS_CONFIRM_OK)};
            gConsoleUIController.RequestAlertMessage(
                static_cast<int>(IDS_TITLE_DECLINE_SAVE_GAME), static_cast<int>(IDS_CONFIRM_DECLINE_SAVE_GAME),
                parameters, 2, gProfile->GetPrimaryPad(), ExitGameDeclineSaveReturned, context, NULL, true);
        }
        return 0;
    }
    bool defaultPack = Minecraft::GetInstance()->mTexturePackRepository->getSelected()
                       == TexturePackRepository::defaultTexturePack;
    if (!defaultPack
        && !static_cast<DLCTexturePack*>(Minecraft::GetInstance()->mTexturePackRepository->getSelected())
                ->getDLCInfoParentPack()
                ->hasPurchasedPack())
    {
        int parameters[2] = {static_cast<int>(IDS_CONFIRM_OK), IDS_CONFIRM_CANCEL};
        gConsoleUIController.RequestAlertMessage(0x3bb3096e, 0x0d52c2e3, parameters, 2, gProfile->GetPrimaryPad(),
                                                 WarningTrialTexturePackReturned, context, NULL, true);
        return 0;
    }
    bool saveExists;
    gStorage->DoesSaveExist(&saveExists);
    if (saveExists)
    {
        int parameters[2] = {IDS_CONFIRM_CANCEL, static_cast<int>(IDS_CONFIRM_OK)};
        gConsoleUIController.RequestAlertMessage(IDS_TITLE_SAVE_GAME, static_cast<int>(IDS_CONFIRM_SAVE_GAME),
                                                 parameters, 2, gProfile->GetPrimaryPad(), ExitGameAndSaveReturned,
                                                 context, NULL, true);
    }
    else
    {
        MinecraftServer::s_saveOnExitAnswered = true;
        gMinecraftServer->saveOnExit = true;
        if (pauseMenu != NULL && pauseMenu->getSceneType() == EUIScene_PauseMenu)
            pauseMenu->SetIgnoreInput(true);
        gApp.SetAction(pad, eXuiAction_ExitWorld, NULL);
    }
    return 0;
}

int UIScene_PauseMenu::SaveGameDialogReturned(void*, int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Decline)
        gApp.SetAction(pad, eXuiAction_SaveGame, NULL);
    return 0;
}

int UIScene_PauseMenu::EnableAutosaveDialogReturned(void*, int pad, C4JStorage::EMessageResult result)
{
    unsigned int disableSaving = 1;
    if (result == C4JStorage::EMessageResult_Decline)
        disableSaving = 0;
    gApp.SetGameHostOption(eGameHostOption_DisableSaving, disableSaving);
    gStorage->SetSaveDisabled(false);
    gApp.SetAction(pad, eXuiAction_SaveGame, NULL);
    return 0;
}

int UIScene_PauseMenu::DisableAutosaveDialogReturned(void*, int pad, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Decline)
    {
        gApp.SetGameHostOption(eGameHostOption_DisableSaving, 1);
        gStorage->SetSaveDisabled(false);
        gApp.SetAction(pad, eXuiAction_SaveGame, NULL);
    }
    return 0;
}

void UIScene_PauseMenu::HandleDLCMountingComplete()
{
}
void UIScene_PauseMenu::HandleDLCInstalled()
{
    gApp.StartInstallDLCProcess(m_playerId);
}

int UIScene_PauseMenu::UnlockFullSaveReturned(void*, int, C4JStorage::EMessageResult result)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (result == C4JStorage::EMessageResult_Accept && gProfile->IsSignedInLive(minecraft->mPlayer.get()->mUserIndex))
        gProfile->DisplayFullVersionPurchase(false, minecraft->mPlayer.get()->mUserIndex, 1);
    return 0;
}

void UIScene_PauseMenu::PerformActionSaveGame()
{
    int unlockParameters[2];
    int texturePackParameters[2];
    int saveParameters[2];
    if (!gProfile->IsFullVersion())
    {
        if (gProfile->IsSignedInLive(m_playerId))
        {
            unlockParameters[0] = (int)IDS_CONFIRM_OK;
            unlockParameters[1] = IDS_CONFIRM_CANCEL;
            gConsoleUIController.RequestAlertMessage((int)IDS_UNLOCK_TITLE, (int)0xc75616c3, unlockParameters, 2,
                                                     m_playerId, UnlockFullSaveReturned,
                                                     reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
        }
        return;
    }

    TexturePackRepository* repository = Minecraft::GetInstance()->mTexturePackRepository;
    bool isDefaultPack = repository->getSelected() == TexturePackRepository::defaultTexturePack;
    if (!isDefaultPack)
    {
        DLCPack* pack = static_cast<DLCTexturePack*>(Minecraft::GetInstance()->mTexturePackRepository->getSelected())
                            ->getDLCInfoParentPack();
        if (!pack->hasPurchasedPack())
        {
            texturePackParameters[0] = (int)IDS_CONFIRM_OK;
            texturePackParameters[1] = IDS_CONFIRM_CANCEL;
            gConsoleUIController.RequestAlertMessage(0x3bb3096e, 0x0d52c2e3, texturePackParameters, 2, m_playerId,
                                                     WarningTrialTexturePackReturned,
                                                     reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
            return;
        }
        m_texturePackWarningPending = false;
    }

    bool saveExists;
    gStorage->DoesSaveExist(&saveExists);
    saveParameters[0] = IDS_CONFIRM_CANCEL;
    saveParameters[1] = (int)IDS_CONFIRM_OK;
    if (saveExists)
    {
        gConsoleUIController.RequestAlertMessage(IDS_TITLE_SAVE_GAME, (int)IDS_CONFIRM_SAVE_GAME, saveParameters, 2,
                                                 m_playerId, SaveGameDialogReturned,
                                                 reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
    }
    else
    {
        gApp.SetAction(m_playerId, eXuiAction_SaveGame, NULL);
    }
}

unsigned int UIScene_PauseMenu::SaveGame_SignInReturned(void* context, bool signedIn, int, int)
{
    UIScene* scene = gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context)));
    UIScene_PauseMenu* pauseMenu = static_cast<UIScene_PauseMenu*>(scene);
    if (pauseMenu != NULL && pauseMenu->getSceneType() == EUIScene_PauseMenu)
        pauseMenu->SetIgnoreInput(false);
    if (signedIn)
    {
        if (pauseMenu != NULL)
            pauseMenu->PerformActionSaveGame();
    }
    return 0;
}
