#include "game/ui/FJ_Label_OutlineTitle_Small.hpp"

FJ_Label_OutlineTitle_Small::~FJ_Label_OutlineTitle_Small()
{
}

FJ_LabelOutline* FJ_Label_OutlineTitle_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_OutlineTitle_Small(renderNode);
}

unsigned int FJ_Label_OutlineTitle_Small::getType()
{
    return 0x809f;
}
