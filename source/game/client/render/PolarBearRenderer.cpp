#include "game/client/render/PolarBearRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/PolarBearModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation POLAR_BEAR_LOCATION(TN_MOB_BEAR_POLARBEAR);

PolarBearRenderer::PolarBearRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new PolarBearModel(), 0.7F)
{
}

const ResourceLocation* PolarBearRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &POLAR_BEAR_LOCATION;
}

void PolarBearRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                               float partialTick)
{
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

void PolarBearRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(1.2F, 1.2F, 1.2F);
    LivingEntityRenderer::scale(entity, partialTicks);
}

Model* PolarBearRenderer::createModel(const ModelDefinition& definition)
{
    return new PolarBearModel(definition);
}
