#include "game/ui/FJ_EnchantingButton_Small.hpp"

FJ_EnchantingButton_Small::FJ_EnchantingButton_Small(fuiRenderNode* pRenderNode) : FJ_EnchantingButton(pRenderNode)
{
}

FJ_EnchantingButton* FJ_EnchantingButton_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_EnchantingButton_Small(pRenderNode);
}

unsigned int FJ_EnchantingButton_Small::getType()
{
    return 0x800D;
}
