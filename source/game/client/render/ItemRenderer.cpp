#include <cmath>
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/EntityBlockRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/ThrownTridentRenderer.hpp"
#include "game/client/render/Font.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Options.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/StringConversion.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemColors.hpp"
#include "game/item/ItemCooldowns.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/item/ItemId.hpp"

extern Renderer* gRenderer;

ResourceLocation ItemRenderer::ENCHANT_GLINT_LOCATION(TN_BLUR_MISC_GLINT);

static int sFoilList = -1;
static const int sPotionStrengthBarWidths[] = {3, 6, 9, 11};

ItemRenderer::ItemRenderer(EntityRenderDispatcher* dispatcher, Textures* textures, ItemColors* itemColors)
    : EntityRenderer(dispatcher)
{
    mRandom = new Random();
    mRenderWithColor = true;
    mDepth = 0.0F;
    mTextures = textures;
    mItemFrameEnabled = false;
    mShadowRadius = 0.15F;
    mShadowStrength = 0.75F;
    mItemColors = itemColors;

    if (sFoilList == -1)
    {
        sFoilList = MemoryTracker::genLists(1);
        GlStateManager::newList(sFoilList, 0);
        GlStateManager::depthFunc(2);
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->begin();
        const float step = 0.0625F;
        const float depth = -0.0625F;
        for (int row = 0; row < 16; ++row)
        {
            float y0 = row * step;
            float v0 = (15 - row) * step;
            for (int column = 0; column < 16; ++column)
            {
                float y1 = y0 + step;
                float v1 = v0 - step;
                float x0 = column * step;
                float x1 = x0 + step;
                float u0 = (15 - column) * step;
                float u1 = u0 - step;

                builder->color(0.5F, 0.25F, 0.8F, 1.0F);
                builder->normal(0.0F, 0.0F, 1.0F);
                builder->vertexUV(x0, y0, 0.0F, u0, v0);
                builder->vertexUV(x1, y0, 0.0F, u1, v0);
                builder->vertexUV(x1, y1, 0.0F, u1, v1);
                builder->vertexUV(x0, y1, 0.0F, u0, v1);

                builder->normal(0.0F, 0.0F, -1.0F);
                builder->vertexUV(x0, y1, depth, u0, v1);
                builder->vertexUV(x1, y1, depth, u1, v1);
                builder->vertexUV(x1, y0, depth, u1, v0);
                builder->vertexUV(x0, y0, depth, u0, v0);

                builder->normal(-1.0F, 0.0F, 0.0F);
                builder->vertexUV(x0, y0, depth, u0, v0);
                builder->vertexUV(x0, y0, 0.0F, u0, v0);
                builder->vertexUV(x0, y1, 0.0F, u0, v1);
                builder->vertexUV(x0, y1, depth, u0, v1);

                builder->normal(1.0F, 0.0F, 0.0F);
                builder->vertexUV(x1, y1, depth, u1, v1);
                builder->vertexUV(x1, y1, 0.0F, u1, v1);
                builder->vertexUV(x1, y0, 0.0F, u1, v0);
                builder->vertexUV(x1, y0, depth, u1, v0);

                builder->normal(0.0F, 1.0F, 0.0F);
                builder->vertexUV(x1, y0, 0.0F, u1, v0);
                builder->vertexUV(x0, y0, 0.0F, u0, v0);
                builder->vertexUV(x0, y0, depth, u0, v0);
                builder->vertexUV(x1, y0, depth, u1, v0);

                builder->normal(0.0F, -1.0F, 0.0F);
                builder->vertexUV(x1, y1, depth, u1, v1);
                builder->vertexUV(x0, y1, depth, u0, v1);
                builder->vertexUV(x0, y1, 0.0F, u0, v1);
                builder->vertexUV(x1, y1, 0.0F, u1, v1);
            }
        }
        builder->end();
        GlStateManager::depthFunc(3);
        GlStateManager::endList(0);
    }
}

ItemRenderer::~ItemRenderer()
{
    delete mRandom;
}

ItemColors* ItemRenderer::getItemColors()
{
    return mItemColors;
}

