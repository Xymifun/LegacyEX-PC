#include "game/ui/FJ_Label_White_VerySmall_Centered.hpp"

FJ_Label* FJ_Label_White_VerySmall_Centered::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_White_VerySmall_Centered(pRenderNode);
}

unsigned int FJ_Label_White_VerySmall_Centered::getType()
{
    return 0x808d;
}
