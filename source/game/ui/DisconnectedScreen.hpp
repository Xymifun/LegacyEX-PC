#ifndef GAME_UI_DISCONNECTEDSCREEN_HPP
#define GAME_UI_DISCONNECTEDSCREEN_HPP

#include <string>
#include "game/ui/Screen.hpp"

class Button;

class DisconnectedScreen : public Screen
{
public:
    DisconnectedScreen(const std::wstring& title, std::wstring message, void* data, ...);

    virtual void tick();
    virtual void keyPressed(char key, int action);
    virtual void init();
    virtual void buttonClicked(Button* button);
    virtual void render(int x, int y, float partialTick);

private:
    std::wstring mTitle;
    std::wstring mMessage;
};

#endif
