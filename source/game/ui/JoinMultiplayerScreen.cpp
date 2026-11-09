#include "game/ui/JoinMultiplayerScreen.hpp"

#include <string>
#include <vector>

#include "game/core/Button.hpp"
#include "game/core/EditBox.hpp"
#include "game/core/GuiComponent.hpp"
#include "game/core/Language.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/core/StringUtils.hpp"

template <typename T>
T _fromString(const std::wstring& value);

JoinMultiplayerScreen::JoinMultiplayerScreen(Screen* parentScreen)
    : Screen(),
      mParentScreen(parentScreen),
      mEditBox(NULL)
{
}

void JoinMultiplayerScreen::tick()
{
    mEditBox->tick();
}

void JoinMultiplayerScreen::init()
{
    Language* language = Language::getInstance();
    mButtons.clear();
    mButtons.push_back(
        new Button(0, mWidth / 2 - 100, mHeight / 4 + 108, language->getElement(L"multiplayer.connect")));
    mButtons.push_back(new Button(1, mWidth / 2 - 100, mHeight / 4 + 132, language->getElement(L"gui.cancel")));

    std::wstring value = replaceAll(mMinecraft->mOptions->mLastServer, L"_", L":");
    mButtons[0]->mActive = value.length() != 0;
    mEditBox = new EditBox(this, mFont, mWidth / 2 - 100, mHeight / 4 + 58, 200, 20, value);
    mEditBox->mFocused = true;
    mEditBox->setMaxLength(128);
}

void JoinMultiplayerScreen::removed()
{
}

void JoinMultiplayerScreen::buttonClicked(Button* button)
{
    if (!button->mActive)
        return;

    if (button->mId == 1)
    {
        mMinecraft->setScreen(mParentScreen);
        return;
    }

    if (button->mId == 0)
    {
        std::wstring value = trimString(mEditBox->getValue());
        mMinecraft->mOptions->mLastServer = replaceAll(value, L":", L"_");
        mMinecraft->mOptions->save();

        std::vector<std::wstring> parts = stringSplit(value, L'L');
        if (value[0] == L'[')
        {
            std::wstring::size_type close = value.find(L"]");
            if (close != std::wstring::npos)
            {
                std::wstring address = value.substr(1, close);
                std::wstring::size_type portStart = close + 1;
                std::wstring port = trimString(value.substr(portStart));
                if (port[0] == L':' && !port.empty())
                {
                    port = port.substr(1);
                    parts.clear();
                    parts.push_back(address);
                    parts.push_back(port);
                }
                else
                {
                    parts.clear();
                    parts.push_back(address);
                }
            }
        }

        if (parts.size() > 2)
        {
            parts.clear();
            parts.push_back(value);
        }
    }
}

void JoinMultiplayerScreen::parseInt(const std::wstring& value, int radix)
{
    _fromString<int>(value);
}

void JoinMultiplayerScreen::keyPressed(wchar_t key, int action)
{
    mEditBox->keyPressed(key, action);

    if (key == 13)
        buttonClicked(mButtons[0]);

    mButtons[0]->mActive = mEditBox->getValue().length() != 0;
}

void JoinMultiplayerScreen::mouseClicked(int x, int y, int button)
{
    Screen::mouseClicked(x, y, button);
    mEditBox->mouseClicked(x, y, button);
}

void JoinMultiplayerScreen::render(int x, int y, float partialTick)
{
    Language* language = Language::getInstance();
    renderBackground();
    drawCenteredString(mFont, language->getElement(L"multiplayer.title"), mWidth / 2, mHeight / 4 - 40, 0xffffff);
    drawString(mFont, language->getElement(L"multiplayer.info1"), mWidth / 2 - 140, mHeight / 4, 0xa0a0a0);
    drawString(mFont, language->getElement(L"multiplayer.info2"), mWidth / 2 - 140, mHeight / 4 + 9, 0xa0a0a0);
    drawString(mFont, language->getElement(L"multiplayer.ipinfo"), mWidth / 2 - 140, mHeight / 4 + 36, 0xa0a0a0);
    mEditBox->render();
    Screen::render(x, y, partialTick);
}
