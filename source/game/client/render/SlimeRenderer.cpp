#include "game/client/render/SlimeRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SlimeModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Slime.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/SlimeOuterLayer.hpp"

static ResourceLocation SLIME_LOCATION(TN_MOB_SLIME);

SlimeRenderer::SlimeRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new SlimeModel(16), 0.25f)
{
    addLayer(new SlimeOuterLayer(this));
}

const ResourceLocation* SlimeRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SLIME_LOCATION;
}

void SlimeRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                           float partialTick)
{
    boost::shared_ptr<Slime> slime = boost::static_pointer_cast<Slime>(entity);
    mShadowRadius = 0.25f * static_cast<float>(slime->getSize());
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

void SlimeRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Slime> slime = boost::static_pointer_cast<Slime>(entity);
    GlStateManager::scalef(0.999f, 0.999f, 0.999f);
    float size = static_cast<float>(slime->getSize());
    float s;
    float squish = (slime->mSquishO + (slime->mSquish - slime->mSquishO) * partialTicks) / (size * 0.5f + 1.0f);
    float scale = 1.0f / (squish + 1.0f);
    s = size;
    GlStateManager::scalef(scale * s, 1.0f / scale * s, scale * s);
}

Model* SlimeRenderer::createModel(const ModelDefinition& definition)
{
    return new SlimeModel(16, definition);
}
