#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <vector>

#include "game/client/render/ProgressRenderer.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/FJ_FullScreenProgress.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/Tooltips.hpp"
#include "game/ui/UIGroup.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "game/core/GameSettings.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

UIScene_FullscreenProgress::UIScene_FullscreenProgress(int type, void* data, UILayer* layer)
    : UIScene(type, layer),
      m_titleText(),
      m_statusText()
{
    initialiseMovie();
    m_movieLoaded = false;
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0)
    {
        m_miniGame = true;
        SetLoadingScreenType(1);
        setMiniGameLogoTexture();
    }
    else
    {
        m_miniGame = false;
        SetLoadingScreenType(0);
    }
    layer->addComponent(type, EUIScene_Panorama, NULL);
    layer->addComponent(type, EUIScene_Logo, NULL);
    layer->showComponent(type, EUIScene_Logo, true);
    layer->showComponent(type, EUIScene_MenuBackground, false);
    m_timerControl.setVisible(false);
    m_titleText = L"";
    m_statusText = L"";
    m_progress = 0;
    m_confirmationButton.init(UIString(L10N::GetString(IDS_CONFIRM_OK)), 0);
    ShowConfirmationButton(false);
    m_wasCancelled = false;

    _LoadingInputParams* params = static_cast<_LoadingInputParams*>(data);
    m_completionData = static_cast<FullscreenProgressState*>(params->m_completionData);
    m_cancelTooltip = params->m_cancelText;
    m_cancelFunc = params->m_cancelFunc;
    m_completeFunc = params->m_completeFunc;
    m_completeFuncParam = params->m_completeFuncParam;
    m_miniGameId = params->m_miniGameId;
    m_cancelFuncParam = params->m_cancelFuncParam;
    m_waitForThreadToDelete = params->m_waitForThreadToDelete;
    m_playerId = m_completionData->playerId;
    Minecraft* minecraft = Minecraft::GetInstance();
    minecraft->mProgressRenderer->progressStart(-1);
    minecraft->mProgressRenderer->progressStage(-1);
    m_progressControl.init(UIString(L""), 0, 0, 100, 0);
    SetTipText(true);
    addTimer(1, 7000);
    m_titleLabel.init(UIString(L""));
    m_tipLabel.setVisible(m_completionData->showTip);
    m_thread = new C4JThreadImpl(params->m_threadProc, params->m_threadParam, "FullscreenProgress", 0);
    m_thread->SetProcessor(0);
    m_confirmationVisible = false;
    m_thread->Run();
    m_threadStarted = true;
    m_confirmationButtonHidden = false;
    gConsoleUIController.TouchBoxRebuild(this);
    if (m_completionData->closeMode == 3)
    {
        m_layer->closeAllScenes();
    }
    delete params;
}

UIScene_FullscreenProgress::~UIScene_FullscreenProgress()
{
    m_layer->removeComponent(EUIScene_Panorama);
    m_layer->removeComponent(EUIScene_Logo);
    if (m_thread->isRunning() == 0)
    {
        delete m_thread;
    }
    else
    {
        m_thread->SetDeleteOnExit(true);
    }
    delete m_completionData;
}

void UIScene_FullscreenProgress::SetLoadingScreenType(int type)
{
    static_cast<FJ_FullScreenProgress*>(m_movie->getRootNode()->getFuiNode())->SetLoadingScreenType(type);
}

void UIScene_FullscreenProgress::setMiniGameLogoTexture()
{
    const MiniGameDef* game = Minecraft::GetInstance()->GetMiniGame();
    std::wstring icon = (game->GetId() == 15 || game->GetId() == 0)
                            ? Minecraft::GetInstance()->GetFirstMiniGame()->getIcon()
                            : game->getIcon();
    if (gApp.hasArchiveFile(icon, false))
    {
        arrayWithLength<unsigned char> file = gApp.getArchiveFile(icon, false);
        if (file.length != 0 && file.data != NULL)
        {
            registerSubstitutionTexture(icon, file.data, file.length, true);
        }
        m_logoControl.setTextureName(icon, false);
    }
    else
    {
        m_logoControl.setTextureName(std::wstring(L""), false);
    }
}

