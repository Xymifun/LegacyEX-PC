#ifndef GAME_WORLD_GENERATION_VILLAGERARMORLAYER_HPP
#define GAME_WORLD_GENERATION_VILLAGERARMORLAYER_HPP

#include "game/world/generation/HumanoidArmorLayer.hpp"

class LivingEntityRenderer;

class VillagerArmorLayer : public HumanoidArmorLayer
{
public:
    VillagerArmorLayer(LivingEntityRenderer* renderer);
    virtual ~VillagerArmorLayer();

    virtual void createArmorModels();
};

#endif
