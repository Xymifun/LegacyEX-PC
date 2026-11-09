#include "game/client/render/AbstractHorseRenderer.hpp"

#include "game/client/render/LivingEntityRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/core/GlStateManager.hpp"

AbstractHorseRenderer::AbstractHorseRenderer(EntityRenderDispatcher* dispatcher, Model* model, float scale)
    : MobRenderer(dispatcher, model, 0.75f)
{
    mScale = scale;
}

void AbstractHorseRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    GlStateManager::scalef(mScale, mScale, mScale);
    LivingEntityRenderer::scale(entity, partialTicks);
}
