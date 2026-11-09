#include "game/client/render/FireballRenderer.hpp"

#include <cmath>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Options.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/projectile/Fireball.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FireBlock.hpp"

FireballRenderer::FireballRenderer(EntityRenderDispatcher* dispatcher, float scale)
    : EntityRenderer(dispatcher),
      mScale(scale)
{
}

FireballRenderer::~FireballRenderer()
{
}

void FireballRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                              float partialTick)
{
    boost::shared_ptr<Fireball> fireball = boost::static_pointer_cast<Fireball>(entity);

    GlStateManager::pushMatrix();
    MemSect(0x1f);
    bindTexture(entity);
    MemSect(0);
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(mScale, mScale, mScale);

    TextureAtlasSprite* sprite = Items::fireCharge->getIcon(fireball->GetType() == EntityType::DRAGON_FIREBALL);

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    float u0 = sprite->getU0(false);
    float u1 = sprite->getU1(false);
    float v0 = sprite->getV0(false);
    float v1 = sprite->getV1(false);
    float r = 1.0F;
    float xo = 0.5F;
    float yo = 0.25F;
    GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef((float)(mDispatcher->mOptions->mThirdPersonView == 2 ? -1 : 1) * -mDispatcher->mCameraXRot,
                            1.0F, 0.0F, 0.0F);
    if (mSolidRender)
        GlStateManager::enableColorMaterial();

    builder->begin();
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertexUV(0.0F - xo, 0.0F - yo, 0.0F, u0, v1);
    builder->vertexUV(r - xo, 0.0F - yo, 0.0F, u1, v1);
    builder->vertexUV(r - xo, r - yo, 0.0F, u1, v0);
    builder->vertexUV(0.0F - xo, r - yo, 0.0F, u0, v0);
    builder->end();
    if (mSolidRender)
        GlStateManager::disableColorMaterial();
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

void FireballRenderer::renderFlame(boost::shared_ptr<Entity>& entity, double x, double y, double z, float partialTick)
{
    GlStateManager::disableLighting();
    FireBlock* fire = static_cast<FireBlock*>(Blocks::fire);
    TextureAtlasSprite* sprite = fire->getTextureLayer(0);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    const float scale = entity->mBbWidth * 1.4F;
    GlStateManager::scalef(scale, scale, scale);
    MemSect(0x1f);
    bindTexture(&BLOCKS_LOCATION);
    MemSect(0);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    float r = 0.5F;
    float xo = 0.0F;
    float yo = (float)(entity->mY - entity->mBoundingBox->mMinY);
    GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(-mDispatcher->mCameraXRot, 1.0F, 0.0F, 0.0F);
    GlStateManager::translatef(0.0F, 0.0F, 0.1F);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    builder->begin();
    builder->normal(0.0F, 1.0F, 0.0F);
    float u0 = sprite->getU0(false);
    float v0 = sprite->getV0(false);
    float u1 = sprite->getU1(false);
    float v1 = sprite->getV1(false);
    builder->vertexUV(xo - r, 0.0F - yo, 0.0F, u0, v1);
    builder->vertexUV(r - xo, 0.0F - yo, 0.0F, u1, v1);
    builder->vertexUV(r - xo, 1.4F - yo, 0.0F, u1, v0);
    builder->vertexUV(xo - r, 1.4F - yo, 0.0F, u0, v0);
    builder->end();
    GlStateManager::popMatrix();
    GlStateManager::enableLighting();
}

const ResourceLocation* FireballRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &ITEM_LOCATION;
}
