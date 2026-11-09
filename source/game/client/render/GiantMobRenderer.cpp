#include "game/client/render/GiantMobRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ZombieModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/ItemInHandLayer.hpp"
#include "game/world/generation/ZombieArmorLayer.hpp"

static ResourceLocation GIANT_LOCATION(TN_ITEM_ARROWS);

GiantMobRenderer::GiantMobRenderer(EntityRenderDispatcher* dispatcher, float shadowRadius)
    : MobRenderer(dispatcher, new ZombieModel(), shadowRadius * 0.5F)
{
    mScale = shadowRadius;
    addLayer(new ItemInHandLayer(this));
    addLayer(new ZombieArmorLayer(this, EntityType::ZOMBIE));
}

void GiantMobRenderer::translateWeaponItem()
{
    GlStateManager::translatef(0.0F, 0.1875F, 0.0F);
}

void GiantMobRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(mScale, mScale, mScale);
}

const ResourceLocation* GiantMobRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &GIANT_LOCATION;
}

Model* GiantMobRenderer::createModel(const ModelDefinition& definition)
{
    return new ZombieModel(definition);
}
