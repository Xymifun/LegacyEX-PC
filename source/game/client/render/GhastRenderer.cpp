#include "game/client/render/GhastRenderer.hpp"

#include "game/client/render/GhastModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Ghast.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation GHAST_LOCATION(TN_MOB_GHAST);
static ResourceLocation GHAST_SHOOTING_LOCATION(TN_MOB_GHAST_FIRE);

GhastRenderer::GhastRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new GhastModel(), 0.5F)
{
}

const ResourceLocation* GhastRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Ghast> ghast = boost::static_pointer_cast<Ghast>(entity);
    if (ghast->isCharging())
        return &GHAST_SHOOTING_LOCATION;

    return &GHAST_LOCATION;
}

Model* GhastRenderer::createModel(const ModelDefinition& definition)
{
    return new GhastModel(definition);
}

void GhastRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(4.5F, 4.5F, 4.5F);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
}
