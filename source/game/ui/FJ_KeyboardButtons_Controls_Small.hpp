#ifndef GAME_UI_FJ_KEYBOARDBUTTONS_CONTROLS_SMALL_HPP
#define GAME_UI_FJ_KEYBOARDBUTTONS_CONTROLS_SMALL_HPP

#include <stdint.h>
#include "game/ui/FJ_KeyboardButtons.hpp"

class fuiRenderNode;

class FJ_KeyboardButtons_Controls_Small : public FJ_KeyboardButtons
{
public:
    explicit FJ_KeyboardButtons_Controls_Small(fuiRenderNode* renderNode);
    static FJ_KeyboardButtons* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
