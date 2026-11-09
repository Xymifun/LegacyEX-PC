#include "game/ui/FJ_ListButtonCheckbox_Normal.hpp"

FJ_ListButtonCheckbox_Normal::FJ_ListButtonCheckbox_Normal(fuiRenderNode* renderNode)
    : FJ_ListButtonCheckbox(renderNode)
{
}

FJ_ListButtonCheckbox* FJ_ListButtonCheckbox_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonCheckbox_Normal(renderNode);
}

uint32_t FJ_ListButtonCheckbox_Normal::getType()
{
    return 0x8020;
}
