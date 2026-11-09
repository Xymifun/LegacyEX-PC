#include "game/client/render/DefaultRenderer.hpp"

#include "game/core/GlStateManager.hpp"

DefaultRenderer::DefaultRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
}

DefaultRenderer::~DefaultRenderer()
{
}

void DefaultRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                             float partialTick)
{
    GlStateManager::pushMatrix();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* DefaultRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return NULL;
}
