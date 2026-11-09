#include "game/ui/FJ_ListButtonDLC_Small.hpp"

FJ_ListButtonDLC_Small::FJ_ListButtonDLC_Small(fuiRenderNode* renderNode) : FJ_ListButtonDLC(renderNode)
{
}

FJ_ListButtonDLC* FJ_ListButtonDLC_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonDLC_Small(renderNode);
}

unsigned int FJ_ListButtonDLC_Small::getType()
{
    return 0x8023;
}
