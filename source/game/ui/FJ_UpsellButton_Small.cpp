#include "game/ui/FJ_UpsellButton_Small.hpp"

FJ_UpsellButton_Small::FJ_UpsellButton_Small(fuiRenderNode* renderNode) : FJ_UpsellButton(renderNode)
{
}

FJ_UpsellButton* FJ_UpsellButton_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_UpsellButton_Small(renderNode);
}

uint32_t FJ_UpsellButton_Small::getType()
{
    return 0x802e;
}
