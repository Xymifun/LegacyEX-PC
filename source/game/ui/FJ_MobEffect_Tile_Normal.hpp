#ifndef GAME_UI_FJ_MOBEFFECT_TILE_NORMAL_HPP
#define GAME_UI_FJ_MOBEFFECT_TILE_NORMAL_HPP

#include "game/ui/FJ_MobEffect_Tile.hpp"

class fuiRenderNode;

class FJ_MobEffect_Tile_Normal : public FJ_MobEffect_Tile
{
public:
    FJ_MobEffect_Tile_Normal(fuiRenderNode* pRenderNode);


    static FJ_MobEffect_Tile_Normal* Create(fuiRenderNode* pRenderNode);

    virtual unsigned int getType();
};

#endif
