#include "game/ui/UIComponent_PressStartToPlay.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/FJ_PressStartToPlay.hpp"
#include "game/ui/UILayer.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/StringConversion.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UIScene_AchievementsMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CProfile.hpp"

namespace
{
struct AvatarRequest
{
    AvatarRequest(UIComponent_PressStartToPlay* scene, int player, int achievement)
        : scene(scene),
          player(player),
          achievement(achievement)
    {
    }

    UIComponent_PressStartToPlay* scene;
    int player;
    int achievement;
};
}

UIComponent_PressStartToPlay::UIComponent_PressStartToPlay(int type, void* data, UILayer* layer) : UIScene(type, layer)
{
    initialiseMovie();
    m_showSaveIcon = false;
    m_showAutosaveTimer = false;
    m_showTrialTimer = false;
    for (int i = 0; i < 4; ++i)
        m_showPressStart[i] = false;
    m_trialTimer = L"";
    m_autosaveTimer = L"";
    m_trialTimerLabel.init(UIString(L""));
    m_trialTimerLabel.setVisible(false);
    m_player = 0;
    m_achievement = 0;
    m_holdToShowAchievements = false;
    m_holdActionCancelled = false;
    m_holdCancelPending = false;
    m_achievementToastShowing = false;
    SetText(UIString(L10N::GetString(IDS_PRESS_START_TO_JOIN)), false);
    m_saveIcon.setVisible(false);
    m_playerNameLabel.init(UIString(L""));
    m_mainPanel.setVisible(false);
    m_playerNameLabel.setVisible(false);
}

UIComponent_PressStartToPlay::~UIComponent_PressStartToPlay()
{
}

bool UIComponent_PressStartToPlay::GetAchievementToastShowing(int player)
{
    return (m_achievementToastShowing || m_holdToShowAchievements) && player == m_player;
}

bool UIComponent_PressStartToPlay::GetHoldToShowAchievementsActive(bool includeCancelled)
{
    if (includeCancelled)
        m_holdCancelPending = includeCancelled;
    return m_holdToShowAchievements;
}

unsigned int UIComponent_PressStartToPlay::handleHold(int player, int action, bool repeat, bool pressed, bool released,
                                                      bool& handled)
{
    if (GetAchievementToastShowing(player)
        && (action == EControllerActions_Inventory || action == EControllerActions_MenuY) && pressed && !repeat)
    {
        m_holdToShowAchievements = true;
        m_holdActionCancelled = false;
        addTimer(2, 1000);
        handled = true;
        return 1;
    }
    if (m_holdToShowAchievements && !handled
        && (action == EControllerActions_Inventory || action == EControllerActions_MenuY) && released)
    {
        if (m_holdCancelPending)
        {
            m_holdCancelPending = false;
            m_holdActionCancelled = true;
        }
        m_holdToShowAchievements = false;
        killTimer(2);
        handled = false;
        return 0;
    }
    return 1;
}

void UIComponent_PressStartToPlay::HideChestRefillAnimation()
{
    if (hasMovie())
        static_cast<FJ_PressStartToPlay*>(m_movie->getRootNode()->getFuiNode())->HideChestRefillAnimation();
}

void UIComponent_PressStartToPlay::ResetUIComponents()
{
    m_achievementToastShowing = false;
    HideChestRefillAnimation();
}

void UIComponent_PressStartToPlay::showTrialTimer(bool show)
{
    m_showTrialTimer = show;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
        m_trialTimerLabel.setVisible(show);
}

void UIComponent_PressStartToPlay::setTrialTimer(const std::wstring& timer)
{
    m_trialTimer = timer;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
        m_trialTimerLabel.setLabel(UIString(timer), false, false);
}

void UIComponent_PressStartToPlay::showSaveIcon(bool show)
{
    m_showSaveIcon = show;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie()
        && GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplaySaveIcon))
        m_saveIcon.setVisible(show);
}

void UIComponent_PressStartToPlay::ShowChestRefillAnimation(bool show, int yOffset)
{
    if (hasMovie())
    {
        static_cast<FJ_PressStartToPlay*>(m_movie->getRootNode()->getFuiNode())
            ->ShowChestRefillAnimation(show, yOffset);
        addTimer(3, 2500);
    }
}

void UIComponent_PressStartToPlay::showPlayerDisplayName(bool show)
{
    m_playerNameLabel.setVisible(false);
}

