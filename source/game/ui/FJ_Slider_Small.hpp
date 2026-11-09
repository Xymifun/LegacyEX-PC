#ifndef GAME_UI_FJ_SLIDER_SMALL_HPP
#define GAME_UI_FJ_SLIDER_SMALL_HPP

#include "game/ui/FJ_Slider.hpp"

class fuiRenderNode;

class FJ_Slider_Small : public FJ_Slider
{
public:
    FJ_Slider_Small(fuiRenderNode* renderNode);
    static FJ_Slider* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
