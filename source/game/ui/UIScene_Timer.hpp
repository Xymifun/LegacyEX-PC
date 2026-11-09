#ifndef GAME_UI_UISCENE_TIMER_HPP
#define GAME_UI_UISCENE_TIMER_HPP

#include <string>
#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"

class UIScene_Timer : public UIScene
{
public:
    UIScene_Timer(int type, void* data, UILayer* layer);
    virtual void reloadMovie(bool value);
    virtual bool needsReloaded();
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual bool hidesLowerScenes();

private:
    UIControl m_control;
};

#endif
