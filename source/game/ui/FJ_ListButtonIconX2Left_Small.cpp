#include "game/ui/FJ_ListButtonIconX2Left_Small.hpp"

FJ_ListButtonIconX2Left_Small::FJ_ListButtonIconX2Left_Small(fuiRenderNode* renderNode)
    : FJ_ListButtonIconLeft(renderNode)
{
}

FJ_ListButtonIconLeft* FJ_ListButtonIconX2Left_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonIconX2Left_Small(renderNode);
}

unsigned int FJ_ListButtonIconX2Left_Small::getType()
{
    return 0x8017;
}
