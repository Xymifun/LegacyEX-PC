#ifndef GAME_UI_CONTROLSSCREEN_HPP
#define GAME_UI_CONTROLSSCREEN_HPP

#include <string>

#include "game/ui/Screen.hpp"

class Options;

class ControlsScreen : public Screen
{
public:
    ControlsScreen(Screen* parent, Options* options);

    int getLeftScreenPosition();
    virtual void init();
    virtual void buttonClicked(Button* button);
    virtual void keyPressed(wchar_t key, int action);
    virtual void render(int x, int y, float partialTick);

private:
    Screen* mParent;
    std::wstring mTitle;
    Options* mOptions;
    int mSelectedKey;
};

#endif
