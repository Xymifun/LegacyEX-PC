#include "game/client/render/ItemFrameRenderer.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/HangingEntity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/core/PIXNamedEvents.hpp"

ResourceLocation ItemFrameRenderer::MAP_BACKGROUND_LOCATION(TN_MISC_MAPBG);

ItemFrameRenderer::ItemFrameRenderer(EntityRenderDispatcher* dispatcher, ItemRenderer* itemRenderer)
    : EntityRenderer(dispatcher),
      mItemRenderer(itemRenderer)
{
    mMinecraft = Minecraft::GetInstance();
    mBlockFrameDisplayList = -1;
    mFrameDisplayList = -1;
}

ItemFrameRenderer::~ItemFrameRenderer()
{
}

void ItemFrameRenderer::registerTerrainTextures(IconRegister* iconRegister)
{
    mFrameIcon = iconRegister->registerIcon(L"itemframe_back");
}

void ItemFrameRenderer::drawBlockFrame(const boost::shared_ptr<ItemFrame>& itemFrame)
{
    mDispatcher->mTextures->bindTexture(&BLOCKS_LOCATION);
    GlStateManager::pushMatrix();
    if (mBlockFrameDisplayList == -1)
    {
        mBlockFrameDisplayList = MemoryTracker::genLists(1);
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        builder->begin();
        GlStateManager::newList(mBlockFrameDisplayList, 0);
        const BlockState* state = Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT,
                                                                                PlanksBlock::Variant::BIRCH);
        BlockRenderer* renderer = Minecraft::GetInstance()->mLevelRenderer->mBlockRenderers[0];
        renderer->setFixedShape(0.0625f, 0.0625f, 0.9375625f, 0.9375f, 0.9375f, 1.0f);
        renderer->setFixedTexture(mFrameIcon);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->clearFixedTexture();
        renderer->clearFixedShape();
        renderer->setFixedShape(0.0f, 0.0f, 0.9375625f, 1.0f, 0.0625f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.0f, 0.9375f, 0.9375625f, 1.0f, 1.0f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.0f, 0.0625f, 0.9375625f, 0.0625f, 0.9375f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.9375f, 0.0625f, 0.9375625f, 1.0f, 0.9375f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->clearFixedShape();
        renderer->clearFixedTexture();
        builder->end();
        GlStateManager::endList(0);
    }
    GlStateManager::callList(mBlockFrameDisplayList);
    GlStateManager::popMatrix();
}

void ItemFrameRenderer::drawFrame(const boost::shared_ptr<ItemFrame>& itemFrame)
{
    GlStateManager::pushMatrix();
    mDispatcher->mTextures->bindTexture(&BLOCKS_LOCATION);
    if (mFrameDisplayList == -1)
    {
        LevelRenderer* levelRenderer = Minecraft::GetInstance()->mLevelRenderer;
        BlockRenderer* renderer = levelRenderer->mBlockRenderers[0];
        if (renderer == NULL && (renderer = levelRenderer->mBlockRenderers[1]) == NULL
            && (renderer = levelRenderer->mBlockRenderers[2]) == NULL)
            renderer = levelRenderer->mBlockRenderers[3];
        mFrameDisplayList = MemoryTracker::genLists(1);
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        builder->begin();
        GlStateManager::newList(mFrameDisplayList, 0);
        const BlockState* state = Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT,
                                                                                PlanksBlock::Variant::BIRCH);
        renderer->setFixedShape(0.1875f, 0.1875f, 0.96875f, 0.8125f, 0.8125f, 1.0f);
        renderer->setFixedTexture(mFrameIcon);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->clearFixedTexture();
        renderer->clearFixedShape();
        renderer->setFixedShape(0.125f, 0.125f, 0.9375f, 0.875f, 0.1875f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.125f, 0.8125f, 0.9375f, 0.875f, 0.875f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.125f, 0.1875f, 0.9375f, 0.1875f, 0.8125f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->setFixedShape(0.8125f, 0.1875f, 0.9375f, 0.875f, 0.8125f, 1.0f);
        renderer->renderBlock(state, 1.0f, 1.0f, false, false);
        renderer->clearFixedShape();
        renderer->clearFixedTexture();
        builder->end();
        GlStateManager::endList(0);
    }
    GlStateManager::callList(mFrameDisplayList);
    GlStateManager::popMatrix();
}