void ItemRenderer::renderFoilLayer(Tesselator*)
{
    GlStateManager::depthFunc(2);
    GlStateManager::disableLighting();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(2, 1);
    Minecraft::GetInstance()->mTextures->bindTexture(&ENCHANT_GLINT_LOCATION);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    GlStateManager::matrixMode(2);
    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.125F, 0.125F, 0.125F);
    float offset = (float)(Minecraft::currentTimeMillis() % 3000LL);
    GlStateManager::translatef(offset / 3000.0F * 8.0F, 0.0F, 0.0F);
    GlStateManager::rotatef(-50.0F, 0.0F, 0.0F, 1.0F);
    GlStateManager::callList(sFoilList);
    GlStateManager::popMatrix();
    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.125F, 0.125F, 0.125F);
    offset = (float)(Minecraft::currentTimeMillis() % 4873LL);
    GlStateManager::translatef(-(offset / 4873.0F * 8.0F), 0.0F, 0.0F);
    GlStateManager::rotatef(10.0F, 0.0F, 0.0F, 1.0F);
    GlStateManager::callList(sFoilList);
    GlStateManager::matrixMode(2);
    GlStateManager::popMatrix();
    GlStateManager::matrixMode(0);
    GlStateManager::disableBlend();
    GlStateManager::enableLighting();
    GlStateManager::depthFunc(3);
}

void ItemRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float, float partialTicks)
{
    boost::shared_ptr<ItemEntity> itemEntity = boost::static_pointer_cast<ItemEntity>(entity);
    bindTexture(entity);
    mRandom->setSeed(187LL);
    not_null_ptr<ItemInstance> item = itemEntity->getItem();
    if (item->isEmpty())
        return;

    GlStateManager::pushMatrix();
    int count = 1;
    if (itemEntity->getItem()->getCount() > 1)
        count = 2;
    if (itemEntity->getItem()->getCount() > 5)
        count = 3;
    if (itemEntity->getItem()->getCount() > 20)
        count = 4;
    if (itemEntity->getItem()->getCount() > 40)
        count = 5;

    float bob = Mth::sin(((float)itemEntity->mAge + partialTicks) / 10.0F + itemEntity->mBobOffs) * 0.1F + 0.1F;
    GlStateManager::translatef((float)x, (float)y + bob + 0.25F, (float)z);
    GlStateManager::enableRescaleNormal();

    Block* block = Block::byItem(item->getItem());
    bool entityBlock = item->getItem() == Items::skull || item->getItem() == Items::banner || block == Blocks::conduit;

    if (entityBlock
        || (item->getItem()->getIconType() == 0 && block != NULL
            && BlockRenderer::canRender(block->defaultBlockState()->getRenderShape())))
    {
        GlStateManager::rotatef((((float)itemEntity->mAge + partialTicks) / 20.0F + itemEntity->mBobOffs) * 57.295776F,
                                0.0F, 1.0F, 0.0F);
        if (mItemFrameEnabled)
        {
            GlStateManager::scalef(1.25F, 1.25F, 1.25F);
            GlStateManager::translatef(0.0F, 0.05F, 0.0F);
            GlStateManager::rotatef(-90.0F, 0.0F, 1.0F, 0.0F);
        }

        float scale = 0.25F;
        if (!entityBlock)
        {
            unsigned int shape = block->defaultBlockState()->getRenderShape();
            if (shape != BLOCK_RENDER_MODEL
                && (shape <= BLOCK_RENDER_TORCH || shape == BLOCK_RENDER_LEVER || shape == BLOCK_RENDER_STEM
                    || shape == BLOCK_RENDER_DOUBLE_PLANT))
                scale = 0.5F;
        }
        GlStateManager::scalef(scale, scale, scale);

        BlockRenderDispatcher* dispatcher = Minecraft::GetInstance()->getBlockRenderer();
        for (int layer = 0; layer < count; ++layer)
        {
            GlStateManager::pushMatrix();
            if (layer > 0)
            {
                float xo = (mRandom->nextFloat() * 2.0F - 1.0F) * 0.2F;
                float yo = (mRandom->nextFloat() * 2.0F - 1.0F) * 0.2F;
                float zo = (mRandom->nextFloat() * 2.0F - 1.0F) * 0.2F;
                GlStateManager::translatef(xo / scale, yo / scale, zo / scale);
            }
            if (entityBlock)
            {
                EntityBlockRenderer::instance->renderByItem(item, 1.0F, true, true);
            }
            else
            {
                if (block->mSemiTransparent)
                {
                    GlStateManager::enableBlend();
                    GlStateManager::blendFunc(4, 5);
                }
                dispatcher->renderSingleBlock(block->getBlockState(item->getAuxValue()), 1.0F, 1.0F, true);
                if (block->mSemiTransparent)
                    GlStateManager::disableBlend();
            }
            GlStateManager::popMatrix();
        }
        GlStateManager::disableRescaleNormal();
        GlStateManager::popMatrix();
    }
    else if (item->getItem()->getIconType() == 1 && item->getItem()->hasMultipleSpriteLayers())
    {
        if (mItemFrameEnabled)
        {
            GlStateManager::scalef(0.5128205F, 0.5128205F, 0.5128205F);
            GlStateManager::translatef(0.0F, -0.05F, 0.0F);
            GlStateManager::disableLighting();
        }
        else
        {
            GlStateManager::scalef(0.5F, 0.5F, 0.5F);
        }
        bindTexture(&ITEM_LOCATION);

        for (int layer = 0; layer < 2; ++layer)
        {
            mRandom->setSeed(187LL);
            TextureAtlasSprite* sprite = item->getItem()->getLayerIcon(item->getAuxValue(), layer, item);
            if (mRenderWithColor)
            {
                int color = mItemColors->getColor(item, layer);
                float red = (float)((color >> 16) & 255) / 255.0F;
                float green = (float)((color >> 8) & 255) / 255.0F;
                float blue = (float)(color & 255) / 255.0F;
                GlStateManager::color4f(red, green, blue, 1.0F);
                renderItemBillboard(itemEntity, sprite, count, partialTicks, red, green, blue);
            }
            else
            {
                renderItemBillboard(itemEntity, sprite, count, partialTicks, 1.0F, 1.0F, 1.0F);
            }
        }
        GlStateManager::disableRescaleNormal();
        GlStateManager::popMatrix();
    }
    else
    {
        if (mItemFrameEnabled)
        {
            GlStateManager::scalef(0.5128205F, 0.5128205F, 0.5128205F);
            GlStateManager::translatef(0.0F, -0.05F, 0.0F);
            GlStateManager::disableLighting();
        }
        else
        {
            GlStateManager::scalef(0.5F, 0.5F, 0.5F);
        }

        if (Item::getId(item->getItem()) == ItemId::COMPASS)
            item->setAuxValue(255);
        if (Item::getId(item->getItem()) == ItemId::COMPASS)
            item->setAuxValue(0);

        TextureAtlasSprite* sprite = item->getItem()->getIcon(item);
        if (mRenderWithColor)
        {
            int color = mItemColors->getColor(item, 0);
            float red = (float)((color >> 16) & 255) / 255.0F;
            float green = (float)((color >> 8) & 255) / 255.0F;
            float blue = (float)(color & 255) / 255.0F;
            GlStateManager::color4f(red, green, blue, 1.0F);
            renderItemBillboard(itemEntity, sprite, count, partialTicks, red, green, blue);
        }
        else
        {
            renderItemBillboard(itemEntity, sprite, count, partialTicks, 1.0F, 1.0F, 1.0F);
        }
        GlStateManager::disableRescaleNormal();
        GlStateManager::popMatrix();
    }

    if (mItemFrameEnabled)
        GlStateManager::enableLighting();
}

const ResourceLocation* ItemRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<ItemEntity> itemEntity = boost::static_pointer_cast<ItemEntity>(entity);
    return getTextureLocation(itemEntity->getItem()->getItem()->getIconType());
}

const ResourceLocation* ItemRenderer::getTextureLocation(int iconType)
{
    if (iconType == 0)
        return &BLOCKS_LOCATION;
    return &ITEM_LOCATION;
}

