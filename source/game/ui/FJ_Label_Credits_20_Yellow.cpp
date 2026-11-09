#include "game/ui/FJ_Label_Credits_20_Yellow.hpp"

FJ_Label_Credits_20_Yellow::~FJ_Label_Credits_20_Yellow()
{
}

FJ_LabelOutline* FJ_Label_Credits_20_Yellow::Create(fuiRenderNode* renderNode)
{
    FJ_LabelOutline* outline = new FJ_Label_Credits_20_Yellow(renderNode);
    return outline;
}

unsigned int FJ_Label_Credits_20_Yellow::getType()
{
    return 0x8095;
}
