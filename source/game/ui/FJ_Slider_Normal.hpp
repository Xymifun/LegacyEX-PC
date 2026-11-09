#ifndef GAME_UI_FJ_SLIDER_NORMAL_HPP
#define GAME_UI_FJ_SLIDER_NORMAL_HPP

#include <stdint.h>
#include "game/ui/FJ_Slider.hpp"

class fuiRenderNode;

class FJ_Slider_Normal : public FJ_Slider
{
public:
    explicit FJ_Slider_Normal(fuiRenderNode* renderNode);
    static FJ_Slider* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