void ItemRenderer::renderItemBillboard(boost::shared_ptr<ItemEntity>& entity, TextureAtlasSprite* sprite, int count,
                                       float partialTicks, float red, float green, float blue)
{
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    if (sprite == NULL)
    {
        not_null_ptr<ItemInstance> item = entity->getItem();
        sprite = mDispatcher->mTextures->getMissingIcon(item->getItem()->getIconType());
    }

    float u0 = (float)sprite->getU0(false);
    float u1 = (float)sprite->getU1(false);
    float v0 = (float)sprite->getV0(false);
    float v1 = (float)sprite->getV1(false);

    if (!mDispatcher->mOptions->mFancyGraphics)
    {
        for (int layer = 0; layer < count; ++layer)
        {
            GlStateManager::pushMatrix();
            if (layer > 0)
            {
                float xo = (mRandom->nextFloat() * 2.0F - 1.0F) * 0.3F;
                float yo = (mRandom->nextFloat() * 2.0F - 1.0F) * 0.3F;
                float zo = (mRandom->nextFloat() * 2.0F - 1.0F) * 0.3F;
                GlStateManager::translatef(xo, yo, zo);
            }
            if (!mItemFrameEnabled)
                GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
            GlStateManager::color4f(red, green, blue, 1.0F);
            builder->begin();
            builder->normal(0.0F, 1.0F, 0.0F);
            builder->vertexUV(-0.5F, -0.25F, 0.0F, u0, v1);
            builder->vertexUV(0.5F, -0.25F, 0.0F, u1, v1);
            builder->vertexUV(0.5F, 0.75F, 0.0F, u1, v0);
            builder->vertexUV(-0.5F, 0.75F, 0.0F, u0, v0);
            tesselator->end();
            GlStateManager::popMatrix();
        }
    }
    else
    {
        int width = sprite->getWidth();
        int lod = -1;
        if (width == 32)
            lod = 1;
        else if (width == 64)
            lod = 2;
        gRenderer->StateSetForceLOD(lod);
        GlStateManager::pushMatrix();
        if (mItemFrameEnabled)
        {
            GlStateManager::rotatef(180.0F, 0.0F, 1.0F, 0.0F);
        }
        else
        {
            GlStateManager::rotatef((((float)entity->mAge + partialTicks) / 20.0F + entity->mBobOffs) * 57.295776F,
                                    0.0F, 1.0F, 0.0F);
        }

        const float thickness = 0.0625F;
        const float spacing = 0.021875F;
        not_null_ptr<ItemInstance> item = entity->getItem();
        int itemCount = item->getCount();
        int layers;
        if (itemCount < 2)
            layers = 1;
        else if (itemCount < 16)
            layers = 2;
        else if (itemCount < 32)
            layers = 3;
        else
            layers = 4;
        GlStateManager::translatef(-0.5F, -0.25F, -(0.084375F * (float)layers * 0.5F));

        for (int layer = 0; layer < layers; ++layer)
        {
            GlStateManager::translatef(0.0F, 0.0F, thickness + spacing);
            if (item->getItem()->getIconType() == 0 && Block::byItem(item->getItem()) != NULL)
                bindTexture(&BLOCKS_LOCATION);
            else
                bindTexture(&ITEM_LOCATION);
            GlStateManager::color4f(red, green, blue, 1.0F);
            if (item->getItem()->isFoil(item))
                renderFoilLayer(tesselator);
        }
        GlStateManager::popMatrix();
        gRenderer->StateSetForceLOD(-1);
    }
}

void ItemRenderer::setupGuiItem(float x, float y, float scaleX, float scaleY, bool lighting)
{
    GlStateManager::translatef(x, y, mDepth + 100.0F);
    GlStateManager::scalef(scaleX, scaleY, 1.0F);
    GlStateManager::translatef(8.0F, 8.0F, 0.0F);
    GlStateManager::scalef(1.0F, -1.0F, 1.0F);
    GlStateManager::scalef(16.0F, 16.0F, 16.0F);
    if (lighting)
        GlStateManager::enableLighting();
    else
        GlStateManager::disableLighting();
}

