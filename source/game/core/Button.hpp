#ifndef GAME_CORE_BUTTON_HPP
#define GAME_CORE_BUTTON_HPP

#include <string>

#include "game/core/GuiComponent.hpp"

class Minecraft;

class Button : public GuiComponent
{
public:
    Button(int id, int x, int y, const std::wstring& message);
    Button(int id, int x, int y, int width, int height, const std::wstring& message);

    void init(int id, int x, int y, int width, int height, const std::wstring& message);
    virtual int getYImage(bool hovered);
    virtual void render(Minecraft* minecraft, int mouseX, int mouseY);
    virtual void renderBg(Minecraft* minecraft, int mouseX, int mouseY);
    virtual void released(int mouseX, int mouseY);
    virtual bool clicked(Minecraft* minecraft, int mouseX, int mouseY);

    int mWidth;
    int mHeight;
    int mX;
    int mY;
    std::wstring mMessage;
    int mId;
    bool mActive;
    bool mVisible;
};

#endif
