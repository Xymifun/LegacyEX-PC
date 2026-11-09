#include "game/ui/UIScene_ConnectingProgress.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FullScreenProgress.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/GameSettings.hpp"
#include "vendor/_ConnectionProgressParams.hpp"

UIScene_ConnectingProgress::UIScene_ConnectingProgress(int type, void* data, UILayer* layer) : UIScene(type, layer)
{
    _ConnectionProgressParams* params = static_cast<_ConnectionProgressParams*>(data);

    initialiseMovie();
    m_tipLabel.init(UIString(L""));
    m_tipLabel.setVisible(false);
    m_miniGameId = params->m_miniGameId;

    if (gApp.GetGameHostOption(eGameHostOption_MiniGame))
    {
        SetLoadingScreenType(1);
        setMiniGameLogoTexture();
        m_miniGameLogoSet = true;
    }
    else
    {
        m_miniGameLogoSet = false;
        SetLoadingScreenType(0);
    }
    layer->addComponent(type, EUIScene_Panorama, NULL);
    layer->addComponent(type, EUIScene_Logo, NULL);
    m_progressControl.showBar(false);
    m_progressControl.setVisible(false);

    if (params->m_stringId != -1)
        m_titleLabel.init(UIString(L10N::GetString(params->m_stringId)));
    else
        m_titleLabel.init(UIString(L""));

    m_progressControl.init(UIString(L""), 0, 0, 100, 0);
    m_confirmationButton.init(UIString(L10N::GetString(IDS_CONFIRM_OK)), 0);
    ShowConfirmationButton(false);

    m_showTooltips = params->m_showTooltips;
    m_setFailTimer = params->m_setFailTimer;
    m_timerTime = params->m_timerTime;
    m_cancelFunc = params->m_cancelFunc;
    m_cancelFuncParam = params->m_cancelFuncParam;
    m_removeLocalPlayer = false;
    m_showingButton = false;
    m_cancelled = false;
    m_movieLoaded = false;
    delete params;
}

UIScene_ConnectingProgress::~UIScene_ConnectingProgress()
{
    m_layer->removeComponent(EUIScene_Panorama);
    m_layer->removeComponent(EUIScene_Logo);
    if (m_showingButton)
    {
        tryRemovePlayer();
        if (m_removeLocalPlayer)
            removeLocalPlayer();
    }
}

void UIScene_ConnectingProgress::SetLoadingScreenType(int type)
{
    static_cast<FJ_FullScreenProgress*>(m_movie->getRootNode()->getFuiNode())->SetLoadingScreenType(type);
}

void UIScene_ConnectingProgress::setMiniGameLogoTexture()
{
    const MiniGameDef* game = Minecraft::GetInstance()->GetMiniGame();
    std::wstring icon = (game->GetId() == eMiniGameId_Any || game->GetId() == eMiniGameId_Lobby)
                            ? Minecraft::GetInstance()->GetFirstMiniGame()->getIcon()
                            : game->getIcon();

    if (gApp.hasArchiveFile(icon, false))
    {
        arrayWithLength<unsigned char> file = gApp.getArchiveFile(icon, false);
        if (file.length != 0 && file.data != NULL)
            registerSubstitutionTexture(icon, file.data, file.length, true);
        m_logoControl.setTextureName(icon, false);
    }
    else
    {
        m_logoControl.setTextureName(L"", false);
    }
}

void UIScene_ConnectingProgress::ShowConfirmationButton(bool show)
{
    static_cast<FJ_FullScreenProgress*>(m_movie->getRootNode()->getFuiNode())->ShowConfirmationButton(show);
}

