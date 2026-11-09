#include "game/core/Button.hpp"

#include "game/client/render/Textures.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"

Button::Button(int id, int x, int y, const std::wstring& message) : GuiComponent()
{
    init(id, x, y, 200, 20, message);
}

Button::Button(int id, int x, int y, int width, int height, const std::wstring& message) : GuiComponent()
{
    init(id, x, y, width, height, message);
}

void Button::init(int id, int x, int y, int width, int height, const std::wstring& message)
{
    mActive = true;
    mVisible = true;
    mId = id;
    mX = x;
    mY = y;
    mWidth = width;
    mHeight = height;
    mMessage = message;
}

int Button::getYImage(bool hovered)
{
    int result = 1;
    if (!mActive)
    {
        result = 0;
    }
    else if (hovered)
    {
        result = 2;
    }
    return result;
}

void Button::render(Minecraft* minecraft, int mouseX, int mouseY)
{
    if (!mVisible)
    {
        return;
    }

    Font* font = minecraft->mFont;
    GlStateManager::bindTexture(minecraft->mTextures->loadTexture(8));
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    bool hovered = mouseX >= mX && mouseY >= mY && mouseX < mX + mWidth && mouseY < mY + mHeight;
    int yImage = getYImage(hovered);
    blit(mX, mY, 0, 46 + yImage * 20, mWidth / 2, mHeight);
    blit(mX + mWidth / 2, mY, 200 - mWidth / 2, 46 + yImage * 20, mWidth / 2, mHeight);
    renderBg(minecraft, mouseX, mouseY);

    if (!mActive)
    {
        drawCenteredString(font, mMessage, mX + mWidth / 2, mY + (mHeight - 8) / 2, 0xffa0a0a0);
    }
    else if (hovered)
    {
        drawCenteredString(font, mMessage, mX + mWidth / 2, mY + (mHeight - 8) / 2, 0xffffa0);
    }
    else
    {
        drawCenteredString(font, mMessage, mX + mWidth / 2, mY + (mHeight - 8) / 2, 0xe0e0e0);
    }
}

void Button::renderBg(Minecraft* minecraft, int mouseX, int mouseY)
{
}

void Button::released(int mouseX, int mouseY)
{
}

bool Button::clicked(Minecraft* minecraft, int mouseX, int mouseY)
{
    return mActive && mouseX >= mX && mouseY >= mY && mouseX < mX + mWidth && mouseY < mY + mHeight;
}
