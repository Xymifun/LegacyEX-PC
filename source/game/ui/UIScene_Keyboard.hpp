#ifndef GAME_UI_UISCENE_KEYBOARD_HPP
#define GAME_UI_UISCENE_KEYBOARD_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_TextInput.hpp"

class UIScene_Keyboard : public UIScene
{
public:
    UIScene_Keyboard(int sceneType, void* sceneData, UILayer* layer);
    virtual ~UIScene_Keyboard();

    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual bool allowRepeat(int action);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    void KeyboardDonePressed();
    virtual void handleTimerComplete(int timerId);
    virtual void handlePress(int controlId, int childId);
    virtual std::wstring getMoviePath();
    virtual int getSceneType();
    virtual bool hasFocus(int sceneType);
    virtual bool hidesLowerScenes();

private:
    bool m_keyboardDone;
    UIControl_Label m_enterTextLabel;
    UIControl_TextInput m_keyboardTextInput;
    UIControl_Button m_spaceButton;
    UIControl_Button m_cursorLeftButton;
    UIControl_Button m_cursorRightButton;
    UIControl_Button m_capsButton;
    UIControl_Button m_doneButton;
    UIControl_Button m_symbolsButton;
    UIControl_Button m_backspaceButton;
};

#endif