void UIScene_FullscreenProgress::ShowConfirmationButton(bool show)
{
    static_cast<FJ_FullScreenProgress*>(m_movie->getRootNode()->getFuiNode())->ShowConfirmationButton(show);
}

void UIScene_FullscreenProgress::SetTipText(bool initial)
{
    std::wstring text;
    EMiniGameId id = m_miniGameId;
    if (id != eMiniGameId_Any)
    {
        if (id == eMiniGameId_Lobby)
        {
            id = gApp.GetFirstMiniGameType();
        }
        unsigned int tip = gApp.GetNextTip(true, id);
        text = gApp.FormatHTMLString(m_playerId, std::wstring(L10N::GetString(static_cast<int>(tip))), -1);
    }
    else
    {
        unsigned int tip = gApp.GetNextTip(false, eMiniGameId_Lobby);
        text = gApp.FormatHTMLString(m_playerId, std::wstring(L10N::GetString(static_cast<int>(tip))), -1);
    }
    unsigned int colour = gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White);
    wchar_t formatted[64];
    swprintf(formatted, 64, L"<font color=\"#%08x\"><p align=center>", colour);
    text = formatted + text + L"</p>";
    if (initial)
    {
        m_tipLabel.init(UIString(text));
    }
    else
    {
        m_tipLabel.setLabel(UIString(text), false, false);
    }
}

unsigned int UIScene_FullscreenProgress::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_progressControl.setupControl(this, currentNode, std::string("ProgressBar"));
        m_controls.push_back(&m_progressControl);
    }
    {
        m_titleLabel.setupControl(this, currentNode, std::string("Title"));
        m_controls.push_back(&m_titleLabel);
    }
    {
        m_tipLabel.setupControl(this, currentNode, std::string("Tip"));
        m_controls.push_back(&m_tipLabel);
    }
    {
        m_timerTitleControl.setupControl(this, currentNode, std::string("GMIcon"));
        m_controls.push_back(&m_timerTitleControl);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_timerTitleControl.getFuiRenderNode();
        {
            m_logoControl.setupControl(this, currentNode, std::string("MG_Logo"));
            m_controls.push_back(&m_logoControl);
        }
        currentNode = parentNode;
    }
    {
        m_confirmationButton.setupControl(this, currentNode, std::string("Confirm"));
        m_controls.push_back(&m_confirmationButton);
    }
    {
        m_timerControl.setupControl(this, currentNode, std::string("Timer"));
        m_controls.push_back(&m_timerControl);
    }
    return 1;
}

void UIScene_FullscreenProgress::handleReload()
{
    m_tipLabel.setVisible(m_completionData->showTip);
    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0)
    {
        m_miniGame = true;
        SetLoadingScreenType(1);
        setMiniGameLogoTexture();
        ShowConfirmationButton(false);
        SetTipText(false);
    }
    else
    {
        m_miniGame = false;
        SetLoadingScreenType(0);
        ShowConfirmationButton(false);
        SetTipText(false);
    }
}

void UIScene_FullscreenProgress::updateTooltips()
{
    Tooltips::ForceAllPlayerTooltipReset();
    int second = m_confirmationVisible ? static_cast<int>(IDS_TOOLTIPS_SELECT) : -1;
    int third = m_confirmationVisible ? -1 : m_cancelTooltip;
    gConsoleUIController.SetTooltips(m_playerId, second, third, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, m_layer->IsFullscreenGroup());
}

void UIScene_FullscreenProgress::handleDestroy()
{
    int code = m_thread->GetExitCode();
    unsigned int exitCode = *reinterpret_cast<unsigned int*>(&code);
    if (exitCode == STILL_ACTIVE && m_cancelFunc != NULL && !m_wasCancelled)
    {
        m_wasCancelled = true;
        m_cancelFunc(m_cancelFuncParam);
    }
}

