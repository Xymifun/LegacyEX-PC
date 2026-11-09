#ifndef GAME_UI_ERRORSCREEN_HPP
#define GAME_UI_ERRORSCREEN_HPP

#include <string>

#include "game/ui/Screen.hpp"

class ErrorScreen : public Screen
{
public:
    ErrorScreen(const std::wstring& title, const std::wstring& message);

    virtual void render(int x, int y, float partialTick);
    virtual void keyPressed(wchar_t key, int action);
    virtual void init();

private:
    std::wstring mTitle;
    std::wstring mMessage;
};

#endif