bool ItemRenderer::isGui3d(not_null_ptr<ItemInstance>)
{
    return true;
}

void ItemRenderer::renderStatic(const not_null_ptr<ItemInstance>& item, boost::shared_ptr<LivingEntity> entity,
                                ItemTransforms::TransformType transformType, float alpha, bool leftHand,
                                TextureAtlasSprite* sprite)
{
    if (item->isEmpty())
        return;

    PIXBeginNamedEvent(0.0F, "renderStatic %i %i", Item::getId(item->getItem()), (int)transformType);
    Minecraft* minecraft = Minecraft::GetInstance();
    GlStateManager::pushMatrix();
    if (mRenderWithColor)
    {
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
        GlStateManager::enableAlphaTest();
        GlStateManager::alphaFunc(4, 0.01F);
    }
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::enableRescaleNormal();
    GlStateManager::pushMatrix();
    ItemTransforms* transforms = ItemTransforms::getTransforms(item.get(), entity.get());
    ItemTransforms::apply(transforms->getTransform(transformType), leftHand);
    GlStateManager::pushMatrix();
    GlStateManager::translatef(-0.5F, -0.5F, -0.5F);

    Block* block = Block::byItem(item->getItem());
    int blockId = Block::getId(block);
    if (item->getItem() == Items::trident && transformType != ItemTransforms::GUI
        && transformType != ItemTransforms::FIXED)
    {
        EntityRenderer* renderer = Minecraft::GetInstance()->getEntityRenderDispatcher()->getRenderer(
            EntityType::THROWN_TRIDENT);
        if (renderer != NULL)
        {
            GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
            Model* model = renderer->getModel();
            boost::shared_ptr<Entity> tridentEntity = boost::static_pointer_cast<Entity>(entity);
            ResourceLocation texture(TN_ITEM_TRIDENT);
            minecraft->mTextures->bindTexture(&texture);
            model->render(tridentEntity, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, true);
            if (item->hasFoil())
                static_cast<ThrownTridentRenderer*>(renderer)->renderFoil(tridentEntity);
        }
    }
    else if (item->getItem() != Items::skull && item->getItem() != Items::banner && blockId != BlockId::CONDUIT
             && !(blockId >= BlockId::WHITE_SHULKER_BOX && blockId <= BlockId::BLACK_SHULKER_BOX))
    {
        if (item->getItem()->getIconType() == 0 && block != NULL
            && BlockRenderer::canRender(block->defaultBlockState()->getRenderShape()))
        {
            GlStateManager::translatef(0.5F, 0.5F, 0.5F);
            int id = Block::getId(block);
            if (id == BlockId::CHEST || id == BlockId::ENDER_CHEST || id == BlockId::TRAPPED_CHEST)
                GlStateManager::rotatef(90.0F, 0.0F, 1.0F, 0.0F);
            MemSect(31);
            minecraft->mTextures->bindTexture(minecraft->mTextures->getTextureLocation(0));
            MemSect(0);
            if (block->hasInHandRenderOffset() && transformType != ItemTransforms::GUI)
            {
                GlStateManager::pushMatrix();
                GlStateManager::rotatef(90.0F, 0.0F, 1.0F, 0.0F);
                GlStateManager::translatef(0.0F, 0.5F, 0.0F);
                GlStateManager::rotatef(-90.0F, 0.0F, 1.0F, 0.0F);
            }
            minecraft->getBlockRenderer()->renderSingleBlock(block->getBlockState(item->getAuxValue()), 1.0F, alpha,
                                                             true);
            if (block->hasInHandRenderOffset() && transformType != ItemTransforms::GUI)
                GlStateManager::popMatrix();
            GlStateManager::disableBlend();
            GlStateManager::popMatrix();
            GlStateManager::popMatrix();
            GlStateManager::disableRescaleNormal();
            GlStateManager::disableBlend();
            GlStateManager::popMatrix();
            PIXEndNamedEvent();
            return;
        }

        if (transformType == ItemTransforms::FIXED)
        {
            GlStateManager::popMatrix();
            GlStateManager::popMatrix();
            GlStateManager::rotatef(180.0F, 0.0F, 1.0F, 0.0F);
            GlStateManager::pushMatrix();
            ItemTransforms::apply(transforms->getTransform(transformType), leftHand);
            GlStateManager::pushMatrix();
            GlStateManager::translatef(-0.5F, -0.5F, -0.5F);
        }
        GlStateManager::translatef(1.0F, 0.0F, 0.46875F);
        GlStateManager::rotatef(180.0F, 0.0F, 1.0F, 0.0F);

        for (int layer = 0; layer < 2; ++layer)
        {
            if (layer > 0 && !item->getItem()->hasMultipleSpriteLayers())
                break;

            if (mRenderWithColor && !item->isEmpty())
            {
                int color = getItemColors()->getColor(item, layer);
                GlStateManager::color4f((float)((color >> 16) & 255) / 255.0F, (float)((color >> 8) & 255) / 255.0F,
                                        (float)(color & 255) / 255.0F, alpha);
            }

            MemSect(31);
            TextureAtlasSprite* icon;
            if (entity == NULL)
                icon = item->getItem()->getLayerIcon(item->getAuxValue(), layer, item);
            else
                icon = entity->getItemInHandIcon(item, layer);
            if (sprite != NULL)
                icon = sprite;
            if (icon == NULL)
                break;

            int iconType = item->getItem()->getIconType();
            minecraft->mTextures->bindTexture(minecraft->mTextures->getTextureLocation(item->getItem()->getIconType()));
            bool pane = item->getItem() == Item::byBlock(Blocks::stained_glass_pane);
            MemSect(0);
            Tesselator* tesselator = Tesselator::getInstance();
            if (transformType != ItemTransforms::GUI)
            {
                int width = icon->getWidth();
                int lod = -1;
                if (width == 32)
                    lod = 1;
                else if (width == 64)
                    lod = 2;
                gRenderer->StateSetForceLOD(lod);
            }

            float u0 = (float)icon->getU0(false);
            float u1 = (float)icon->getU1(false);
            float v0 = (float)icon->getV0(false);
            float v1 = (float)icon->getV1(false);
            if (iconType == 0)
            {
                if (transformType == ItemTransforms::GUI || pane)
                    minecraft->getItemInHandRenderer()->renderItem3D(tesselator, u0, v0, u1, v1, icon->getSourceWidth(),
                                                                     icon->getSourceHeight(), 0.0625F,
                                                                     ItemInHandRenderer::ITEM_RENDER_MODE_GUI);
                else
                    minecraft->getItemInHandRenderer()->renderItem3D(tesselator, u0, v0, u1, v1, icon->getSourceWidth(),
                                                                     icon->getSourceHeight(), 0.0625F,
                                                                     ItemInHandRenderer::ITEM_RENDER_MODE_NONE);
            }
            else
            {
                minecraft->getItemInHandRenderer()->renderItem3D(
                    tesselator, u0, v0, u1, v1, icon->getSourceWidth(), icon->getSourceHeight(), 0.0625F,
                    transformType == ItemTransforms::GUI ? ItemInHandRenderer::ITEM_RENDER_MODE_FLAT
                                                         : ItemInHandRenderer::ITEM_RENDER_MODE_3D);
            }
            if (item->hasFoil())
                renderFoilLayer(tesselator);
        }
        gRenderer->StateSetForceLOD(-1);
    }
    else
    {
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
        GlStateManager::enableRescaleNormal();
        EntityBlockRenderer::instance->renderByItem(item, alpha, true, true);
    }

    GlStateManager::popMatrix();
    GlStateManager::popMatrix();
    GlStateManager::disableRescaleNormal();
    GlStateManager::disableBlend();
    GlStateManager::popMatrix();
    PIXEndNamedEvent();
}

