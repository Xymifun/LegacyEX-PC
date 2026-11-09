#ifndef GAME_UI_FJ_MOBEFFECT_SMALL_HPP
#define GAME_UI_FJ_MOBEFFECT_SMALL_HPP

#include "game/ui/FJ_MobEffect.hpp"

class fuiRenderNode;

class FJ_MobEffect_Small : public FJ_MobEffect
{
public:
    FJ_MobEffect_Small(fuiRenderNode* pRenderNode);
    static FJ_MobEffect* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
