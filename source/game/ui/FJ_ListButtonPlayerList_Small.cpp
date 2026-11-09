#include "game/ui/FJ_ListButtonPlayerList_Small.hpp"

FJ_ListButtonPlayerList_Small::FJ_ListButtonPlayerList_Small(fuiRenderNode* renderNode)
    : FJ_ListButtonPlayerList(renderNode)
{
}

FJ_ListButtonPlayerList* FJ_ListButtonPlayerList_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonPlayerList_Small(renderNode);
}

uint32_t FJ_ListButtonPlayerList_Small::getType()
{
    return 0x8029;
}
