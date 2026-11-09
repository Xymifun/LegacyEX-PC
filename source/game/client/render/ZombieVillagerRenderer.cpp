#include "game/client/render/ZombieVillagerRenderer.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/VillagerZombieModel.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/VillagerArmorLayer.hpp"
#include "game/world/generation/ZombieVillager.hpp"

static ResourceLocation ZOMBIE_VILLAGER_LOCATION(TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_VILLAGER);
static ResourceLocation ZOMBIE_VILLAGER_FARMER_LOCATION(TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_FARMER);
static ResourceLocation ZOMBIE_VILLAGER_LIBRARIAN_LOCATION(TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_LIBRARIAN);
static ResourceLocation ZOMBIE_VILLAGER_PRIEST_LOCATION(TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_PRIEST);
static ResourceLocation ZOMBIE_VILLAGER_SMITH_LOCATION(TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_SMITH);
static ResourceLocation ZOMBIE_VILLAGER_BUTCHER_LOCATION(TN_MOB_ZOMBIE_VILLAGER_ZOMBIE_BUTCHER);

ZombieVillagerRenderer::ZombieVillagerRenderer(EntityRenderDispatcher* dispatcher)
    : HumanoidMobRenderer(dispatcher, new VillagerZombieModel(), 0.5F)
{
    addLayer(new VillagerArmorLayer(this));
}

const ResourceLocation* ZombieVillagerRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<ZombieVillager> zombieVillager = boost::static_pointer_cast<ZombieVillager>(entity);
    switch (zombieVillager->getProfession())
    {
    case 0:
        return &ZOMBIE_VILLAGER_FARMER_LOCATION;
    case 1:
        return &ZOMBIE_VILLAGER_LIBRARIAN_LOCATION;
    case 2:
        return &ZOMBIE_VILLAGER_PRIEST_LOCATION;
    case 3:
        return &ZOMBIE_VILLAGER_SMITH_LOCATION;
    case 4:
        return &ZOMBIE_VILLAGER_BUTCHER_LOCATION;
    default:
        return &ZOMBIE_VILLAGER_LOCATION;
    }
}

void ZombieVillagerRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks,
                                            float rotationYaw, float partialTicks)
{
    boost::shared_ptr<ZombieVillager> zombieVillager = boost::static_pointer_cast<ZombieVillager>(entity);
    if (zombieVillager->isConverting())
    {
        rotationYaw += (float)(std::cos((double)zombieVillager->mTickCount * 3.25) * 3.1415927410125732 * 0.25);
    }

    LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
}

Model* ZombieVillagerRenderer::createModel(const ModelDefinition& definition)
{
    return new VillagerZombieModel(definition);
}
