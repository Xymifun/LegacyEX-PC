#include "game/ui/FJ_Label_Black_Small_Centered_HUD.hpp"

FJ_Label* FJ_Label_Black_Small_Centered_HUD::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_Black_Small_Centered_HUD(pRenderNode);
}

unsigned int FJ_Label_Black_Small_Centered_HUD::getType()
{
    return 0x8061;
}
