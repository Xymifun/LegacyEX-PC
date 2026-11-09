#include "game/client/render/ArrowRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ARROW_LOCATION(TN_ITEM_FIREWORK);

ArrowRenderer::ArrowRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
}

void ArrowRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                           float partialTick)
{
    float yRot = entity.get()->mYRot;
    float xRot = entity.get()->mXRot;
    float yRotO = entity.get()->mYRotO;
    float xRotO = entity.get()->mXRotO;
    float shake = 0.0F;

    if (eTYPE_DERIVED_FROM(EntityType::ARROW, entity.get()->GetType()))
    {
        boost::shared_ptr<Arrow> arrow = boost::static_pointer_cast<Arrow>(entity);
        shake = (float)arrow.get()->mShake - partialTick;
    }

    bindTexture(entity);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    GlStateManager::pushMatrix();

    if (yRot - yRotO > 180.0F)
    {
        yRot -= 360.0F;
    }
    else if (yRot - yRotO < -180.0F)
    {
        yRot += 360.0F;
    }

    if (xRot - xRotO > 180.0F)
    {
        xRot -= 360.0F;
    }
    else if (xRot - xRotO < -180.0F)
    {
        xRot += 360.0F;
    }

    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::rotatef(yRotO + (yRot - yRotO) * partialTick - 90.0F, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(xRotO + (xRot - xRotO) * partialTick, 0.0F, 0.0F, 1.0F);

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();

    int type = 0;
    float u0 = 0 / 32.0F;
    float u1 = 16 / 32.0F;
    float v0 = (0 + type * 10) / 32.0F;
    float v1 = (5 + type * 10) / 32.0F;

    float sideU0 = 0 / 32.0F;
    float sideU1 = 5 / 32.0F;
    float sideV0 = (5 + type * 10) / 32.0F;
    float sideV1 = (10 + type * 10) / 32.0F;
    float scale = 0.9F / 16.0F;

    GlStateManager::enableRescaleNormal();

    if (shake > 0.0F)
    {
        GlStateManager::rotatef(-Mth::sin(shake * 3.0F) * shake, 0.0F, 0.0F, 1.0F);
    }

    GlStateManager::rotatef(45.0F, 1.0F, 0.0F, 0.0F);
    GlStateManager::scalef(scale, scale, scale);
    GlStateManager::translatef(-4.0F, 0.0F, 0.0F);

    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }

    builder->begin();
    builder->normal(1.0F, 0.0F, 0.0F);
    builder->vertexUV(-7.0F, -2.0F, -2.0F, sideU0, sideV0);
    builder->vertexUV(-7.0F, -2.0F, 2.0F, sideU1, sideV0);
    builder->vertexUV(-7.0F, 2.0F, 2.0F, sideU1, sideV1);
    builder->vertexUV(-7.0F, 2.0F, -2.0F, sideU0, sideV1);
    builder->end();

    builder->begin();
    builder->normal(-1.0F, 0.0F, 0.0F);
    builder->vertexUV(-7.0F, 2.0F, -2.0F, sideU0, sideV0);
    builder->vertexUV(-7.0F, 2.0F, 2.0F, sideU1, sideV0);
    builder->vertexUV(-7.0F, -2.0F, 2.0F, sideU1, sideV1);
    builder->vertexUV(-7.0F, -2.0F, -2.0F, sideU0, sideV1);
    builder->end();

    for (int side = 0; side < 4; ++side)
    {
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        builder->begin();
        builder->normal(0.0F, 0.0F, 1.0F);
        builder->vertexUV(-8.0F, -2.0F, 0.0F, u0, v0);
        builder->vertexUV(8.0F, -2.0F, 0.0F, u1, v0);
        builder->vertexUV(8.0F, 2.0F, 0.0F, u1, v1);
        builder->vertexUV(-8.0F, 2.0F, 0.0F, u0, v1);
        builder->end();
    }

    GlStateManager::disableRescaleNormal();
    GlStateManager::enableLighting();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* ArrowRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &ARROW_LOCATION;
}
