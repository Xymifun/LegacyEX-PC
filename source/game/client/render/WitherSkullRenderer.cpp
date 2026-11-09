#include "game/client/render/WitherSkullRenderer.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/WitherSkull.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation WITHER_SKULL_LOCATION(TN_MOB_WITHER_WITHER);
static ResourceLocation WITHER_SKULL_INVULNERABLE_LOCATION(TN_MOB_WITHER_WITHER_INVULNERABLE);

WitherSkullRenderer::WitherSkullRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher),
      mWitherSkullModel(new SkeletonHeadModel())
{
}

WitherSkullRenderer::~WitherSkullRenderer()
{
}

float WitherSkullRenderer::rotlerp(float from, float to, float partialTick)
{
    float rotation = to - from;
    while (rotation < -180.0F)
        rotation += 360.0F;
    while (rotation >= 180.0F)
        rotation -= 360.0F;
    return from + partialTick * rotation;
}

void WitherSkullRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                 float partialTick)
{
    GlStateManager::pushMatrix();
    GlStateManager::disableCull();

    float interpolatedYaw = rotlerp(entity->mYRotO, entity->mYRot, partialTick);
    float interpolatedPitch = entity->mXRotO + (entity->mXRot - entity->mXRotO) * partialTick;

    GlStateManager::translatef((float)x, (float)y, (float)z);
    float scale = 0.0625F;
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
    GlStateManager::enableAlphaTest();
    bindTexture(entity);

    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine(getTeamColor(entity));
    }

    mWitherSkullModel->render(entity, 0.0F, 0.0F, 0.0F, interpolatedYaw, interpolatedPitch, scale, true);

    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }

    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* WitherSkullRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<WitherSkull> skull = boost::static_pointer_cast<WitherSkull>(entity);
    return skull->isDangerous() ? &WITHER_SKULL_INVULNERABLE_LOCATION : &WITHER_SKULL_LOCATION;
}

Model* WitherSkullRenderer::createModel(const ModelDefinition& definition)
{
    return new SkeletonHeadModel(definition);
}
