#include "game/ui/FJ_MobEffect_Small.hpp"

FJ_MobEffect_Small::FJ_MobEffect_Small(fuiRenderNode* pRenderNode) : FJ_MobEffect(pRenderNode)
{
}

FJ_MobEffect* FJ_MobEffect_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_MobEffect_Small(pRenderNode);
}

unsigned int FJ_MobEffect_Small::getType()
{
    return 0x62;
}