unsigned int UIScene_ConnectingProgress::mapElementsAndNames()
{
    fuiRenderNode* currentRoot = m_movie->getRootNode();
    {
        m_progressControl.setupControl(this, currentRoot, std::string("ProgressBar"));
        m_controls.push_back(&m_progressControl);
    }
    {
        m_titleLabel.setupControl(this, currentRoot, std::string("Title"));
        m_controls.push_back(&m_titleLabel);
    }
    {
        m_tipLabel.setupControl(this, currentRoot, std::string("Tip"));
        m_controls.push_back(&m_tipLabel);
    }
    {
        m_gameModeIconControl.setupControl(this, currentRoot, std::string("GMIcon"));
        m_controls.push_back(&m_gameModeIconControl);
    }
    {
        fuiRenderNode* lastRoot = currentRoot;
        currentRoot = m_gameModeIconControl.getFuiRenderNode();
        {
            m_logoControl.setupControl(this, currentRoot, std::string("MG_Logo"));
            m_controls.push_back(&m_logoControl);
        }
        currentRoot = lastRoot;
    }
    {
        m_confirmationButton.setupControl(this, currentRoot, std::string("Confirm"));
        m_controls.push_back(&m_confirmationButton);
    }
    {
        m_timerControl.setupControl(this, currentRoot, std::string("Timer"));
        m_controls.push_back(&m_timerControl);
    }
    return 1;
}

void UIScene_ConnectingProgress::tryRemovePlayer()
{
    m_showingButton = false;
    if (m_playerId != gProfile->GetPrimaryPad() && CGameNetworkManager::sInstance.IsInSession())
    {
        m_removeLocalPlayer = true;
        return;
    }
    gConsoleUIController.NavigateToHomeMenu(false);
}

void UIScene_ConnectingProgress::removeLocalPlayer()
{
    m_removeLocalPlayer = false;
    Minecraft::GetInstance()->removeLocalPlayerIdx(m_playerId);
    gProfile->SignOutUser(m_playerId);
}

void UIScene_ConnectingProgress::tick()
{
    UIScene::tick();
    if (m_removeLocalPlayer)
        removeLocalPlayer();

    if (!m_miniGameLogoSet && m_movie != NULL && Minecraft::GetInstance()->GetMiniGame()->GetId() != eMiniGameId_Any)
    {
        SetLoadingScreenType(1);
        setMiniGameLogoTexture();
        m_miniGameLogoSet = true;
        m_miniGameId = Minecraft::GetInstance()->GetMiniGame()->GetId();
        gApp.SetFirstMiniGameType(m_miniGameId);
    }
}

void UIScene_ConnectingProgress::handleReload()
{
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame))
    {
        SetLoadingScreenType(1);
        setMiniGameLogoTexture();
        m_miniGameLogoSet = true;
        return;
    }
    m_miniGameLogoSet = false;
    SetLoadingScreenType(0);
}

void UIScene_ConnectingProgress::updateTooltips()
{
    int cancelTooltip = m_showTooltips ? static_cast<int>(IDS_TOOLTIPS_CANCEL_JOIN) : -1;
    gConsoleUIController.SetTooltips(m_playerId, -1, cancelTooltip, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, false, false);
}

void UIScene_ConnectingProgress::handleGainFocus(bool navigatedBack)
{
    if (!navigatedBack && m_setFailTimer)
        addTimer(0, m_timerTime);
}

void UIScene_ConnectingProgress::handleLoseFocus()
{
    int first = 0;
    m_timers[first];
    System::processTimeInMilliSecs();
    int second = 0;
    m_timers[second];
    killTimer(0);
}

