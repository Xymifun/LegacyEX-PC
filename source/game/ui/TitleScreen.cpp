#include "game/ui/TitleScreen.hpp"

#include <string>
#include <vector>

#include "game/core/Button.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/JoinMultiplayerScreen.hpp"
#include "game/ui/OptionsScreen.hpp"
#include "game/ui/SelectWorldScreen.hpp"

TitleScreen::TitleScreen()
{
    mLogoWidth = 0.0f;
    mMultiplayerButton = NULL;
    mTitle.assign(L"missingno");
    std::vector<std::wstring> splashes;
    mTitle.assign(L"");
}

void TitleScreen::tick()
{
}

void TitleScreen::keyPressed(wchar_t key, int action)
{
}

void TitleScreen::init()
{
    Language* language = Language::getInstance();
    const int top = mHeight / 4 + 48;

    mButtons.push_back(new Button(1, mWidth / 2 - 100, top, language->getElement(L"menu.singleplayer")));

    mButtons.push_back(
        mMultiplayerButton = new Button(2, mWidth / 2 - 100, top + 24, language->getElement(L"menu.multiplayer")));

    mButtons.push_back(new Button(3, mWidth / 2 - 100, top + 48, language->getElement(L"menu.mods")));

    if (mMinecraft->mPaused)
    {
        mButtons.push_back(new Button(0, mWidth / 2 - 100, top + 72, language->getElement(L"menu.options")));
    }
    else
    {
        mButtons.push_back(new Button(0, mWidth / 2 - 100, top + 84, 98, 20, language->getElement(L"menu.options")));
        mButtons.push_back(new Button(4, mWidth / 2 + 2, top + 84, 98, 20, language->getElement(L"menu.quit")));
    }

    if (mMinecraft->mUser == NULL)
    {
        mMultiplayerButton->mActive = false;
    }
}

void TitleScreen::buttonClicked(Button* button)
{
    if (button->mId == 0)
    {
        mMinecraft->setScreen(new OptionsScreen(this, mMinecraft->mOptions));
    }

    if (button->mId == 1)
    {
        mMinecraft->setScreen(new SelectWorldScreen(this));
    }

    if (button->mId == 2)
    {
        mMinecraft->setScreen(new JoinMultiplayerScreen(this));
    }

    if (button->mId == 4)
    {
        mMinecraft->stop();
    }
}

void TitleScreen::render(int x, int y, float partialTick)
{
}