void ItemFrameRenderer::drawItem(const boost::shared_ptr<ItemFrame>& itemFrame)
{
    not_null_ptr<ItemInstance> item = itemFrame->getItem();
    if (item->isEmpty())
        return;
    GlStateManager::pushMatrix();
    GlStateManager::disableLighting();
    const bool isMap = item->getItem() == Items::filledMap;
    int rotation;
    if (isMap)
        rotation = itemFrame->getRotation() % 4 * 2;
    else
        rotation = itemFrame->getRotation();
    GlStateManager::rotatef(static_cast<float>(rotation) * 360.0f / 8.0f, 0.0f, 0.0f, 1.0f);
    if (isMap)
    {
        mDispatcher->mTextures->bindTexture(&MAP_BACKGROUND_LOCATION);
        GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::scalef(0.0078125f, 0.0078125f, 0.0078125f);
        GlStateManager::translatef(-64.0f, -64.0f, 0.0f);
        boost::shared_ptr<MapItemSavedData> data =
            static_cast<MapItem*>(Items::filledMap)->getSavedData(item, itemFrame->mLevel);
        GlStateManager::translatef(0.0f, 0.0f, -1.0f);
        if (data)
            mMinecraft->mGameRenderer->getMapRenderer()->render(data, boost::shared_ptr<Player>());
    }
    else
    {
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        mItemRenderer->renderStatic(item, boost::shared_ptr<LivingEntity>(), ItemTransforms::FIXED, 1.0f, false, NULL);
    }
    GlStateManager::enableLighting();
    GlStateManager::popMatrix();
}

void ItemFrameRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                               float partialTick)
{
    ++gHangingEntityRenderCount;
    PIXBeginNamedEvent(0.0f, "ItemFrameRenderer");
    boost::shared_ptr<ItemFrame> itemFrame = boost::static_pointer_cast<ItemFrame>(entity);
    GlStateManager::pushMatrix();
    BlockPos pos = itemFrame->getPos();
    double dx = (double)pos.x - itemFrame->mX + x + 0.5;
    double dy = (double)pos.y - itemFrame->mY + y + 0.5;
    double dz = (double)pos.z - itemFrame->mZ + z + 0.5;
    GlStateManager::translated(dx, dy, dz);
    GlStateManager::rotatef(180.0f - itemFrame->mYRot, 0.0f, 1.0f, 0.0f);
    GlStateManager::pushMatrix();
    GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }
    if (itemFrame->getItem()->getItem() == Items::filledMap)
        drawBlockFrame(itemFrame);
    else
        drawFrame(itemFrame);
    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }
    GlStateManager::popMatrix();
    GlStateManager::translatef(0.0f, 0.0f, 0.4375f);
    drawItem(itemFrame);
    GlStateManager::popMatrix();
    renderName(entity, x + static_cast<float>(itemFrame->mDirection->getStepX()) * 0.3f, y - 0.25,
               z + static_cast<float>(itemFrame->mDirection->getStepZ()) * 0.3f);
    PIXEndNamedEvent();
}

void ItemFrameRenderer::renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z)
{
    boost::shared_ptr<ItemFrame> itemFrame = boost::static_pointer_cast<ItemFrame>(entity);
    if (!(Minecraft::renderNames() && !itemFrame->getItem()->isEmpty() && itemFrame->getItem()->hasCustomHoverName()
          && mDispatcher->mPlayer.get() == itemFrame.get()))
        return;
    double distance = itemFrame->distanceToSqr(mDispatcher->mCamera);
    float maxDistance = itemFrame->isSneaking() ? 32.0f : 64.0f;
    if (distance < (double)(maxDistance * maxDistance))
    {
        std::wstring name = itemFrame->getItem()->getHoverName();
        renderNameTag(entity, name, x, y, z, 64, 0xff000000);
    }
}
