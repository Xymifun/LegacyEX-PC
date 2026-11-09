#include "game/ui/FJ_ListButtonLayerIcon_Normal.hpp"

FJ_ListButtonLayerIcon_Normal::FJ_ListButtonLayerIcon_Normal(fuiRenderNode* pRenderNode)
    : FJ_ListButtonLayerIcon(pRenderNode)
{
}

FJ_ListButtonLayerIcon* FJ_ListButtonLayerIcon_Normal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_ListButtonLayerIcon_Normal(pRenderNode);
}

unsigned int FJ_ListButtonLayerIcon_Normal::getType()
{
    return 0x8024;
}
