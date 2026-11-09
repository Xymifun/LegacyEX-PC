#ifndef GAME_UI_UISCENE_FULLSCREENPROGRESS_HPP
#define GAME_UI_UISCENE_FULLSCREENPROGRESS_HPP

#include <string>

#include "game/minigame/EMiniGameId.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_Progress.hpp"

class C4JThreadImpl;
class UILayer;

struct FullscreenProgressState
{
    bool showConfirmation;
    bool reservedFlag1;
    bool reservedFlag2;
    bool showTip;
    int closeMode;
    int playerId;
    EUIScene backScene;

    FullscreenProgressState()
        : showConfirmation(false),
          reservedFlag1(true),
          reservedFlag2(true),
          showTip(true),
          closeMode(0),
          backScene(EUIScene_None)
    {
    }
};

class UIScene_FullscreenProgress : public UIScene
{
public:
    UIScene_FullscreenProgress(int type, void* data, UILayer* layer);
    virtual ~UIScene_FullscreenProgress();

    void SetLoadingScreenType(int type);
    void setMiniGameLogoTexture();
    void ShowConfirmationButton(bool show);
    void SetTipText(bool initial);
    virtual unsigned int mapElementsAndNames();
    virtual void handleReload();
    virtual void updateTooltips();
    virtual void handleDestroy();
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void handleTimerComplete(int timerId);
    void SetWasCancelled(bool cancelled);
    virtual bool isReadyToDelete();
    virtual int getSceneType();
    virtual unsigned long long getDefaultGtcButtons();

private:
    C4JThreadImpl* m_thread;
    bool m_threadStarted;
    FullscreenProgressState* m_completionData;
    bool m_confirmationVisible;
    void (*m_cancelFunc)(void*);
    int (*m_completeFunc)(void*);
    void* m_cancelFuncParam;
    void* m_completeFuncParam;
    bool m_waitForThreadToDelete;
    EMiniGameId m_miniGameId;
    bool m_miniGame;
    std::wstring m_titleText;
    std::wstring m_statusText;
    int m_progress;
    int m_cancelTooltip;
    bool m_wasCancelled;
    bool m_confirmationButtonHidden;
    UIControl_Progress m_progressControl;
    UIControl_Label m_titleLabel;
    UIControl_Label m_tipLabel;
    UIControl_Button m_confirmationButton;
    UIControl m_timerControl;
    UIControl m_timerTitleControl;
    UIControl_BitmapIcon m_logoControl;
};

#endif