void ItemRenderer::renderGuiItem(not_null_ptr<ItemInstance> item, float x, float y, float scaleX, float scaleY,
                                 float alpha, bool, TextureAtlasSprite* sprite)
{
    GlStateManager::pushMatrix();
    GlStateManager::enableRescaleNormal();
    GlStateManager::enableAlphaTest();
    GlStateManager::alphaFunc(4, 0.01F);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
    setupGuiItem(x, y, scaleX, scaleY, isGui3d(item));
    renderStatic(item, boost::shared_ptr<LivingEntity>(), ItemTransforms::GUI, alpha, false, sprite);
    GlStateManager::disableAlphaTest();
    GlStateManager::disableRescaleNormal();
    GlStateManager::disableLighting();
    GlStateManager::popMatrix();
}

void ItemRenderer::renderGuiItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float alpha)
{
    renderGuiItem(item, x, y, scale, scale, alpha, true, NULL);
}

void ItemRenderer::renderGuiItem(not_null_ptr<ItemInstance> item, int x, int y)
{
    renderGuiItem(item, (float)x, (float)y, 1.0F, 1.0F);
}

void ItemRenderer::renderAndDecorateItem(not_null_ptr<ItemInstance> item, float x, float y, float scaleX, float scaleY,
                                         float alpha, bool, bool, bool lighting, TextureAtlasSprite* sprite)
{
    if (item->isEmpty())
        return;
    renderGuiItem(item, x, y, scaleX, scaleY, alpha, lighting, sprite);
}

