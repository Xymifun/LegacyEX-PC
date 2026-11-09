#include "game/client/render/AreaEffectCloudRenderer.hpp"

AreaEffectCloudRenderer::AreaEffectCloudRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
}

AreaEffectCloudRenderer::~AreaEffectCloudRenderer()
{
}

const ResourceLocation* AreaEffectCloudRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return NULL;
}
