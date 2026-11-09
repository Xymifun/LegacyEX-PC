#ifndef GAME_UI_FJ_KEYBOARDBUTTONS_KEYS_SMALL_HPP
#define GAME_UI_FJ_KEYBOARDBUTTONS_KEYS_SMALL_HPP

#include "game/ui/FJ_KeyboardButtons.hpp"

class FJ_KeyboardButtons_Keys_Small : public FJ_KeyboardButtons
{
public:
    explicit FJ_KeyboardButtons_Keys_Small(fuiRenderNode* renderNode);
    static FJ_KeyboardButtons* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