void ItemRenderer::renderAndDecorateItem(not_null_ptr<ItemInstance> item, float x, float y, float scale, float alpha,
                                         bool foil)
{
    if (item->isEmpty())
        return;
    renderAndDecorateItem(item, x, y, scale, scale, alpha, foil, false, true, NULL);
}

void ItemRenderer::renderAndDecorateItem(not_null_ptr<ItemInstance> item, int x, int y)
{
    renderAndDecorateItem(item, (float)x, (float)y, 1.0F, 1.0F, item->getItem()->isFoil(item));
}

void ItemRenderer::fillRect(Tesselator* tesselator, int x, int y, int width, int height, int rgb, int alpha)
{
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    builder->color(rgb, alpha);
    builder->vertex((float)x, (float)y, 0.0F);
    builder->vertex((float)x, (float)(y + height), 0.0F);
    builder->vertex((float)(x + width), (float)(y + height), 0.0F);
    builder->vertex((float)(x + width), (float)y, 0.0F);
    tesselator->end();
}

void ItemRenderer::renderGuiItemDecorations(Font* font, not_null_ptr<ItemInstance> item, int x, int y,
                                            const std::wstring& countText, int colour, float alpha)
{
    if (item->isEmpty())
        return;

    if (item->isDamaged())
    {
        GlStateManager::disableLighting();
        GlStateManager::disableDepthTest();
        GlStateManager::disableTexture();
        Tesselator* tesselator = Tesselator::getInstance();
        float damage = (float)item->getDamageValue();
        float maxDamage = (float)item->getMaxDamage();
        float health = (maxDamage - damage) / maxDamage;
        if (health < 0.0F)
            health = 0.0F;
        int barWidth = Math::round(13.0F - damage * 13.0F / maxDamage);
        int barColour = Mth::hsvToRgb(health / 3.0F, 1.0F, 1.0F);
        int barAlpha = (int)ceilf(alpha * 255.0F);
        fillRect(tesselator, x + 2, y + 13, 13, 2, 0, barAlpha);
        fillRect(tesselator, x + 2, y + 13, barWidth, 1, barColour, barAlpha);
        alpha = ceilf(ceilf(alpha * 255.0F) / 255.0F);
        GlStateManager::enableTexture();
        GlStateManager::enableLighting();
        GlStateManager::enableDepthTest();
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    }
    else if (item->hasPotionStrengthBar())
    {
        GlStateManager::disableLighting();
        GlStateManager::disableDepthTest();
        GlStateManager::disableTexture();
        Tesselator* tesselator = Tesselator::getInstance();
        int barAlpha = (int)ceilf(alpha * 255.0F);
        fillRect(tesselator, x + 3, y + 13, 11, 2, 0, barAlpha);
        fillRect(tesselator, x + 3, y + 13, sPotionStrengthBarWidths[item->GetPotionStrength()], 2, 0xE1EB, barAlpha);
        fillRect(tesselator, x + 5, y + 13, 1, 2, 0, barAlpha);
        fillRect(tesselator, x + 8, y + 13, 1, 2, 0, barAlpha);
        fillRect(tesselator, x + 11, y + 13, 1, 2, 0, barAlpha);
        alpha = ceilf(ceilf(alpha * 255.0F) / 255.0F);
        GlStateManager::enableTexture();
        GlStateManager::enableLighting();
        GlStateManager::enableDepthTest();
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    }

    if (item->getCount() > 1 || !countText.empty() || item->mUnknown)
    {
        if (colour == -1)
            colour = gApp.GetHTMLColour(eMinecraftColour_Color_ItemCount);
        MemSect(31);
        std::wstring text = countText;
        if (text.empty())
        {
            if (item->getCount() <= 64)
                text = _toString<int>(item->getCount());
            else
                text = _toString<int>(64) + L"+";
        }
        MemSect(0);
        GlStateManager::enableAlphaTest();
        GlStateManager::disableLighting();
        GlStateManager::disableDepthTest();
        unsigned int textAlpha = (unsigned int)(alpha * 255.0F);
        font->drawShadow(text, x + 17 - font->width(text), y + 9, colour | (int)(textAlpha << 24));
        GlStateManager::enableLighting();
        GlStateManager::enableDepthTest();
        GlStateManager::disableAlphaTest();
    }

    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mPlayer;
    if (player != NULL)
    {
        Item* rawItem = item->getItem();
        float cooldown = player->getCooldowns()->getCooldownPercent(rawItem, Minecraft::GetInstance()->getFrameTime());
        if (cooldown > 0.0F)
        {
            GlStateManager::disableLighting();
            GlStateManager::disableDepthTest();
            GlStateManager::disableTexture();
            Tesselator* tesselator = Tesselator::getInstance();
            float top = 16.0F * (1.0F - cooldown);
            int topPixel = (int)top;
            if (top < (float)topPixel)
                --topPixel;
            float height = 16.0F * cooldown;
            int heightPixels = (int)height;
            if ((float)heightPixels < height)
                ++heightPixels;
            fillRect(tesselator, x, y + topPixel, 16, heightPixels, 0xFFFFFF, 0x7F);
            GlStateManager::enableTexture();
            GlStateManager::enableLighting();
            GlStateManager::enableDepthTest();
        }
    }
    GlStateManager::disableBlend();
}

