#ifndef GAME_ENTITY_MOB_INSTANTENOUSMOBEFFECT_HPP
#define GAME_ENTITY_MOB_INSTANTENOUSMOBEFFECT_HPP

#include "game/entity/mob/MobEffect.hpp"

class InstantenousMobEffect : public MobEffect
{
public:
    InstantenousMobEffect(bool harmful, eMinecraftColour color);

    virtual bool isDurationEffectTick(int duration, int amplifier);
    virtual bool isInstantenous();
};

#endif
