#include "game/ui/FJ_Label_Credits_10_Yellow.hpp"

FJ_Label_Credits_10_Yellow::~FJ_Label_Credits_10_Yellow()
{
}

FJ_LabelOutline* FJ_Label_Credits_10_Yellow::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_Credits_10_Yellow(pRenderNode);
}

unsigned int FJ_Label_Credits_10_Yellow::getType()
{
    return 0x8093;
}
