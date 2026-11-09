#ifndef GAME_UI_UISCENE_DEATHMENU_HPP
#define GAME_UI_UISCENE_DEATHMENU_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"

class UIScene_DeathMenu : public UIScene
{
public:
    UIScene_DeathMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_DeathMenu();

    virtual unsigned int mapElementsAndNames();
    virtual void handleTimerComplete(int timerId);
    virtual void updateTooltips();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void tick();
    virtual void handlePress(int controlId, int childId);
    virtual int getSceneType();

private:
    bool m_deathMenuActive;
    bool m_unknownStateA;
    bool m_unknownStateB;
    UIControl_Button m_respawnButton;
    UIControl_Button m_exitButton;
    UIControl_Label m_titleLabel;
    UIControl_Label m_deathMessageLabel;
    UIControl_Label m_extraMessageLabel;
};

#endif
