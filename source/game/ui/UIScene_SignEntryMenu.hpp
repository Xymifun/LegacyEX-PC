#ifndef GAME_UI_UISCENE_SIGNENTRYMENU_HPP
#define GAME_UI_UISCENE_SIGNENTRYMENU_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_TextInput.hpp"

class SignBlockEntity;

class UIScene_SignEntryMenu : public UIScene
{
public:
    UIScene_SignEntryMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_SignEntryMenu();

    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    static int KeyboardCompleteCallback(void* data, bool accepted);
    virtual void handlePress(int controlId, int childId);
    virtual void handleDestroy();
    virtual int getSceneType();

private:
    boost::shared_ptr<SignBlockEntity> m_sign;
    int m_selectedLine;
    bool m_confirmRequested;
    bool m_keyboardActive;
    UIControl_Button m_confirmButton;
    UIControl_Label m_messageLabel;
    UIControl_TextInput m_lineInputs[4];
};

#endif
