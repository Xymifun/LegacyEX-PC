#ifndef GAME_UI_FJ_ENCHANTINGBUTTON_SMALL_HPP
#define GAME_UI_FJ_ENCHANTINGBUTTON_SMALL_HPP

#include "game/ui/FJ_EnchantingButton.hpp"

class fuiRenderNode;

class FJ_EnchantingButton_Small : public FJ_EnchantingButton
{
public:
    FJ_EnchantingButton_Small(fuiRenderNode* pRenderNode);
    static FJ_EnchantingButton* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
