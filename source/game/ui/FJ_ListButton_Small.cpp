#include "game/ui/FJ_ListButton_Small.hpp"

FJ_ListButton_Small::FJ_ListButton_Small(fuiRenderNode* renderNode) : FJ_ListButton(renderNode)
{
}

FJ_ListButton* FJ_ListButton_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButton_Small(renderNode);
}

unsigned int FJ_ListButton_Small::getType()
{
    return 0x8015;
}
