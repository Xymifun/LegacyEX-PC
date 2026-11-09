#include "game/ui/Screen.hpp"

#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/GuiParticles.hpp"
#include "game/core/Button.hpp"
#include "game/core/Minecraft.hpp"

Screen::Screen() : GuiComponent()
{
    mMinecraft = NULL;
    mWidth = 0;
    mHeight = 0;
    mPassEvents = false;
    mFont = NULL;
    mGuiParticles = NULL;
    mSelectedButton = NULL;
}

void Screen::render(int x, int y, float partialTick)
{
    std::vector<Button*>::iterator end = mButtons.end();
    for (std::vector<Button*>::iterator it = mButtons.begin(); it != end; ++it)
        (*it)->render(mMinecraft, x, y);
}

void Screen::keyPressed(wchar_t key, int action)
{
    if (action == 28)
        mMinecraft->setScreen(NULL);
}

void Screen::mouseClicked(int x, int y, int button)
{
    if (button != 0)
        return;

    std::vector<Button*>::iterator end = mButtons.end();
    for (std::vector<Button*>::iterator it = mButtons.begin(); it != end; ++it)
    {
        Button* current = *it;
        if (current->clicked(mMinecraft, x, y))
        {
            mSelectedButton = current;
            mMinecraft->mSoundEngine->playUI(SoundEvents::UI_BUTTON_CLICK, 1.0f, 1.0f);
            buttonClicked(current);
        }
    }
}

void Screen::mouseReleased(int x, int y, int button)
{
    if (mSelectedButton != NULL && button == 0)
    {
        mSelectedButton->released(x, y);
        mSelectedButton = NULL;
    }
}

void Screen::buttonClicked(Button* button)
{
}

void Screen::init(Minecraft* minecraft, int width, int height)
{
    mGuiParticles = new GuiParticles(minecraft);
    mMinecraft = minecraft;
    mWidth = width;
    mFont = minecraft->mFont;
    mHeight = height;
    mButtons.clear();
    init();
}

void Screen::setSize(int width, int height)
{
    mWidth = width;
    mHeight = height;
}

void Screen::init()
{
    return;
}

void Screen::updateEvents()
{
}

void Screen::mouseEvent()
{
}

void Screen::keyboardEvent()
{
}

void Screen::tick()
{
}

void Screen::removed()
{
}

void Screen::renderBackground()
{
    renderBackground(0);
}

void Screen::renderBackground(int shade)
{
    if (mMinecraft->mLevel != NULL)
    {
        fillGradient(0, 0, mWidth, mHeight, 0xc0101010, 0xd0101010);
        return;
    }
    renderDirtBackground(shade);
}

void Screen::renderDirtBackground(int shade)
{
}

bool Screen::isPauseScreen()
{
    return true;
}

void Screen::confirmResult(bool result, int id)
{
}

void Screen::tabPressed()
{
}
