#ifndef GAME_UI_FJ_MOBEFFECT_NORMAL_HPP
#define GAME_UI_FJ_MOBEFFECT_NORMAL_HPP

#include "game/ui/FJ_MobEffect.hpp"

class fuiRenderNode;

class FJ_MobEffect_Normal : public FJ_MobEffect
{
public:
    FJ_MobEffect_Normal(fuiRenderNode* pRenderNode);
    static FJ_MobEffect* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
