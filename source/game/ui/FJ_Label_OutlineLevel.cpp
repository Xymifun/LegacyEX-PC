#include "game/ui/FJ_Label_OutlineLevel.hpp"

FJ_Label_OutlineLevel::~FJ_Label_OutlineLevel()
{
}

FJ_LabelOutline* FJ_Label_OutlineLevel::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_OutlineLevel(renderNode);
}

unsigned int FJ_Label_OutlineLevel::getType()
{
    return 0x809c;
}