void ItemRenderer::renderGuiItemDecorations(Font* font, not_null_ptr<ItemInstance> item, int x, int y, float alpha)
{
    renderGuiItemDecorations(font, item, x, y, L"", -1, alpha);
}

void ItemRenderer::blit(float x, float y, TextureAtlasSprite* sprite, float width, float height)
{
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    Minecraft* minecraft = Minecraft::GetInstance();
    float xScale = (float)minecraft->mWidth / (float)minecraft->mFullWidth;
    float yScale = (float)minecraft->mHeight / (float)minecraft->mFullHeight;
    float x0 = x * xScale;
    float x1 = (x + width) * xScale;
    float y1 = (y + height) * yScale;
    float y0 = y * yScale;
    x0 = ceilf(x0);
    x1 = floorf(x1);
    y0 = ceilf(y0);
    y1 = floorf(y1);
    x0 = (x0 + 0.5F) / xScale;
    x1 = (x1 - 0.5F) / xScale;
    y0 = (y0 + 0.5F) / yScale;
    y1 = (y1 - 0.5F) / yScale;
    builder->vertexUV(x0, y1, mDepth, (float)sprite->getU0(true), (float)sprite->getV1(true));
    builder->vertexUV(x1, y1, mDepth, (float)sprite->getU1(true), (float)sprite->getV1(true));
    builder->vertexUV(x1, y0, mDepth, (float)sprite->getU1(true), (float)sprite->getV0(true));
    builder->vertexUV(x0, y0, mDepth, (float)sprite->getU0(true), (float)sprite->getV0(true));
    tesselator->end();
}

void ItemRenderer::SetItemFrame(bool itemFrame)
{
    mItemFrameEnabled = itemFrame;
}
