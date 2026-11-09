#include "game/core/EditBox.hpp"

#include "game/client/render/Font.hpp"
#include "game/core/GuiComponent.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/ui/Screen.hpp"

EditBox::EditBox(Screen* screen, Font* font, int x, int y, int width, int height, const std::wstring& value)
    : mMaxLength(0),
      mTickCount(0)
{
    mX = x;
    mFont = font;
    mScreen = screen;
    mY = y;
    mWidth = width;
    mHeight = height;
    setValue(value);
}

void EditBox::tick()
{
    ++mTickCount;
}

void EditBox::setValue(const std::wstring& value)
{
    mValue.assign(value);
}

void EditBox::setMaxLength(int maxLength)
{
    mMaxLength = maxLength;
}

std::wstring EditBox::getValue()
{
    return mValue;
}

void EditBox::keyPressed(wchar_t key, int action)
{
    if (!mActive || !mFocused)
    {
        return;
    }

    if (key == 9)
    {
        mScreen->tabPressed();
    }

    if (action == 29 && !mValue.empty())
    {
        mValue = mValue.substr(0, mValue.size() - 1);
    }

    wchar_t character = key;
    if (SharedConstants::ALLOWED_CHARACTERS.find(&character, 0, 1) != std::wstring::npos
        && (mValue.size() < static_cast<std::wstring::size_type>(mMaxLength) || mMaxLength == 0))
    {
        mValue += key;
    }
}

void EditBox::focus(bool focused)
{
    if (focused && !mFocused)
    {
        mTickCount = 0;
    }
    mFocused = focused;
}

void EditBox::mouseClicked(int x, int y, int button)
{
    bool focused = mActive && x >= mX && x < mX + mWidth && y >= mY && y < mY + mHeight;
    focus(focused);
}

void EditBox::render()
{
    fill(mX - 1, mY - 1, mX + mWidth + 1, mY + mHeight + 1, 0xffa0a0a0);
    fill(mX, mY, mX + mWidth, mY + mHeight, 0xff000000);

    if (mActive)
    {
        bool showCursor = mFocused && (mTickCount / 6 & 1) == 0;
        const wchar_t* cursor = L"";
        if (showCursor)
        {
            cursor = L"_";
        }
        std::wstring displayedValue = mValue + cursor;
        drawString(mFont, displayedValue, mX + 4, mY + (mHeight - 8) / 2, 0xe0e0e0);
    }
    else
    {
        drawString(mFont, mValue, mX + 4, mY + (mHeight - 8) / 2, 0x707070);
    }
}
