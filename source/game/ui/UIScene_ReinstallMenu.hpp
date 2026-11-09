#ifndef GAME_UI_UISCENE_REINSTALLMENU_HPP
#define GAME_UI_UISCENE_REINSTALLMENU_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"

class UILayer;

class UIScene_ReinstallMenu : public UIScene
{
public:
    UIScene_ReinstallMenu(int type, void* data, UILayer* layer);

    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual int getSceneType();

private:
    UIControl_Button m_buttons[6];
};

#endif
