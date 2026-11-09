#include "game/ui/FJ_ListButtonIconLeft_Small.hpp"

FJ_ListButtonIconLeft_Small::FJ_ListButtonIconLeft_Small(fuiRenderNode* renderNode) : FJ_ListButtonIconLeft(renderNode)
{
}

FJ_ListButtonIconLeft* FJ_ListButtonIconLeft_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonIconLeft_Small(renderNode);
}

unsigned int FJ_ListButtonIconLeft_Small::getType()
{
    return 0x8017;
}
