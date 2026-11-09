#ifndef GAME_UI_UICOMPONENT_PRESSSTARTTOPLAY_HPP
#define GAME_UI_UICOMPONENT_PRESSSTARTTOPLAY_HPP

#include <string>
#include <vector>

#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIScene.hpp"

class UILayer;
class UIString;

class UIComponent_PressStartToPlay : public UIScene
{
public:
    class Toast
    {
    public:
        std::wstring title;
        std::wstring description;
        std::wstring gamerPic;
        std::wstring achievementIcon;
    };

    UIComponent_PressStartToPlay(int type, void* data, UILayer* layer);
    virtual ~UIComponent_PressStartToPlay();
    bool GetAchievementToastShowing(int player);
    bool GetHoldToShowAchievementsActive(bool includeCancelled);
    unsigned int handleHold(int player, int action, bool repeat, bool pressed, bool released, bool& handled);
    void HideChestRefillAnimation();
    void ResetUIComponents();
    void showTrialTimer(bool show);
    void setTrialTimer(const std::wstring& timer);
    void showSaveIcon(bool show);
    void ShowChestRefillAnimation(bool show, int yOffset);
    void showPlayerDisplayName(bool show);
    void SetText(UIString text, bool multiLine);
    void showPressStart(int player, bool show);
    void registerTexture(int achievement);
    void ShowAchievementToast(const Toast& toast);
    static int AvatarReturned(void* context, unsigned char* data, unsigned int size);
    void SetAchievementUnlocked(int player, int achievement);
    bool GetHoldActionCancelled(int player);
    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    void HideAchievementToast();
    void ShowAchievementMenu();
    virtual void handleTimerComplete(int timerId);
    void setAutosaveTimer(const std::wstring& timer);
    void showAutosaveTimer(bool show);
    void ShowBetaWatermark();
    virtual void tick();
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int sceneType);
    virtual bool hidesLowerScenes();

private:
    bool m_showSaveIcon;
    bool m_showAutosaveTimer;
    bool m_showTrialTimer;
    bool m_showPressStart[4];
    std::wstring m_trialTimer;
    std::wstring m_autosaveTimer;
    UIControl_Label m_trialTimerLabel;
    UIControl_Label m_playerNameLabel;
    UIControl m_saveIcon;
    UIControl m_mainPanel;
    int m_achievement;
    bool m_achievementToastShowing;
    int m_player;
    bool m_holdToShowAchievements;
    bool m_holdActionCancelled;
    bool m_holdCancelPending;
    std::vector<Toast> m_achievementToasts;
};

#endif
