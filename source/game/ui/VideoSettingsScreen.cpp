#include "game/ui/VideoSettingsScreen.hpp"

#include "game/core/Button.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/core/SlideButton.hpp"
#include "game/core/SmallButton.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"

VideoSettingsScreen::VideoSettingsScreen(Screen* parentScreen, Options* options) : Screen()
{
    mTitle = L"Video Settings";
    mParentScreen = parentScreen;
    mOptions = options;
}

void VideoSettingsScreen::init()
{
    Language* language = Language::getInstance();
    mTitle = language->getElement(L"options.videoTitle");

    const Options::Option* videoOptions[8] = {Options::Option::graphics,         Options::Option::renderDistance,
                                              Options::Option::ambientOcclusion, Options::Option::framerateLimit,
                                              Options::Option::anaglyph,         Options::Option::viewBobbing,
                                              Options::Option::guiScale,         Options::Option::advancedOpengl};
    for (int i = 0; i < 8; ++i)
    {
        const Options::Option* option = videoOptions[i];
        if (!option->isProgress())
        {
            mButtons.push_back(new SmallButton(option->getId(), mWidth / 2 - 155 + i % 2 * 160,
                                               mHeight / 6 + 24 * (i >> 1), option, mOptions->getMessage(option)));
        }
        else
        {
            mButtons.push_back(new SlideButton(option->getId(), mWidth / 2 - 155 + i % 2 * 160,
                                               mHeight / 6 + 24 * (i >> 1), option, mOptions->getMessage(option),
                                               mOptions->getProgressValue(option)));
        }
    }

    mButtons.push_back(new Button(200, mWidth / 2 - 100, mHeight / 6 + 168, language->getElement(L"gui.done")));
}

void VideoSettingsScreen::buttonClicked(Button* button)
{
    if (!button->mActive)
    {
        return;
    }

    int id = button->mId;
    if (id < 100)
    {
        if (dynamic_cast<SmallButton*>(button) != NULL)
        {
            mOptions->toggle(static_cast<SmallButton*>(button)->getOption(), 1);
            button->mMessage = mOptions->getMessage(Options::Option::getItem(button->mId));
        }
        id = button->mId;
    }

    if (id == 200)
    {
        mMinecraft->mOptions->save();
        mMinecraft->setScreen(mParentScreen);
    }

    ScreenSizeCalculator screenSizeCalculator(mMinecraft->mOptions, -1);
    Screen::init(mMinecraft, static_cast<int>(screenSizeCalculator.getWidth()),
                 static_cast<int>(screenSizeCalculator.getHeight()));
}

void VideoSettingsScreen::render(int x, int y, float partialTick)
{
    renderBackground();
    drawCenteredString(mFont, mTitle, mWidth / 2, 20, 0xffffff);
    Screen::render(x, y, partialTick);
}
