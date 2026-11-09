#include "game/ui/FJ_EnchantingButton_Normal.hpp"

FJ_EnchantingButton_Normal::FJ_EnchantingButton_Normal(fuiRenderNode* pRenderNode) : FJ_EnchantingButton(pRenderNode)
{
}

FJ_EnchantingButton* FJ_EnchantingButton_Normal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_EnchantingButton_Normal(pRenderNode);
}

unsigned int FJ_EnchantingButton_Normal::getType()
{
    return 0x800c;
}