void UIComponent_PressStartToPlay::SetText(UIString text, bool multiLine)
{
    if (!multiLine)
        static_cast<FJ_PressStartToPlay*>(m_movie->getRootNode()->getFuiNode())->SetText(text.getString());
    else
        static_cast<FJ_PressStartToPlay*>(m_movie->getRootNode()->getFuiNode())->SetMultiLineText(text.getString());
}

void UIComponent_PressStartToPlay::showPressStart(int player, bool show)
{
    if (gConsoleUIController.IsSceneInStack(0xff, EUIScene_MessageBox))
        return;
    m_showPressStart[player] = show;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
    {
        if (gApp.IsDrcPreventingSplitscreen())
            SetText(UIString(L10N::GetString(0x9a0686fb)), true);
        else
            SetText(UIString(L10N::GetString(IDS_PRESS_START_TO_JOIN)), false);
        m_mainPanel.setVisible(show);
        if (show)
        {
            addTimer(0, 3000);
            static_cast<FJ_PressStartToPlay*>(m_movie->getRootNode()->getFuiNode())->ShowController(player, false);
        }
    }
}

void UIComponent_PressStartToPlay::registerTexture(int achievement)
{
    AchievementInfo* info = (AchievementInfo*)UIScene_AchievementsMenu::GetInfo(achievement);
    wchar_t path[64];
    std::swprintf(path, 64, L"Graphics\\Achievements\\%ls.png", info->icon);
    if (gApp.hasArchiveFile(std::wstring(path), false))
    {
        arrayWithLength<unsigned char> data = gApp.getArchiveFile(std::wstring(path), false);
        if (data.length != 0 && data.data != 0)
            registerSubstitutionTexture(std::wstring(info->icon), data.data, data.length, true);
    }
}

void UIComponent_PressStartToPlay::ShowAchievementToast(const Toast& toast)
{
    addTimer(1, 4000);
    m_achievementToastShowing = true;
    static_cast<FJ_PressStartToPlay*>(m_movie->getRootNode()->getFuiNode())
        ->ShowAchievementToast(toast.title, toast.description, toast.gamerPic, toast.achievementIcon);
}

int UIComponent_PressStartToPlay::AvatarReturned(void* context, unsigned char* data, unsigned int size)
{
    AvatarRequest* request = static_cast<AvatarRequest*>(context);
    UIComponent_PressStartToPlay* scene = request->scene;
    int achievement = request->achievement;
    int player = request->player;
    delete request;

    if (data == 0 && size != 0)
    {
        scene->m_achievement = achievement;
        scene->m_player = player;
        std::wstring texture = L"PlayerIcon" + _toString<int>(player);
        scene->registerTexture(achievement);
        scene->registerSubstitutionTexture(texture, 0, size, true);
        AchievementInfo* info = (AchievementInfo*)UIScene_AchievementsMenu::GetInfo(achievement);
        std::wstring title = L10N::GetString(info->title);
        std::wstring text = L10N::GetString(0x4523129e);
        std::wstring description = gApp.FormatHTMLString(
            player, replaceAll(text, L"{*CONTROLLER_VK_X*}", L"{*CONTROLLER_VK_Y*}"), -1);
        Toast toast = {title, description, texture, info->icon};
        if (!scene->m_achievementToastShowing)
            scene->ShowAchievementToast(toast);
        else
            scene->m_achievementToasts.push_back(toast);
    }
    return 0;
}

void UIComponent_PressStartToPlay::SetAchievementUnlocked(int player, int achievement)
{
    gProfile->GetProfileAvatar(player, AvatarReturned, new AvatarRequest(this, player, achievement));
}

bool UIComponent_PressStartToPlay::GetHoldActionCancelled(int player)
{
    if (GetAchievementToastShowing(player))
    {
        bool cancelled = m_holdActionCancelled;
        m_holdActionCancelled = false;
        return cancelled;
    }
    return false;
}

unsigned int UIComponent_PressStartToPlay::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_trialTimerLabel.setupControl(this, root, std::string("TrialTimer"));
        m_controls.push_back(&m_trialTimerLabel);
    }
    {
        m_saveIcon.setupControl(this, root, std::string("SaveIcon"));
        m_controls.push_back(&m_saveIcon);
    }
    {
        m_playerNameLabel.setupControl(this, root, std::string("PlayerName"));
        m_controls.push_back(&m_playerNameLabel);
    }
    {
        m_mainPanel.setupControl(this, root, std::string("MainPanel"));
        m_controls.push_back(&m_mainPanel);
    }
    return 1;
}