void UIScene_FullscreenProgress::tick()
{
    UIScene::tick();
    Minecraft* minecraft = Minecraft::GetInstance();
    int percent = minecraft->mProgressRenderer->getCurrentPercent();
    if (percent < 0)
        percent = 0;
#ifdef MINECRAFT_PC
    if (percent < m_progress && minecraft->mProgressRenderer->getCurrentTitleString().compare(m_titleText) == 0
        && minecraft->mProgressRenderer->getCurrentStatusString().compare(m_statusText) == 0)
        percent = m_progress;
#endif
    if (percent != m_progress)
    {
        m_progress = percent;
        m_progressControl.setProgress(percent);
    }
    {
        std::wstring title = minecraft->mProgressRenderer->getCurrentTitleString();
        if (title.compare(m_titleText) != 0)
        {
            m_titleLabel.setLabel(UIString(title), false, false);
            m_titleText = title;
        }
    }
    {
        std::wstring status = minecraft->mProgressRenderer->getCurrentStatusString();
        if (status.compare(m_statusText) != 0)
        {
            m_progressControl.setLabel(UIString(status), false, false);
            m_statusText = status;
        }
    }
    int code = m_thread->GetExitCode();
    unsigned int exitCode = *reinterpret_cast<unsigned int*>(&code);
    if (exitCode != STILL_ACTIVE)
    {
        if (exitCode != 0)
        {
            if (exitCode != ERROR_CANCELLED)
            {
                CGameNetworkManager::sInstance.WiiU_ShowErrorViewer();
                if (gApp.GetXuiAction(m_playerId) != eXuiAction_ConnectionLostWait)
                {
                    int parameter = static_cast<int>(IDS_CONFIRM_OK);
                    int title = CGameNetworkManager::sInstance.CorrectErrorIDS(IDS_CONNECTION_FAILED);
                    int message = CGameNetworkManager::sInstance.CorrectErrorIDS(IDS_CONNECTION_LOST_SERVER);
                    gConsoleUIController.RequestErrorMessage(title, message, &parameter, 1, 0xff, NULL, NULL, NULL,
                                                             true);
                    gConsoleUIController.NavigateToHomeMenu(false);
                }
                gConsoleUIController.UpdatePlayerBasePositions();
            }
        }
        else
        {
            if (!m_wasCancelled && m_completionData->showConfirmation)
            {
                m_confirmationVisible = true;
                if (m_confirmationButtonHidden)
                {
                    gConsoleUIController.TouchBoxRebuild(this);
                }
                m_confirmationButtonHidden = !m_confirmationButton.m_visible;
                ShowConfirmationButton(true);
                updateTooltips();
            }
            else
            {
                if (m_wasCancelled)
                {
                    m_confirmationVisible = true;
                }
                if (m_completeFunc != NULL)
                {
                    m_completeFunc(m_completeFuncParam);
                }
                switch (m_completionData->closeMode)
                {
                case 6:
                    gConsoleUIController.UpdatePlayerBasePositions();
                    m_layer->m_group->NavigateBack(m_playerId, m_completionData->backScene,
                                                   static_cast<EUILayer>(m_layer->m_layerId));
                    gConsoleUIController.FixupMenuDisplayedAfterNavigateBack();
                    break;
                case 5:
                {
                    gConsoleUIController.SetIgnoreAutosaveMenuDisplayed(m_playerId, false);
                    Minecraft* mc = Minecraft::GetInstance();
                    if (mc->mGameModes[gProfile->GetPrimaryPad()] != NULL)
                    {
                        Tutorial* tutorial = mc->mGameModes[gProfile->GetPrimaryPad()]->getTutorial();
                        if (tutorial != NULL)
                        {
                            tutorial->showTutorialPopup(true);
                        }
                    }
                    gConsoleUIController.UpdatePlayerBasePositions();
                    m_layer->m_group->NavigateBack(m_playerId, m_completionData->backScene,
                                                   static_cast<EUILayer>(m_layer->m_layerId));
                    gConsoleUIController.FixupMenuDisplayedAfterNavigateBack();
                    break;
                }
                case 1:
                    gConsoleUIController.UpdatePlayerBasePositions();
                    navigateBack();
                    break;
                case 2:
                    gConsoleUIController.CloseUIScenes(m_completionData->playerId, false);
                    gConsoleUIController.UpdatePlayerBasePositions();
                    break;
                case 3:
                    gConsoleUIController.CloseAllPlayersScenes(false);
                    gConsoleUIController.UpdatePlayerBasePositions();
                    break;
                case 4:
                    gConsoleUIController.NavigateToHomeMenu(false);
                    gConsoleUIController.UpdatePlayerBasePositions();
                    break;
                }
            }
        }
    }

    if (!m_miniGame && m_movie != NULL)
    {
        EMiniGameId miniGameId = Minecraft::GetInstance()->GetMiniGameId();
        if (miniGameId != eMiniGameId_Any && miniGameId != eMiniGameId_Lobby)
        {
            SetLoadingScreenType(1);
            setMiniGameLogoTexture();
            m_miniGame = true;
            m_miniGameId = miniGameId;
            gApp.SetFirstMiniGameType(miniGameId);
            handleTimerComplete(1);
            killTimer(1);
            addTimer(1, 7000);
        }
    }
}

