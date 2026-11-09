#ifndef GAME_UI_UISCENE_TRIALEXITUPSELL_HPP
#define GAME_UI_UISCENE_TRIALEXITUPSELL_HPP

#include <string>

#include "game/ui/UIScene.hpp"

class UILayer;

class UIScene_TrialExitUpsell : public UIScene
{
public:
    UIScene_TrialExitUpsell(int sceneType, void* data, UILayer* layer);
    virtual void updateTooltips();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleAnimationEnd();
    virtual int getSceneType();
    virtual bool hasFocus(int sceneType);
};

#endif
