#ifndef GAME_CORE_EDITBOX_HPP
#define GAME_CORE_EDITBOX_HPP

#include <string>

#include "game/core/GuiComponent.hpp"

class Font;
class Screen;

class EditBox : public GuiComponent
{
public:
    EditBox(Screen* screen, Font* font, int x, int y, int width, int height, const std::wstring& value);

    void tick();
    void setValue(const std::wstring& value);
    void setMaxLength(int maxLength);
    std::wstring getValue();
    void keyPressed(wchar_t key, int action);
    void focus(bool focused);
    void mouseClicked(int x, int y, int button);
    void render();
    int getMaxLength();

private:
    Font* mFont;
    int mX;
    int mY;
    int mWidth;
    int mHeight;
    std::wstring mValue;
    int mMaxLength;
    int mTickCount;

public:
    bool mFocused;

private:
    bool mActive;
    Screen* mScreen;
};

#endif
