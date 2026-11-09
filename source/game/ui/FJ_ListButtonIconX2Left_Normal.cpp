#include "game/ui/FJ_ListButtonIconX2Left_Normal.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_ListButtonIconX2Left_Normal::FJ_ListButtonIconX2Left_Normal(fuiRenderNode* renderNode)
    : FJ_ListButtonIconLeft(renderNode)
{
}

FJ_ListButtonIconLeft* FJ_ListButtonIconX2Left_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonIconX2Left_Normal(renderNode);
}

uint32_t FJ_ListButtonIconX2Left_Normal::getType()
{
    return 0x8016;
}
