#ifndef GAME_UI_NAMEENTRYSCREEN_HPP
#define GAME_UI_NAMEENTRYSCREEN_HPP

#include <string>

#include "game/ui/Screen.hpp"

class Button;

class NameEntryScreen : public Screen
{
public:
    NameEntryScreen(Screen* parent, const std::wstring& name, int buttonId);

    virtual void render(int x, int y, float partialTick);
    virtual void keyPressed(wchar_t key, int action);
    virtual void buttonClicked(Button* button);
    virtual void init();
    virtual void tick();
    virtual void removed();

private:
    Screen* mParentScreen;
    std::wstring mTitle;
    unsigned char mReserved[4];
    std::wstring mName;
    int mTickCount;
};

#endif
