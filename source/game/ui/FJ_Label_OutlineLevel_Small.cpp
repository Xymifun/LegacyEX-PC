#include "game/ui/FJ_Label_OutlineLevel_Small.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label_OutlineLevel_Small::~FJ_Label_OutlineLevel_Small()
{
}

FJ_LabelOutline* FJ_Label_OutlineLevel_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_OutlineLevel_Small(renderNode);
}

unsigned int FJ_Label_OutlineLevel_Small::getType()
{
    return 0x809d;
}
