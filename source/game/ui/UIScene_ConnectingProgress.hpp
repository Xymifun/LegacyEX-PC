#ifndef GAME_UI_UISCENE_CONNECTINGPROGRESS_HPP
#define GAME_UI_UISCENE_CONNECTINGPROGRESS_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_Progress.hpp"
#include "game/minigame/EMiniGameId.hpp"

class UIScene_ConnectingProgress : public UIScene
{
public:
    UIScene_ConnectingProgress(int type, void* data, UILayer* layer);
    virtual ~UIScene_ConnectingProgress();

    void SetLoadingScreenType(int type);
    void setMiniGameLogoTexture();
    void ShowConfirmationButton(bool show);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void handleTimerComplete(int timerId);
    virtual void updateTooltips();
    virtual void handleGainFocus(bool navigatedBack);
    virtual void handleLoseFocus();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);
    void tryRemovePlayer();
    void removeLocalPlayer();

private:
    bool m_setFailTimer;
    int m_timerTime;
    bool m_showTooltips;
    bool m_removeLocalPlayer;
    bool m_showingButton;
    bool m_cancelled;
    void (*m_cancelFunc)(void*);
    void* m_cancelFuncParam;
    EMiniGameId m_miniGameId;
    bool m_miniGameLogoSet;
    UIControl_Progress m_progressControl;
    UIControl_Label m_titleLabel;
    UIControl_Label m_tipLabel;
    UIControl_Button m_confirmationButton;
    UIControl m_timerControl;
    UIControl m_gameModeIconControl;
    UIControl_BitmapIcon m_logoControl;
};

#endif
