#include "game/world/generation/VillagerArmorLayer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/VillagerZombieModel.hpp"

VillagerArmorLayer::VillagerArmorLayer(LivingEntityRenderer* renderer) : HumanoidArmorLayer(renderer)
{
    createArmorModels();
}

VillagerArmorLayer::~VillagerArmorLayer()
{
}

void VillagerArmorLayer::createArmorModels()
{
    if (mModelArmorInner != NULL)
        delete mModelArmorInner;
    if (mModelArmor != NULL)
        delete mModelArmor;

    mModelArmorInner = new VillagerZombieModel(0.5f, 0.0f, true, ModelDefinition());
    mModelArmor = new VillagerZombieModel(1.0f, 0.0f, true, ModelDefinition());
}
