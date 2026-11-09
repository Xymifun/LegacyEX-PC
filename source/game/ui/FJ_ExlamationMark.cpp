#include "game/ui/FJ_ExlamationMark.hpp"

FJ_FuiNode* FJ_ExlamationMark::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_ExlamationMark(pRenderNode);
}

unsigned int FJ_ExlamationMark::getType()
{
    return 0x60;
}
