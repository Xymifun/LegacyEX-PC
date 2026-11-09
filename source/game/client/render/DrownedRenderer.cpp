#include "game/client/render/DrownedRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/DrownedModel.hpp"
#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/core/Drowned.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/ZombieArmorLayer.hpp"

static ResourceLocation DROWNED_LOCATION(TN_MOB_ZOMBIE_DROWNED);

DrownedRenderer::DrownedRenderer(EntityRenderDispatcher* dispatcher)
    : HumanoidMobRenderer(dispatcher, new DrownedModel(), 0.5F)
{
    mArmorLayer = new ZombieArmorLayer(this, EntityType::DROWNED);
    addLayer(mArmorLayer);
}

DrownedRenderer::~DrownedRenderer()
{
    delete mArmorLayer;
}

void DrownedRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(1.0625F, 1.0625F, 1.0625F);
    LivingEntityRenderer::scale(entity, partialTicks);
}

Model* DrownedRenderer::createModel(const ModelDefinition& definition)
{
    return new DrownedModel(definition);
}

const ResourceLocation* DrownedRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &DROWNED_LOCATION;
}

void DrownedRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                     float partialTicks)
{
    boost::shared_ptr<Drowned> drowned = boost::static_pointer_cast<Drowned>(entity);
    LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
}
