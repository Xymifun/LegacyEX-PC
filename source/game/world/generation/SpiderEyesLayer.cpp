#include "game/world/generation/SpiderEyesLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/SpiderRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation SPIDER_EYES_LOCATION(TN_MOB_SPIDER_EYES);

SpiderEyesLayer::SpiderEyesLayer(SpiderRenderer* renderer) : mRenderer(renderer)
{
}

SpiderEyesLayer::~SpiderEyesLayer()
{
}

void SpiderEyesLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                             float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                             bool bUseDisplayList)
{
    mRenderer->bindTexture(&SPIDER_EYES_LOCATION);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(1, 1);
    if (entity->isInvisible())
    {
        GlStateManager::depthMask(false);
    }
    else
    {
        GlStateManager::depthMask(true);
    }
    GlStateManager::multiTexCoord2f(61680.0f, 0.0f);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    boost::shared_ptr<Entity> renderEntity(entity);
    mRenderer->getModel()->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
                                  bUseDisplayList);

    int lightColor = entity->getLightColor();
    GlStateManager::multiTexCoord2f((float)(lightColor % 65536), (float)(lightColor / 65536));
    mRenderer->setLightColor(renderEntity);

    GlStateManager::disableBlend();
    GlStateManager::enableAlphaTest();
}

bool SpiderEyesLayer::colorsOnDamage()
{
    return false;
}
