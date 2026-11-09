#include "game/client/render/ThrownItemRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemColors.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/ResourceLocation.hpp"

ThrownItemRenderer::ThrownItemRenderer(EntityRenderDispatcher* dispatcher, Item* item)
    : EntityRenderer(dispatcher),
      mItem(item)
{
}

void ThrownItemRenderer::renderIcon(Tesselator* tesselator, TextureAtlasSprite* sprite)
{
    BufferBuilder* builder = tesselator->getBuilder();
    float u0 = sprite->getU0(false);
    float u1 = sprite->getU1(false);
    float v0 = sprite->getV0(false);
    float v1 = sprite->getV1(false);
    GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(-mDispatcher->mCameraXRot, 1.0F, 0.0F, 0.0F);
    builder->begin();
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertexUV(-0.5F, -0.25F, 0.0F, u0, v1);
    builder->vertexUV(0.5F, -0.25F, 0.0F, u1, v1);
    builder->vertexUV(0.5F, 0.75F, 0.0F, u1, v0);
    builder->vertexUV(-0.5F, 0.75F, 0.0F, u0, v0);
    tesselator->end();
}

void ThrownItemRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                float partialTick)
{
    not_null_ptr<ItemInstance> item = getItem(entity);
    Item* itemType = item->getItem();
    TextureAtlasSprite* icon = itemType->getIcon(item);
    if (icon == 0)
        return;

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(0.5F, 0.5F, 0.5F);
    bindTexture(entity);
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }

    Tesselator* tesselator = Tesselator::getInstance();
    if (item->getItem()->hasMultipleSpriteLayers())
    {
        int color = Minecraft::GetInstance()->getItemColors()->getColor(item, 0);
        GlStateManager::color3f(((color >> 16) & 255) / 255.0F, ((color >> 8) & 255) / 255.0F, (color & 255) / 255.0F);
        GlStateManager::pushMatrix();
        renderIcon(tesselator, item->getItem()->getLayerIcon(0, 0, item));
        GlStateManager::popMatrix();
        GlStateManager::color3f(1.0F, 1.0F, 1.0F);
    }
    renderIcon(tesselator, icon);
    if (item->hasFoil())
    {
        GlStateManager::translatef(-0.5F, -0.25F, 0.0F);
        Minecraft::GetInstance()->getItemRenderer()->renderFoilLayer(tesselator);
    }
    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
}

const ResourceLocation* ThrownItemRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &ITEM_LOCATION;
}

not_null_ptr<ItemInstance> ThrownItemRenderer::getItem(boost::shared_ptr<Entity> entity)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(mItem, 1, 0));
}
