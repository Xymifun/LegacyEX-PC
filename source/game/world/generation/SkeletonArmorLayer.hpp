#ifndef GAME_WORLD_GENERATION_SKELETONARMORLAYER_HPP
#define GAME_WORLD_GENERATION_SKELETONARMORLAYER_HPP

#include "game/world/generation/HumanoidArmorLayer.hpp"

class LivingEntityRenderer;

class SkeletonArmorLayer : public HumanoidArmorLayer
{
public:
    SkeletonArmorLayer(LivingEntityRenderer* renderer);
    virtual ~SkeletonArmorLayer();

    virtual void createArmorModels();
};

#endif
