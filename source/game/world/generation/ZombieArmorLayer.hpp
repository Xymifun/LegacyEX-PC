#ifndef GAME_WORLD_GENERATION_ZOMBIEARMORLAYER_HPP
#define GAME_WORLD_GENERATION_ZOMBIEARMORLAYER_HPP

#include "game/world/generation/HumanoidArmorLayer.hpp"

class LivingEntityRenderer;

class ZombieArmorLayer : public HumanoidArmorLayer
{
public:
    ZombieArmorLayer(LivingEntityRenderer* renderer, unsigned long long entityType);
    virtual ~ZombieArmorLayer();

    virtual void createArmorModels();

private:
    unsigned long long mEntityType;
};

#endif
