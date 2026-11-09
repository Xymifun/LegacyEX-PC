#include "game/client/render/CaveSpiderRenderer.hpp"

#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation CAVE_SPIDER_LOCATION(TN_MOB_CAVESPIDER);

CaveSpiderRenderer::CaveSpiderRenderer(EntityRenderDispatcher* dispatcher) : SpiderRenderer(dispatcher)
{
    mShadowRadius *= 0.7F;
}

void CaveSpiderRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(0.7F, 0.7F, 0.7F);
}

const ResourceLocation* CaveSpiderRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &CAVE_SPIDER_LOCATION;
}
