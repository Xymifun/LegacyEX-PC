#include "game/client/render/DolphinRenderer.hpp"

#include "game/client/render/DolphinModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/DolphinCarryingItemLayer.hpp"

static ResourceLocation* DOLPHIN_LOCATION = new ResourceLocation(TN_MOB_DOLPHIN);

DolphinRenderer::DolphinRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new DolphinModel(), 0.7F)
{
    addLayer(new DolphinCarryingItemLayer(this));
}

const ResourceLocation* DolphinRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return DOLPHIN_LOCATION;
}

void DolphinRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(1.0F, 1.0F, 1.0F);
}

Model* DolphinRenderer::createModel(const ModelDefinition& definition)
{
    return new DolphinModel(definition);
}
