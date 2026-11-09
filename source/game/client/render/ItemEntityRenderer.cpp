#include "game/client/render/ItemEntityRenderer.hpp"

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/EntityBlockRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Options.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemColors.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemTransform.hpp"
#include "game/item/ItemTransforms.hpp"
#include "game/item/Items.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/StainedGlassPaneBlock.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#endif

ItemEntityRenderer::ItemEntityRenderer(EntityRenderDispatcher* dispatcher, ItemRenderer* itemRenderer)
    : EntityRenderer(dispatcher),
      mRandom()
{
    mItemRenderer = itemRenderer;
    mShadowRadius = 0.15F;
    mShadowStrength = 0.75F;
    mItemFrame = false;
    mIsGui3d = true;
}

ItemEntityRenderer::~ItemEntityRenderer()
{
}

void ItemEntityRenderer::renderItemBillboard(boost::shared_ptr<ItemEntity> entity, TextureAtlasSprite* sprite,
                                             int count, float partialTick, float red, float green, float blue)
{
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();

    if (sprite == NULL)
    {
        not_null_ptr<ItemInstance> missing = entity->getItem();
        sprite = mDispatcher->mTextures->getMissingIcon(missing->getItem()->getIconType());
    }

    float u0 = sprite->getU0(false);
    float u1 = sprite->getU1(false);
    float v0 = sprite->getV0(false);
    float v1 = sprite->getV1(false);
    float size = 1.0F;
    float xOffset = 0.5F;
    float yOffset = 0.25F;

    bool flat = false;
#ifdef MINECRAFT_PC
    flat = !mItemFrame && PcGoldenAgeOn(kPcGoldenAgeItems);
#endif
    if (mDispatcher->mOptions->mFancyGraphics || flat)
    {
        int width = (int)sprite->getWidth();
        int lod = -1;
        if (width == 32)
            lod = 1;
        else if (width == 64)
            lod = 2;
        gRenderer->StateSetForceLOD(lod);

        GlStateManager::pushMatrix();
        if (mItemFrame)
        {
            GlStateManager::rotatef(180.0F, 0.0F, 1.0F, 0.0F);
        }
#ifdef MINECRAFT_PC
        else if (flat)
        {
            GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
        }
#endif
        else
        {
            GlStateManager::rotatef((((float)entity->getAge() + partialTick) / 20.0F + entity->mBobOffs)
                                        * Mth::RAD_TO_DEG,
                                    0.0F, 1.0F, 0.0F);
        }

        float thickness = 0.0625F;
        float gap = 0.021875F;
        not_null_ptr<ItemInstance> item = entity->getItem();
        int stackCount = item->getCount();
        int layers;
        if (stackCount < 2)
            layers = 1;
        else if (stackCount < 16)
            layers = 2;
        else if (stackCount < 32)
            layers = 3;
        else
            layers = 4;
#ifdef MINECRAFT_PC
        if (flat)
        {
            thickness = 0.0F;
            gap = 0.0F;
            layers = 1;
            GlStateManager::disableLighting();
        }
#endif
        GlStateManager::translatef(-xOffset, -yOffset, -((thickness + gap) * (float)layers / 2.0F));

        for (int layer = 0; layer < layers; ++layer)
        {
            GlStateManager::translatef(0.0F, 0.0F, thickness + gap);
            Block* block = Block::byItem(item->getItem());
            if (dynamic_cast<StainedGlassPaneBlock*>(block) != NULL)
            {
                bindTexture(&BLOCKS_LOCATION);
                GlStateManager::color4f(red, green, blue, 1.0F);
                Minecraft::GetInstance()->getItemInHandRenderer()->renderItem3D(
                    tesselator, u0, v0, u1, v1, sprite->getSourceWidth(), sprite->getSourceHeight(), thickness,
                    ItemInHandRenderer::ITEM_RENDER_MODE_GUI);
            }
            else if (item->getItem()->getIconType() == 0 && block != NULL)
            {
                bindTexture(&BLOCKS_LOCATION);
                GlStateManager::color4f(red, green, blue, 1.0F);
                Minecraft::GetInstance()->getItemInHandRenderer()->renderItem3D(
                    tesselator, u0, v0, u1, v1, sprite->getSourceWidth(), sprite->getSourceHeight(), thickness,
                    ItemInHandRenderer::ITEM_RENDER_MODE_NONE);
            }
            else
            {
                bindTexture(&ITEM_LOCATION);
                GlStateManager::color4f(red, green, blue, 1.0F);
                Minecraft::GetInstance()->getItemInHandRenderer()->renderItem3D(
                    tesselator, u0, v0, u1, v1, sprite->getSourceWidth(), sprite->getSourceHeight(), thickness,
                    ItemInHandRenderer::ITEM_RENDER_MODE_3D);
            }
            if (item->getItem()->isFoil(item))
                mItemRenderer->renderFoilLayer(tesselator);
        }

        GlStateManager::popMatrix();
        gRenderer->StateSetForceLOD(-1);
#ifdef MINECRAFT_PC
        if (flat)
            GlStateManager::enableLighting();
#endif
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            GlStateManager::pushMatrix();
            if (i > 0)
            {
                float offsetX = (mRandom.nextFloat() * 2.0F - 1.0F) * 0.3F;
                float offsetY = (mRandom.nextFloat() * 2.0F - 1.0F) * 0.3F;
                float offsetZ = (mRandom.nextFloat() * 2.0F - 1.0F) * 0.3F;
                GlStateManager::translatef(offsetX, offsetY, offsetZ);
            }
            if (!mItemFrame)
                GlStateManager::rotatef(180.0F - mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
            GlStateManager::color4f(red, green, blue, 1.0F);
            builder->begin();
            builder->normal(0.0F, 1.0F, 0.0F);
            builder->vertexUV(0.0F - xOffset, 0.0F - yOffset, 0.0F, u0, v1);
            builder->vertexUV(size - xOffset, 0.0F - yOffset, 0.0F, u1, v1);
            builder->vertexUV(size - xOffset, 1.0F - yOffset, 0.0F, u1, v0);
            builder->vertexUV(0.0F - xOffset, 1.0F - yOffset, 0.0F, u0, v0);
            builder->end();
            GlStateManager::popMatrix();
        }
    }
}

void ItemEntityRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float,
                                float partialTick)
{
    boost::shared_ptr<ItemEntity> itemEntity = boost::static_pointer_cast<ItemEntity>(entity);
    bindTexture(entity);
    mRandom.setSeed(187LL);

    not_null_ptr<ItemInstance> item = itemEntity->getItem();
    if (item == ItemInstance::EMPTY)
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

    float bob = 0.0F;
    if (!itemEntity->isInWater())
    {
        bob = Mth::sin(((float)itemEntity->getAge() + partialTick) / 10.0F + itemEntity->mBobOffs) * 0.1F + 0.1F;
    }

    ItemTransform* groundTransform =
        ItemTransforms::getTransforms(item.get(), NULL)->getTransform(ItemTransforms::GROUND);
    GlStateManager::translatef((float)x, 0.25F * groundTransform->scale.y + ((float)y + bob), (float)z);

    Block* block = Block::byItem(item->getItem());
    if (block != NULL && block->mSemiTransparent)
    {
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
    }

    bool special = item->getItem() == Items::skull || item->getItem() == Items::banner
                   || item->getItem() == Item::byBlock(Blocks::conduit);

    if (special
        || (item->getItem()->getIconType() == 0 && block != NULL
            && BlockRenderer::canRender(block->defaultBlockState()->getRenderShape())))
    {
        GlStateManager::rotatef((((float)itemEntity->getAge() + partialTick) / 20.0F + itemEntity->mBobOffs)
                                    * 57.295776F,
                                0.0F, 1.0F, 0.0F);

        if (mItemFrame)
        {
            GlStateManager::scalef(1.25F, 1.25F, 1.25F);
            GlStateManager::translatef(0.0F, 0.05F, 0.0F);
            GlStateManager::rotatef(-90.0F, 0.0F, 1.0F, 0.0F);
            if (special)
                GlStateManager::rotatef(-90.0F, 0.0F, 1.0F, 0.0F);
        }

        float scale = 1.0F;
        if (!special)
        {
            scale = 0.25F;
            const unsigned int renderShape = block->defaultBlockState()->getRenderShape();
            if (renderShape != BLOCK_RENDER_MODEL
                && (renderShape <= BLOCK_RENDER_TORCH || renderShape == BLOCK_RENDER_LEVER
                    || renderShape == BLOCK_RENDER_STEM || renderShape == BLOCK_RENDER_DOUBLE_PLANT))
            {
                scale = 0.5F;
            }
        }
        GlStateManager::scalef(scale, scale, scale);

        BlockRenderDispatcher* blockRenderer = Minecraft::GetInstance()->getBlockRenderer();
        if (special)
        {
            for (int i = 0; i < count; ++i)
            {
                GlStateManager::pushMatrix();
                if (i > 0)
                {
                    float offsetX = (mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F;
                    float offsetY = (mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F;
                    float offsetZ = (mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F;
                    GlStateManager::translatef(offsetX / scale, offsetY / scale, offsetZ / scale);
                }
                ItemTransforms::apply(
                    ItemTransforms::getTransforms(item.get(), NULL)->getTransform(ItemTransforms::GROUND), false);
                GlStateManager::pushMatrix();
                GlStateManager::translatef(-0.5F, -0.5F, -0.5F);
                GlStateManager::enableRescaleNormal();
                EntityBlockRenderer::instance->renderByItem(item, 1.0F, true, true);
                GlStateManager::popMatrix();
                GlStateManager::popMatrix();
            }
        }
        else
        {
            for (int i = 0; i < count; ++i)
            {
                GlStateManager::pushMatrix();
                if (i > 0)
                {
                    float offsetX = (mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F;
                    float offsetY = (mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F;
                    float offsetZ = (mRandom.nextFloat() * 2.0F - 1.0F) * 0.2F;
                    GlStateManager::translatef(offsetX / scale, offsetY / scale, offsetZ / scale);
                }
                blockRenderer->renderSingleBlock(block->getBlockState(item->getAuxValue()), 1.0F, 1.0F, true);
                GlStateManager::popMatrix();
            }
        }
    }
    else if (item->getItem()->getIconType() == 1 && item->getItem()->hasMultipleSpriteLayers())
    {
        if (mItemFrame)
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
            mRandom.setSeed(187LL);
            TextureAtlasSprite* sprite = item->getItem()->getLayerIcon(item->getAuxValue(), layer, item);
            if (!mIsGui3d)
            {
                renderItemBillboard(itemEntity, sprite, count, partialTick, 1.0F, 1.0F, 1.0F);
            }
            else
            {
                int colour = mItemRenderer->getItemColors()->getColor(item, layer);
                float red = (float)((colour >> 16) & 0xFF) / 255.0F;
                float green = (float)((colour >> 8) & 0xFF) / 255.0F;
                float blue = (float)(colour & 0xFF) / 255.0F;
                GlStateManager::color4f(red * 1.0, green * 1.0, blue * 1.0, 1.0F);
                renderItemBillboard(itemEntity, sprite, count, partialTick, red * 1.0, green * 1.0, blue * 1.0);
            }
        }
    }
    else
    {
        if (mItemFrame)
        {
            GlStateManager::scalef(0.5128205F, 0.5128205F, 0.5128205F);
            GlStateManager::translatef(0.0F, -0.05F, 0.0F);
            GlStateManager::disableLighting();
        }
        else
        {
            GlStateManager::scalef(0.5F, 0.5F, 0.5F);
        }

        if (item->getItem() == Items::compass)
            item->setAuxValue(255);

        TextureAtlasSprite* sprite = item->getItem()->getIcon(item);
        if (!mIsGui3d)
        {
            renderItemBillboard(itemEntity, sprite, count, partialTick, 1.0F, 1.0F, 1.0F);
        }
        else
        {
            int colour = mItemRenderer->getItemColors()->getColor(item, 0);
            float red = (float)((colour >> 16) & 0xFF) / 255.0F;
            float green = (float)((colour >> 8) & 0xFF) / 255.0F;
            float blue = (float)(colour & 0xFF) / 255.0F;
            GlStateManager::color4f(red, green, blue, 1.0F);
            renderItemBillboard(itemEntity, sprite, count, partialTick, red * 1.0, green * 1.0, blue * 1.0);
        }
    }

    if (block != NULL && block->mSemiTransparent)
        GlStateManager::disableBlend();

    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    if (mItemFrame)
        GlStateManager::enableLighting();
}

const ResourceLocation* ItemEntityRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &BLOCKS_LOCATION;
}

void ItemEntityRenderer::SetItemFrame(bool itemFrame)
{
    mItemFrame = itemFrame;
}
