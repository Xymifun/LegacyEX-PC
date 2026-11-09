#include "game/ui/FJ_Label_Credits_20.hpp"

FJ_Label_Credits_20::~FJ_Label_Credits_20()
{
}

FJ_LabelOutline* FJ_Label_Credits_20::Create(fuiRenderNode* renderNode)
{
    FJ_LabelOutline* outline = new FJ_Label_Credits_20(renderNode);
    return outline;
}

unsigned int FJ_Label_Credits_20::getType()
{
    return 0x8094;
}
