#include "game/ui/FJ_MobEffect_Normal.hpp"

FJ_MobEffect_Normal::FJ_MobEffect_Normal(fuiRenderNode* pRenderNode) : FJ_MobEffect(pRenderNode)
{
}

FJ_MobEffect* FJ_MobEffect_Normal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_MobEffect_Normal(pRenderNode);
}

unsigned int FJ_MobEffect_Normal::getType()
{
    return 97;
}
