#ifndef GAME_UI_UICONTROL_MINECRAFTHORSE_HPP
#define GAME_UI_UICONTROL_MINECRAFTHORSE_HPP

#include "game/ui/UIControl.hpp"

struct fuiRect;

class UIControl_MinecraftHorse : public UIControl
{
public:
    UIControl_MinecraftHorse();
    void render(fuiRect* rect);

private:
    float mScreenWidth;
    float mScreenHeight;
    float mScaledWidth;
    float mScaledHeight;
};

#endif
