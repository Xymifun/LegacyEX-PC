#include "game/client/render/ZombieRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ZombieModel.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/ZombieArmorLayer.hpp"

static ResourceLocation sZombieLocation(TN_MOB_ZOMBIE);

ZombieRenderer::ZombieRenderer(EntityRenderDispatcher* dispatcher)
    : HumanoidMobRenderer(dispatcher, new ZombieModel(), 0.5F)
{
    addLayer(new ZombieArmorLayer(this, EntityType::ZOMBIE));
}

const ResourceLocation* ZombieRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &sZombieLocation;
}

void ZombieRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                    float partialTicks)
{
    boost::shared_ptr<Zombie> zombie = boost::static_pointer_cast<Zombie>(entity);
    if (zombie->isConverting())
    {
        rotationYaw += (float)(std::cos((double)zombie->mTickCount * 3.25) * 3.1415927410125732 * 0.25);
    }

    LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
}

Model* ZombieRenderer::createModel(const ModelDefinition& definition)
{
    return new ZombieModel(definition);
}
