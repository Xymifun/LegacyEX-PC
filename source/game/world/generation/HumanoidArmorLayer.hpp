#ifndef GAME_WORLD_GENERATION_HUMANOIDARMORLAYER_HPP
#define GAME_WORLD_GENERATION_HUMANOIDARMORLAYER_HPP

#include "game/world/generation/AbstractArmorLayer.hpp"

class EquipmentSlot;
class LivingEntityRenderer;
class Model;

class HumanoidArmorLayer : public AbstractArmorLayer
{
public:
    HumanoidArmorLayer(LivingEntityRenderer* renderer);
    virtual ~HumanoidArmorLayer() {}

    virtual void createArmorModels();
    virtual void setPartVisibility(Model* model, const EquipmentSlot* slot);
    virtual void hideAllArmor(Model* model);
};

#endif
