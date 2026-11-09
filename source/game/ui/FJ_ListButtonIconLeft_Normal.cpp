#include "game/ui/FJ_ListButtonIconLeft_Normal.hpp"

FJ_ListButtonIconLeft_Normal::FJ_ListButtonIconLeft_Normal(fuiRenderNode* renderNode)
    : FJ_ListButtonIconLeft(renderNode)
{
}

FJ_ListButtonIconLeft* FJ_ListButtonIconLeft_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonIconLeft_Normal(renderNode);
}

unsigned int FJ_ListButtonIconLeft_Normal::getType()
{
    return 0x8016;
}
