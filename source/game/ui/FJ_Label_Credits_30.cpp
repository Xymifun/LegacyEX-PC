#include "game/ui/FJ_Label_Credits_30.hpp"

FJ_Label_Credits_30::~FJ_Label_Credits_30()
{
}

FJ_LabelOutline* FJ_Label_Credits_30::Create(fuiRenderNode* renderNode)
{
    FJ_LabelOutline* outline = new FJ_Label_Credits_30(renderNode);
    return outline;
}

unsigned int FJ_Label_Credits_30::getType()
{
    return 0x8096;
}
