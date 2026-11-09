#include "game/ui/FJ_ListButton_Medium.hpp"

FJ_ListButton_Medium::FJ_ListButton_Medium(fuiRenderNode* renderNode) : FJ_ListButton(renderNode)
{
}

FJ_ListButton* FJ_ListButton_Medium::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButton_Medium(renderNode);
}

uint32_t FJ_ListButton_Medium::getType()
{
    return 0x8013;
}
