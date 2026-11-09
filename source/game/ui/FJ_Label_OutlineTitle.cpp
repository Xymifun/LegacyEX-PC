#include "game/ui/FJ_Label_OutlineTitle.hpp"

FJ_Label_OutlineTitle::~FJ_Label_OutlineTitle()
{
}

FJ_LabelOutline* FJ_Label_OutlineTitle::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_OutlineTitle(renderNode);
}

unsigned int FJ_Label_OutlineTitle::getType()
{
    return 0x809e;
}
