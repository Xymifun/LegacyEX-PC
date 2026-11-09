#ifndef GAME_UI_FJ_KEYBOARDBUTTONS_KEYS_NORMAL_HPP
#define GAME_UI_FJ_KEYBOARDBUTTONS_KEYS_NORMAL_HPP

#include <stdint.h>
#include "game/ui/FJ_KeyboardButtons.hpp"

class fuiRenderNode;

class FJ_KeyboardButtons_Keys_Normal : public FJ_KeyboardButtons
{
public:
    explicit FJ_KeyboardButtons_Keys_Normal(fuiRenderNode* renderNode);
    static FJ_KeyboardButtons* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
