#include "game/ui/FJ_Label_Credits_10.hpp"

FJ_Label_Credits_10::~FJ_Label_Credits_10()
{
}

FJ_LabelOutline* FJ_Label_Credits_10::Create(fuiRenderNode* renderNode)
{
    FJ_LabelOutline* outline = new FJ_Label_Credits_10(renderNode);
    return outline;
}

unsigned int FJ_Label_Credits_10::getType()
{
    return 0x8092;
}
