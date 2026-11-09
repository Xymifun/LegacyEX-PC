#ifndef GAME_UI_UISCENE_EULA_HPP
#define GAME_UI_UISCENE_EULA_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_DynamicLabel.hpp"

class UIScene_EULA : public UIScene
{
public:
    UIScene_EULA(int sceneType, void* callback, UILayer* layer);
    virtual ~UIScene_EULA();
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual int getSceneType();
    virtual bool hasFocus(int controller);
    virtual unsigned long long getDefaultGtcButtons();

private:
    bool m_ignoreInput;
    UIControl_Button m_acceptButton;
    UIControl_DynamicLabel m_eulaText;
};

#endif
