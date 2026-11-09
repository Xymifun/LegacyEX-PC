#ifndef GAME_UI_DEATHSCREEN_HPP
#define GAME_UI_DEATHSCREEN_HPP

#include "game/ui/Screen.hpp"

class Button;
class Font;
class Minecraft;

class DeathScreen : public Screen
{
public:
    virtual void init();
    virtual void keyPressed(wchar_t key, int action);
    virtual void buttonClicked(Button* button);
    virtual void render(int x, int y, float partialTick);
    virtual bool isPauseScreen();
};

#endif
