#ifndef GAME_UI_UISCENE_INTRO_HPP
#define GAME_UI_UISCENE_INTRO_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Touch.hpp"

class UILayer;

class UIScene_Intro : public UIScene
{
public:
    UIScene_Intro(int sceneType, void* data, UILayer* layer);
    void setIntroPlatform();

    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual bool hasFocus(int sceneType);
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleReload();
    virtual void handleAnimationEnd();
    virtual void handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                  bool repeat, bool released);

private:
    bool m_inputHandled;
    bool m_introFocus;
    UIControl_Touch m_touchControl;
};

#endif
