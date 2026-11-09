#include "game/ui/FJ_MobEffect_Tile_Normal.hpp"

FJ_MobEffect_Tile_Normal::FJ_MobEffect_Tile_Normal(fuiRenderNode* pRenderNode) : FJ_MobEffect_Tile(pRenderNode)
{
}

FJ_MobEffect_Tile_Normal* FJ_MobEffect_Tile_Normal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_MobEffect_Tile_Normal(pRenderNode);
}

unsigned int FJ_MobEffect_Tile_Normal::getType()
{
    return 99;
}
