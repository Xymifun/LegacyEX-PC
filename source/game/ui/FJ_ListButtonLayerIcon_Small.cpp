#include "game/ui/FJ_ListButtonLayerIcon_Small.hpp"

FJ_ListButtonLayerIcon_Small::FJ_ListButtonLayerIcon_Small(fuiRenderNode* renderNode)
    : FJ_ListButtonLayerIcon(renderNode)
{
}

FJ_ListButtonLayerIcon* FJ_ListButtonLayerIcon_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonLayerIcon_Small(renderNode);
}

uint32_t FJ_ListButtonLayerIcon_Small::getType()
{
    return 0x8025;
}