void UIScene_ConnectingProgress::handleTimerComplete(int)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (!minecraft->mConnectionFailed[m_playerId] && CGameNetworkManager::sInstance.IsInSession())
        return;

    int message;
    switch (minecraft->mDisconnectReasons[m_playerId])
    {
    case DisconnectPacket::eDisconnectReason_LoginTooLong:
        message = IDS_DISCONNECTED_LOGIN_TOO_LONG;
        break;
    case DisconnectPacket::eDisconnectReason_GameFull:
        message = IDS_DISCONNECTED_SERVER_FULL;
        break;
    case DisconnectPacket::eDisconnectReason_HostOutOfDate:
        message = IDS_DISCONNECTED_SERVER_OLD;
        break;
    case DisconnectPacket::eDisconnectReason_ClientOutOfDate:
        message = IDS_DISCONNECTED_CLIENT_OLD;
        break;
    case DisconnectPacket::eDisconnectReason_Kicked:
        message = IDS_DISCONNECTED_KICKED;
        break;
    case DisconnectPacket::eDisconnectReason_ContentRestrictedSingleLocal:
        message = STRING_JOIN_LOCAL_PLAYER_CONTENT_RESTRICTED;
        break;
    case DisconnectPacket::eDisconnectReason_ContentRestrictedAllLocal:
        message = STRING_JOIN_CONTENT_PRIVILEGE_TOO_RESTRICTIVE;
        break;
    case DisconnectPacket::eDisconnectReason_NoFlying:
        message = IDS_DISCONNECTED_FLYING;
        break;
    case DisconnectPacket::eDisconnectReason_Quit:
        message = IDS_DISCONNECTED_SERVER_QUIT;
        break;
    case DisconnectPacket::eDisconnectReason_MiiInUse:
        message = STRING_MII_ALREADY_IN_USE;
        break;
    case DisconnectPacket::eDisconnectReason_CannotJoinGame:
        message = STRING_CANNOT_JOIN_GAME;
        break;
    case DisconnectPacket::eDisconnectReason_NoSuitableGame:
        message = STRING_NO_SUITABLE_GAME_TO_JOIN;
        break;
#ifdef MINECRAFT_PC
    case DisconnectPacket::eDisconnectReason_PcMismatchedContent:
        message = ePcString_MismatchedContent;
        break;
#endif
    default:
        message = IDS_CONNECTION_LOST_SERVER;
        break;
    }

    if (m_playerId != gProfile->GetPrimaryPad() && CGameNetworkManager::sInstance.IsInSession())
    {
        ShowConfirmationButton(true);
        m_showingButton = true;
        m_titleLabel.setLabel(UIString(L10N::GetString(IDS_CONNECTION_FAILED)), false, false);
        m_progressControl.setLabel(UIString(L10N::GetString(message)), false, false);
        m_progressControl.setVisible(true);
        m_timerControl.setVisible(false);
        return;
    }

    if (m_cancelled)
    {
        gApp.SetDisconnectReason(static_cast<DisconnectPacket::eDisconnectReason>(0x21));
    }
    else
    {
        int parameters[1] = {IDS_CONFIRM_OK};
        gConsoleUIController.RequestErrorMessage(IDS_CONNECTION_FAILED, message, parameters, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
    }
    gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_ExitWorld, reinterpret_cast<void*>(1));
}

std::wstring UIScene_ConnectingProgress::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1 && !m_layer->IsFullscreenGroup())
        return L"FullscreenProgressSplit";
    return L"FullscreenProgress";
}

void UIScene_ConnectingProgress::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                             bool& handled)
{
    if (m_showTooltips)
    {
        gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
        switch (action)
        {
        case EControllerActions_MenuUp:
        case EControllerActions_MenuDown:
        case EControllerActions_MenuOk:
            break;
        case EControllerActions_MenuCancel:
            if (m_cancelFunc != NULL)
                m_cancelFunc(m_cancelFuncParam);
            handled = true;
            m_cancelled = true;
            return;
        default:
            return;
        }
    }
    else
    {
        if (!m_showingButton || action != EControllerActions_MenuOk)
            return;
    }

    if (pressed)
        sendInputToMovie(action, repeat, pressed, released);
}

void UIScene_ConnectingProgress::handlePress(int controlId, int)
{
    if (controlId == 0 && m_showingButton)
        tryRemovePlayer();
}

int UIScene_ConnectingProgress::getSceneType()
{
    return EUIScene_ConnectingProgress;
}
