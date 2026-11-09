#include "game/ui/FJ_Label_SkinSelect_Small.hpp"

FJ_Label* FJ_Label_SkinSelect_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_SkinSelect_Small(pRenderNode);
}

unsigned int FJ_Label_SkinSelect_Small::getType()
{
    return 0x807e;
}
