#ifndef GAME_UI_PAUSESCREEN_HPP
#define GAME_UI_PAUSESCREEN_HPP

#include "game/ui/Screen.hpp"

class Button;

class PauseScreen : public Screen
{
public:
    PauseScreen();

    virtual void render(int x, int y, float partialTick);
    void buttonClicked(Button button);
    virtual void tick();
    void init();

private:
    int mButtonListState;
    int mTickCount;
};

#endif
