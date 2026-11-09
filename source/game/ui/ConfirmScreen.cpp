#include "game/ui/ConfirmScreen.hpp"

#include "game/core/GuiComponent.hpp"
#include "game/core/SmallButton.hpp"

ConfirmScreen::ConfirmScreen(Screen* parent, const std::wstring& title, const std::wstring& message,
                             const std::wstring& yesMessage, const std::wstring& noMessage, int id)
    : Screen()
{
    mParent = parent;
    mTitle = title;
    mMessage = message;
    mYesMessage = yesMessage;
    mNoMessage = noMessage;
    mId = id;
}

void ConfirmScreen::init()
{
    mButtons.push_back(new SmallButton(0, mWidth / 2 - 155, mHeight / 6 + 96, mYesMessage));
    mButtons.push_back(new SmallButton(1, mWidth / 2 + 5, mHeight / 6 + 96, mNoMessage));
}

void ConfirmScreen::buttonClicked(Button* button)
{
    mParent->confirmResult(button->mId == 0, mId);
}

void ConfirmScreen::render(int x, int y, float partialTick)
{
    static int ticks = 0;
    renderBackground();
    drawCenteredString(mFont, mTitle, mWidth / 2, 70, 0xffffff);
    drawCenteredString(mFont, mMessage, mWidth / 2, 90, 0xffffff);
    Screen::render(x, y, partialTick);

    if (ticks++ == 100)
    {
        ticks = 0;
        buttonClicked(mButtons[0]);
    }
}
