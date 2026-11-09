#include "game/ui/FJ_MobEffect_Tile_Small.hpp"

FJ_MobEffect_Tile_Small::FJ_MobEffect_Tile_Small(fuiRenderNode* pRenderNode) : FJ_MobEffect_Tile(pRenderNode)
{
}

FJ_MobEffect_Tile_Small* FJ_MobEffect_Tile_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_MobEffect_Tile_Small(pRenderNode);
}

unsigned int FJ_MobEffect_Tile_Small::getType()
{
    return 100;
}
