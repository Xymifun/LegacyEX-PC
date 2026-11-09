#ifndef GAME_UI_CONFIRMSCREEN_HPP
#define GAME_UI_CONFIRMSCREEN_HPP

#include <string>

#include "game/ui/Screen.hpp"

class ConfirmScreen : public Screen
{
public:
    ConfirmScreen(Screen* parent, const std::wstring& title, const std::wstring& message,
                  const std::wstring& yesMessage, const std::wstring& noMessage, int id);
    ConfirmScreen(Screen* parent, const std::wstring& title, const std::wstring& message, int id);

    virtual void render(int x, int y, float partialTick);
    virtual void buttonClicked(Button* button);
    virtual void init();

private:
    Screen* mParent;
    std::wstring mTitle;
    std::wstring mMessage;
    std::wstring mYesMessage;
    std::wstring mNoMessage;
    int mId;
};

#endif
