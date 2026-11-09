#ifndef GAME_UI_UISCENE_DLCMAINMENU_HPP
#define GAME_UI_UISCENE_DLCMAINMENU_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_DynamicButtonList.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class UILayer;

class UIScene_DLCMainMenu : public UIScene
{
public:
    UIScene_DLCMainMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_DLCMainMenu();

    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    static int ExitDLCMainMenu(void* context, int pad, C4JStorage::EMessageResult result);
    virtual void handleTimerComplete(int timerId);
    virtual void updateTooltips();
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);

private:
    UIControl_DynamicButtonList m_offerList;
    UIControl_Label m_titleLabel;
    UIControl m_timer;
    bool m_initialized;
};

#endif
