#ifndef GAME_UI_TITLESCREEN_HPP
#define GAME_UI_TITLESCREEN_HPP

#include <string>

#include "game/ui/Screen.hpp"

class Button;

class TitleScreen : public Screen
{
public:
    TitleScreen();

    virtual void render(int x, int y, float partialTick);
    virtual void keyPressed(wchar_t key, int action);
    virtual void buttonClicked(Button* button);
    virtual void init();
    virtual void tick();

private:
    float mLogoWidth;
    std::wstring mTitle;
    Button* mMultiplayerButton;
};

#endif
