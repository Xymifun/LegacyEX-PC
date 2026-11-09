#include "game/ui/ErrorScreen.hpp"

#include "game/core/GuiComponent.hpp"

void ErrorScreen::render(int x, int y, float partialTick)
{
    fillGradient(0, 0, mWidth, mHeight, 0xff402020, 0xff501010);
    drawCenteredString(mFont, mTitle, mWidth / 2, 90, 0xffffff);
    drawCenteredString(mFont, mMessage, mWidth / 2, 110, 0xffffff);
    Screen::render(x, y, partialTick);
}

void ErrorScreen::keyPressed(wchar_t key, int action)
{
    (void)key;
    (void)action;
}

void ErrorScreen::init()
{
    (void)mButtons;
}
