#include "game/world/generation/EnderDragonEyesLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/EnderDragonRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ENDER_DRAGON_EYES_LOCATION(TN_MOB_ENDERDRAGON_ENDER_EYES);

EnderDragonEyesLayer::EnderDragonEyesLayer(EnderDragonRenderer* renderer) : mRenderer(renderer)
{
}

EnderDragonEyesLayer::~EnderDragonEyesLayer()
{
}

void EnderDragonEyesLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                  float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                  bool bUseDisplayList)
{
    mRenderer->bindTexture(&ENDER_DRAGON_EYES_LOCATION);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(1, 1);
    GlStateManager::disableLighting();
    GlStateManager::depthFunc(2);
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

    GlStateManager::disableBlend();
    GlStateManager::depthFunc(3);
}

bool EnderDragonEyesLayer::colorsOnDamage()
{
    return false;
}
