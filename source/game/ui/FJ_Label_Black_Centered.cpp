#include "game/ui/FJ_Label_Black_Centered.hpp"

FJ_Label* FJ_Label_Black_Centered::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_Black_Centered(pRenderNode);
}

unsigned int FJ_Label_Black_Centered::getType()
{
    return 0x805e;
}
