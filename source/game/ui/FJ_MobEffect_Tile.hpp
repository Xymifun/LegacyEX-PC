#ifndef GAME_UI_FJ_MOBEFFECT_TILE_HPP
#define GAME_UI_FJ_MOBEFFECT_TILE_HPP

#include "game/ui/FJ_FuiNode.hpp"

class FJ_MobEffect_Tile : public FJ_FuiNode
{
public:
    FJ_MobEffect_Tile(fuiRenderNode* pRenderNode);
    virtual ~FJ_MobEffect_Tile() {}
    void Init();
    void SetupEffectType(int iEffectType);
    void SetEffectAlpha(float fAlpha);
};

#endif
