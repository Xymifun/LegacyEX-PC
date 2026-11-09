#ifndef GAME_UI_UICONTROL_MINECRAFTPLAYER_HPP
#define GAME_UI_UICONTROL_MINECRAFTPLAYER_HPP

#include "game/ui/UIControl.hpp"

struct fuiRect;

class UIControl_MinecraftPlayer : public UIControl
{
public:
    UIControl_MinecraftPlayer();
    void render(fuiRect* rect);

private:
    float mScreenWidth;
    float mScreenHeight;
    float mScaledWidth;
    float mScaledHeight;
};

#endif
