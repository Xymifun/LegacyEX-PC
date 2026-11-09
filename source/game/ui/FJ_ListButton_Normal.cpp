#include "game/ui/FJ_ListButton_Normal.hpp"

FJ_ListButton_Normal::FJ_ListButton_Normal(fuiRenderNode* renderNode) : FJ_ListButton(renderNode)
{
}

FJ_ListButton* FJ_ListButton_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButton_Normal(renderNode);
}

uint32_t FJ_ListButton_Normal::getType()
{
    return 0x8014;
}
