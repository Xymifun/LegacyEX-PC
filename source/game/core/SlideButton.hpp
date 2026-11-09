#ifndef GAME_CORE_SLIDEBUTTON_HPP
#define GAME_CORE_SLIDEBUTTON_HPP

#include <string>

#include "game/core/Button.hpp"
#include "game/core/Options.hpp"

class SlideButton : public Button
{
public:
    SlideButton(int id, int x, int y, const Options::Option* option, const std::wstring& message, float value);

    virtual int getYImage(bool hovered);
    virtual void renderBg(Minecraft* minecraft, int mouseX, int mouseY);
    virtual bool clicked(Minecraft* minecraft, int mouseX, int mouseY);
    virtual void released(int mouseX, int mouseY);

private:
    float mValue;
    bool mSliding;
    const Options::Option* mOption;
};

#endif
