#ifndef GAME_UI_VIDEOSETTINGSSCREEN_HPP
#define GAME_UI_VIDEOSETTINGSSCREEN_HPP

#include <string>

#include "game/ui/Screen.hpp"

class Options;

class VideoSettingsScreen : public Screen
{
public:
    VideoSettingsScreen(Screen* parentScreen, Options* options);

    virtual void init();
    virtual void buttonClicked(Button* button);
    virtual void render(int x, int y, float partialTick);

private:
    Screen* mParentScreen;
    std::wstring mTitle;
    Options* mOptions;
};

#endif
