#include "game/ui/FJ_ListButtonDLC_Normal.hpp"

FJ_ListButtonDLC_Normal::FJ_ListButtonDLC_Normal(fuiRenderNode* renderNode) : FJ_ListButtonDLC(renderNode)
{
}

FJ_ListButtonDLC* FJ_ListButtonDLC_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonDLC_Normal(renderNode);
}

uint32_t FJ_ListButtonDLC_Normal::getType()
{
    return 0x8022;
}
