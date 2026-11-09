#ifndef GAME_UI_FJ_KEYBOARDBUTTONS_CONTROLS_NORMAL_HPP
#define GAME_UI_FJ_KEYBOARDBUTTONS_CONTROLS_NORMAL_HPP

#include <stdint.h>
#include "game/ui/FJ_KeyboardButtons.hpp"

class fuiRenderNode;

class FJ_KeyboardButtons_Controls_Normal : public FJ_KeyboardButtons
{
public:
    explicit FJ_KeyboardButtons_Controls_Normal(fuiRenderNode* pRenderNode);
    static FJ_KeyboardButtons* Create(fuiRenderNode* pRenderNode);
    virtual uint32_t getType();
};

#endif