std::wstring UIScene_FullscreenProgress::getMoviePath()
{
    return L"FullscreenProgress";
}

void UIScene_FullscreenProgress::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                             bool& handled)
{
    switch (action)
    {
    case EControllerActions_MenuOk:
        if (pressed)
        {
            sendInputToMovie(action, repeat, pressed, released);
        }
        break;
    case EControllerActions_MenuB:
    case EControllerActions_MenuCancel:
        if (pressed && m_cancelFunc != NULL && !m_wasCancelled)
        {
            m_wasCancelled = true;
            m_cancelFunc(m_cancelFuncParam);
        }
        break;
    }
}

void UIScene_FullscreenProgress::handlePress(int controlId, int childId)
{
    if (!m_confirmationVisible || controlId != 0)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, 0, false, true, false);
    if (m_completeFunc != NULL)
        m_completeFunc(m_completeFuncParam);
    switch (m_completionData->closeMode)
    {
    case 1:
        gConsoleUIController.UpdatePlayerBasePositions();
        navigateBack();
        break;
    case 6:
        gConsoleUIController.UpdatePlayerBasePositions();
        m_layer->m_group->NavigateBack(m_playerId, m_completionData->backScene,
                                       static_cast<EUILayer>(m_layer->m_layerId));
        gConsoleUIController.FixupMenuDisplayedAfterNavigateBack();
        break;
    case 2:
        gConsoleUIController.CloseUIScenes(m_completionData->playerId, false);
        gConsoleUIController.UpdatePlayerBasePositions();
        break;
    case 3:
        gConsoleUIController.CloseAllPlayersScenes(false);
        gConsoleUIController.UpdatePlayerBasePositions();
        break;
    case 4:
        gConsoleUIController.NavigateToHomeMenu(false);
        gConsoleUIController.UpdatePlayerBasePositions();
        break;
    }
}

void UIScene_FullscreenProgress::handleTimerComplete(int timerId)
{
    if (timerId == 1)
        SetTipText(false);
}

void UIScene_FullscreenProgress::SetWasCancelled(bool cancelled)
{
    m_wasCancelled = cancelled;
}

bool UIScene_FullscreenProgress::isReadyToDelete()
{
    if (m_waitForThreadToDelete)
    {
        return !m_thread->isRunning();
    }
    return true;
}

int UIScene_FullscreenProgress::getSceneType()
{
    return EUIScene_FullscreenProgress;
}

unsigned long long UIScene_FullscreenProgress::getDefaultGtcButtons()
{
    return 0;
}