std::wstring UIComponent_PressStartToPlay::getMoviePath()
{
    return std::wstring(L"PressStartToPlay");
}

void UIComponent_PressStartToPlay::handleReload()
{
    bool saveVisible = m_showSaveIcon && GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplaySaveIcon);
    m_saveIcon.setVisible(saveVisible);
    m_trialTimerLabel.setVisible(m_showAutosaveTimer);
    m_trialTimerLabel.setLabel(UIString(m_autosaveTimer), false, false);
    m_trialTimerLabel.setVisible(m_showTrialTimer);
    m_trialTimerLabel.setLabel(UIString(m_trialTimer), false, false);
    SetText(UIString(L10N::GetString(IDS_PRESS_START_TO_JOIN)), false);
    bool any = false;
    for (int i = 0; i < 4; ++i)
    {
        bool show = m_showPressStart[i];
        any |= show;
        if (show)
        {
            addTimer(0, 3000);
            static_cast<FJ_PressStartToPlay*>(m_movie->getRootNode()->getFuiNode())->ShowController(i, false);
        }
    }
    m_mainPanel.setVisible(any);
}

void UIComponent_PressStartToPlay::HideAchievementToast()
{
    m_achievementToastShowing = false;
    static_cast<FJ_PressStartToPlay*>(m_movie->getRootNode()->getFuiNode())->HideAchievementToast();
    if (!m_achievementToasts.empty())
    {
        std::vector<Toast>::iterator it = m_achievementToasts.begin();
        ShowAchievementToast(*it);
        m_achievementToasts.erase(it);
    }
}

void UIComponent_PressStartToPlay::ShowAchievementMenu()
{
    m_holdToShowAchievements = false;
    m_achievementToastShowing = false;
    if (gApp.GetGameStarted() != 0)
        gConsoleUIController.CloseAllPlayersScenes(false);
    gConsoleUIController.NavigateToScene(m_player, EUIScene_AchievementsMenu, (void*)(unsigned int)m_achievement,
                                         EUILayer_Scene, EUIGroup_PlayerDefault);
}

void UIComponent_PressStartToPlay::handleTimerComplete(int timerId)
{
    if (gConsoleUIController.IsExpectingOrReloadingSkin() || !hasMovie())
        return;
    if (timerId == 0)
    {
        m_mainPanel.setVisible(false);
        for (int i = 0; i < 4; ++i)
            m_showPressStart[i] = false;
        gConsoleUIController.ClearPressStart();
    }
    else if (timerId == 3)
    {
        HideChestRefillAnimation();
        killTimer(3);
    }
    else if (timerId == 1)
    {
        killTimer(1);
        HideAchievementToast();
    }
    else if (timerId == 2)
    {
        ShowAchievementMenu();
        killTimer(2);
    }
}

void UIComponent_PressStartToPlay::setAutosaveTimer(const std::wstring& timer)
{
    m_autosaveTimer = timer;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
        m_trialTimerLabel.setLabel(UIString(timer), false, false);
}

void UIComponent_PressStartToPlay::showAutosaveTimer(bool show)
{
    m_showAutosaveTimer = show;
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
        m_trialTimerLabel.setVisible(show);
}

void UIComponent_PressStartToPlay::ShowBetaWatermark()
{
    if (hasMovie())
        static_cast<FJ_PressStartToPlay*>(m_movie->getRootNode()->getFuiNode())->ShowBetaWatermark();
}

void UIComponent_PressStartToPlay::tick()
{
    UIScene::tick();
    Minecraft* minecraft = Minecraft::GetInstance();
    if (!gConsoleUIController.IsExpectingOrReloadingSkin() && hasMovie())
    {
        for (int i = 0; i < 4; ++i)
            if (m_showPressStart[i] && minecraft->mLocalPlayers[i] != 0)
                showPressStart(i, false);
    }
}

int UIComponent_PressStartToPlay::getSceneType()
{
    return EUIScene_PressStartToPlay;
}

bool UIComponent_PressStartToPlay::stealsFocus()
{
    return false;
}

bool UIComponent_PressStartToPlay::hasFocus(int sceneType)
{
    return false;
}

bool UIComponent_PressStartToPlay::hidesLowerScenes()
{
    return false;
}
