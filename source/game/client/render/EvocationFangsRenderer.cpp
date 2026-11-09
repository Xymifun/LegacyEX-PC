#include "game/client/render/EvocationFangsRenderer.hpp"

#include "game/client/render/EvocationFangsModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/EvocationFangs.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation FANGS_LOCATION(TN_MOB_ILLAGER_FANGS);

EvocationFangsRenderer::EvocationFangsRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher),
      mFangsModel(new EvocationFangsModel())
{
}

EvocationFangsRenderer::~EvocationFangsRenderer()
{
    delete mFangsModel;
}

void EvocationFangsRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                    float partialTick)
{
    boost::shared_ptr<EvocationFangs> fangs = boost::static_pointer_cast<EvocationFangs>(entity);
    float progress = fangs->getAnimationProgress(partialTick);

    if (progress == 0.0F)
    {
        return;
    }

    float scale = 2.0F;
    if (progress > 0.9F)
    {
        scale *= (1.0F - progress) / 0.1F;
    }

    GlStateManager::pushMatrix();
    GlStateManager::disableCull();
    GlStateManager::enableAlphaTest();
    bindTexture(entity);
    GlStateManager::translatef(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    GlStateManager::rotatef(90.0F - fangs->mYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::scalef(-scale, -scale, scale);
    float modelScale = 0.03125F;
    GlStateManager::translatef(0.0F, -0.626F, 0.0F);
    mFangsModel->render(entity, progress, 0.0F, 0.0F, fangs->mYRot, fangs->mXRot, modelScale, true);
    GlStateManager::popMatrix();
    GlStateManager::enableCull();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* EvocationFangsRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &FANGS_LOCATION;
}

Model* EvocationFangsRenderer::createModel(const ModelDefinition& definition)
{
    return new EvocationFangsModel(definition);
}
