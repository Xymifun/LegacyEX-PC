#include "game/client/render/DragonFireballRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Options.hpp"
#include "game/entity/projectile/DragonFireball.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation DRAGON_FIREBALL_LOCATION(TN_MOB_ENDERDRAGON_DRAGON_FIREBALL);

DragonFireballRenderer::DragonFireballRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
}

DragonFireballRenderer::~DragonFireballRenderer()
{
}

void DragonFireballRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                    float partialTick)
{
    boost::shared_ptr<DragonFireball> fireball = boost::static_pointer_cast<DragonFireball>(entity);
    GlStateManager::pushMatrix();
    bindTexture(entity);
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(2.0F, 2.0F, 2.0F);

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    float r = 1.0F;
    float xo = 0.5F;
    float yo = 0.25F;
    GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef((float)(mDispatcher->mOptions->mThirdPersonView == 2 ? -1 : 1) * -mDispatcher->mCameraXRot,
                            1.0F, 0.0F, 0.0F);

    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }

    builder->begin();
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertexUV(0.0F - xo, 0.0F - yo, 0.0F, 0.0F, 1.0F);
    builder->vertexUV(r - xo, 0.0F - yo, 0.0F, 1.0F, 1.0F);
    builder->vertexUV(r - xo, r - yo, 0.0F, 1.0F, 0.0F);
    builder->vertexUV(0.0F - xo, r - yo, 0.0F, 0.0F, 0.0F);
    builder->end();
    tesselator->end();

    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* DragonFireballRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &DRAGON_FIREBALL_LOCATION;
}
