#include "game/ui/NameEntryScreen.hpp"

#include <string>

#include "game/core/Button.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/GuiComponent.hpp"
#include "game/core/StringUtils.hpp"

static std::wstring ALLOWED_CHARACTERS(
    L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ,.:-_'*!\"#%/()=+?[]{}<>");

void NameEntryScreen::init()
{
    mButtons.clear();
    mButtons.push_back(new Button(0, mWidth / 2 - 100, mHeight / 4 + 120, L"Save"));
    mButtons.push_back(new Button(1, mWidth / 2 - 100, mHeight / 4 + 144, L"Cancel"));

    mButtons[0]->mActive = trimString(mName).size() > 1;
}

void NameEntryScreen::removed()
{
}

void NameEntryScreen::tick()
{
    ++mTickCount;
}

void NameEntryScreen::buttonClicked(Button* button)
{
    if (button->mActive)
    {
        if (button->mId == 0 && trimString(mName).size() > 1)
        {
            trimString(mName);
            mMinecraft->setScreen(NULL);
        }

        if (button->mId == 1)
        {
            mMinecraft->setScreen(mParentScreen);
        }
    }
}

void NameEntryScreen::keyPressed(wchar_t key, int action)
{
    if (action == 29 && !mName.empty())
    {
        mName = mName.substr(0, mName.size() - 1);
    }

    if (ALLOWED_CHARACTERS.find(key, 0) != std::wstring::npos && mName.size() < 64)
    {
        mName += key;
    }

    mButtons[0]->mActive = trimString(mName).size() > 1;
}

void NameEntryScreen::render(int x, int y, float partialTick)
{
    renderBackground();
    drawCenteredString(mFont, mTitle, mWidth / 2, 40, 0xffffff);

    int left = mWidth / 2 - 100;
    int top = mHeight / 2 - 10;
    fill(left - 1, top - 1, left + 201, top + 21, 0xffa0a0a0);
    fill(left, top, left + 200, top + 20, 0xff000000);

    const wchar_t* cursor = (mTickCount / 6 & 1) == 0 ? L"_" : L"";
    drawString(mFont, mName + cursor, left + 4, top + 6, 0xe0e0e0);

    Screen::render(x, y, partialTick);
}
