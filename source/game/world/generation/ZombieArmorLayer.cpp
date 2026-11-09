#include "game/world/generation/ZombieArmorLayer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ZombieModel.hpp"

ZombieArmorLayer::ZombieArmorLayer(LivingEntityRenderer* renderer, unsigned long long entityType)
    : HumanoidArmorLayer(renderer),
      mEntityType(entityType)
{
    createArmorModels();
}

ZombieArmorLayer::~ZombieArmorLayer()
{
}

void ZombieArmorLayer::createArmorModels()
{
    delete mModelArmorInner;
    delete mModelArmor;

    if (mEntityType == EntityType::DROWNED)
    {
        mModelArmorInner = new ZombieModel(0.75f, true, ModelDefinition());
        mModelArmor = new ZombieModel(1.25f, true, ModelDefinition());
    }
    else
    {
        mModelArmorInner = new ZombieModel(0.5f, true, ModelDefinition());
        mModelArmor = new ZombieModel(1.0f, true, ModelDefinition());
    }
}
