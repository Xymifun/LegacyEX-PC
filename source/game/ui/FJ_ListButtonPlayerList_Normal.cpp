#include "game/ui/FJ_ListButtonPlayerList_Normal.hpp"

FJ_ListButtonPlayerList_Normal::FJ_ListButtonPlayerList_Normal(fuiRenderNode* renderNode)
    : FJ_ListButtonPlayerList(renderNode)
{
}

FJ_ListButtonPlayerList* FJ_ListButtonPlayerList_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonPlayerList_Normal(renderNode);
}

uint32_t FJ_ListButtonPlayerList_Normal::getType()
{
    return 0x8028;
}
