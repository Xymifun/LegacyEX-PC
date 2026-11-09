#include "game/client/render/ShulkerBulletRenderer.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ShulkerBulletModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/ShulkerBullet.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation SHULKER_BULLET_LOCATION(TN_MOB_SHULKER_SPARK);

ShulkerBulletRenderer::ShulkerBulletRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher),
      mBulletModel(new ShulkerBulletModel())
{
}

ShulkerBulletRenderer::~ShulkerBulletRenderer()
{
}

float ShulkerBulletRenderer::rotlerp(float previousRotation, float rotation, float partialTick)
{
    float difference = rotation - previousRotation;
    while (difference < -180.0F)
        difference += 360.0F;
    while (difference >= 180.0F)
        difference -= 360.0F;
    return previousRotation + partialTick * difference;
}

void ShulkerBulletRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                   float partialTick)
{
    boost::shared_ptr<ShulkerBullet> bullet = boost::static_pointer_cast<ShulkerBullet>(entity);

    GlStateManager::pushMatrix();
    float yRot = rotlerp(bullet->mYRotO, bullet->mYRot, partialTick);
    float xRot = bullet->mXRotO + (bullet->mXRot - bullet->mXRotO) * partialTick;
    float animation = bullet->mTickCount + partialTick;

    GlStateManager::translatef((float)x, (float)y + 0.15F, (float)z);
    GlStateManager::rotatef((float)std::sin(animation * 0.1F) * 180.0F, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef((float)std::cos(animation * 0.1F) * 180.0F, 1.0F, 0.0F, 0.0F);
    GlStateManager::rotatef((float)std::sin(animation * 0.15F) * 360.0F, 0.0F, 0.0F, 1.0F);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);

    bindTexture(entity);
    mBulletModel->render(entity, 0.0F, 0.0F, 0.0F, yRot, xRot, 0.03125F, true);

    GlStateManager::enableBlend();
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 0.5F);
    GlStateManager::scalef(1.5F, 1.5F, 1.5F);
    mBulletModel->render(entity, 0.0F, 0.0F, 0.0F, yRot, xRot, 0.03125F, true);
    GlStateManager::disableBlend();
    GlStateManager::popMatrix();

    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* ShulkerBulletRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &SHULKER_BULLET_LOCATION;
}

Model* ShulkerBulletRenderer::createModel(const ModelDefinition& definition)
{
    return new ShulkerBulletModel(definition);
}
