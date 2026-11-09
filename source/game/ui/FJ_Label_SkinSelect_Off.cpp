#include "game/ui/FJ_Label_SkinSelect_Off.hpp"

FJ_Label* FJ_Label_SkinSelect_Off::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_SkinSelect_Off(pRenderNode);
}

unsigned int FJ_Label_SkinSelect_Off::getType()
{
    return 0x807d;
}
