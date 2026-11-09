#include "game/world/generation/EnderEyesLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/EndermanRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ENDER_EYES_LOCATION(TN_MOB_ENDERMAN_EYES);

EnderEyesLayer::EnderEyesLayer(EndermanRenderer* renderer) : mRenderer(renderer)
{
}

EnderEyesLayer::~EnderEyesLayer()
{
}

void EnderEyesLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                            float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                            bool bUseDisplayList)
{
    mRenderer->bindTexture(&ENDER_EYES_LOCATION);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(1, 1);
    GlStateManager::disableLighting();
    GlStateManager::depthMask(!entity->isInvisible());
    int lightColor = 61680;
    int lightU = lightColor % 65536;
    int lightV = lightColor / 65536;
    GlStateManager::multiTexCoord2f((float)lightU, (float)lightV);
    GlStateManager::enableLighting();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    boost::shared_ptr<Entity> renderEntity = entity;
    mRenderer->getModel()->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
                                  bUseDisplayList);
    mRenderer->setLightColor(renderEntity);

    GlStateManager::depthMask(true);
    GlStateManager::disableBlend();
    GlStateManager::enableAlphaTest();
}

bool EnderEyesLayer::colorsOnDamage()
{
    return false;
}
