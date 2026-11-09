#ifndef GAME_UI_UISCENE_SAVEMESSAGE_HPP
#define GAME_UI_UISCENE_SAVEMESSAGE_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"

class UIScene_SaveMessage : public UIScene
{
public:
    UIScene_SaveMessage(int playerId, void* sceneData, UILayer* layer);
    virtual ~UIScene_SaveMessage();

    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleTimerComplete(int timerId);
    virtual void handlePress(int controlId, int childId);
    virtual int getSceneType();
    virtual bool hasFocus(int sceneType);
    virtual unsigned long long getDefaultGtcButtons();

private:
    bool m_saveComplete;
    UIControl_Button m_confirmButton;
    UIControl_Label m_descriptionLabel;
};

#endif
