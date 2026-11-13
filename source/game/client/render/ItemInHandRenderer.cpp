#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/world/block/BlockId.hpp"
#include <cmath>
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Direction.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Enchantments.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/item/BowItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/MapItemSavedData.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FireBlock.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/ResourceLocation.hpp"

namespace
{
int sItemRenderDisplayLists[5] = {-1, -1, -1, -1, -1};
static ResourceLocation WATER_OVERLAY_LOCATION(TN_MISC_WATER);
static ResourceLocation MAP_BACKGROUND_LOCATION(TN_MISC_MAPBG);

}

ItemInHandRenderer::ItemInHandRenderer(Minecraft* minecraft)
{
    mMainHandItem = ItemInstance::EMPTY;
    mOffHandItem = ItemInstance::EMPTY;
    mMainHandHeight = 0.0f;
    mOffHandHeight = 0.0f;
    mMainHandHeightOld = 0.0f;
    mOffHandHeightOld = 0.0f;
    mListId = -1;
    mMinecraft = minecraft;
    mEntityRenderDispatcher = minecraft->getEntityRenderDispatcher();
    mItemRenderer = minecraft->getItemRenderer();

    if (sItemRenderDisplayLists[0] == -1)
    {
        sItemRenderDisplayLists[0] = MemoryTracker::genLists(1);
        GlStateManager::newList(sItemRenderDisplayLists[0], 0);
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->begin();
        for (int row = 0; row < 16; ++row)
        {
            float y0 = static_cast<float>(row) * 0.0625f;
            for (int column = 0; column < 16; ++column)
            {
                float y1 = 0.0625f + y0;
                float v = 0.0018382353f + static_cast<float>(15 - row) / 272.0f;
                float x0 = static_cast<float>(column) * 0.0625f;
                float u = static_cast<float>(15 - column) * 0.00390625f + 0.001953125f;
                float x1 = x0 + 0.0625f;
                builder->normal(0.0f, 0.0f, 1.0f);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->normal(0.0f, 0.0f, -1.0f);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->normal(-1.0f, 0.0f, 0.0f);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->normal(1.0f, 0.0f, 0.0f);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->normal(0.0f, 1.0f, 0.0f);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->normal(0.0f, -1.0f, 0.0f);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
            }
        }
        builder->end();
        GlStateManager::endList(0);
    }

    if (sItemRenderDisplayLists[1] == -1)
    {
        int list = MemoryTracker::genLists(2);
        sItemRenderDisplayLists[1] = list;
        float pixels = 16.0f;
        float textureSize = 256.0f;
        float depth = 0.0625f;
        for (int size = 0; size < 2; ++size, ++list)
        {
            GlStateManager::newList(list, 0);
            BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
            builder->begin();
            float uvSize = pixels / textureSize;
            float uvEdge = uvSize + -6.1035156e-05f;
            builder->normal(0.0f, 0.0f, 1.0f);
            builder->vertexUV(0.0f, 0.0f, 0.0f, uvEdge, uvEdge);
            builder->vertexUV(1.0f, 0.0f, 0.0f, -6.1035156e-05f, uvEdge);
            builder->vertexUV(1.0f, 1.0f, 0.0f, -6.1035156e-05f, -6.1035156e-05f);
            builder->vertexUV(0.0f, 1.0f, 0.0f, uvEdge, -6.1035156e-05f);
            builder->normal(0.0f, 0.0f, -1.0f);
            float back = -depth;
            builder->vertexUV(0.0f, 1.0f, back, uvEdge, -6.1035156e-05f);
            builder->vertexUV(1.0f, 1.0f, back, -6.1035156e-05f, -6.1035156e-05f);
            builder->vertexUV(1.0f, 0.0f, back, -6.1035156e-05f, uvEdge);
            builder->vertexUV(0.0f, 0.0f, back, uvEdge, uvEdge);
            builder->normal(-1.0f, 0.0f, 0.0f);
            int layers = static_cast<int>(pixels);
            float halfTexel = 0.5f / textureSize;
            for (int layer = 0; layer < layers; ++layer)
            {
                float x = static_cast<float>(layer) / pixels;
                float u = -(uvSize * x - uvSize) - halfTexel;
                builder->vertexUV(x, 0.0f, back, u, uvSize);
                builder->vertexUV(x, 0.0f, 0.0f, u, uvSize);
                builder->vertexUV(x, 1.0f, 0.0f, u, 0.0f);
                builder->vertexUV(x, 1.0f, back, u, 0.0f);
            }
            builder->normal(1.0f, 0.0f, 0.0f);
            for (int layer = 0; layer < layers; ++layer)
            {
                float x = static_cast<float>(layer) / pixels;
                float x1 = x + 1.0f / pixels;
                float u = -(uvSize * x - uvSize) - halfTexel;
                builder->vertexUV(x1, 1.0f, back, u, 0.0f);
                builder->vertexUV(x1, 1.0f, 0.0f, u, 0.0f);
                builder->vertexUV(x1, 0.0f, 0.0f, u, uvSize);
                builder->vertexUV(x1, 0.0f, back, u, uvSize);
            }
            builder->normal(0.0f, 1.0f, 0.0f);
            for (int layer = 0; layer < 16; ++layer)
            {
                float y = static_cast<float>(layer) / pixels;
                float y1 = y + 1.0f / pixels;
                float v = -(uvSize * y - uvSize) - halfTexel;
                builder->vertexUV(0.0f, y1, 0.0f, uvSize, v);
                builder->vertexUV(1.0f, y1, 0.0f, 0.0f, v);
                builder->vertexUV(1.0f, y1, back, 0.0f, v);
                builder->vertexUV(0.0f, y1, back, uvSize, v);
            }
            builder->normal(0.0f, -1.0f, 0.0f);
            for (int layer = 0; layer < 16; ++layer)
            {
                float y = static_cast<float>(layer) / pixels;
                float v = -(uvSize * y - uvSize) - halfTexel;
                builder->vertexUV(1.0f, y, 0.0f, 0.0f, v);
                builder->vertexUV(0.0f, y, 0.0f, uvSize, v);
                builder->vertexUV(0.0f, y, back, uvSize, v);
                builder->vertexUV(1.0f, y, back, 0.0f, v);
            }
            builder->end();
            GlStateManager::endList(0);
            pixels = pixels + pixels;
            textureSize = textureSize + textureSize;
            depth = depth * 0.5f;
        }
    }

    if (sItemRenderDisplayLists[2] == -1)
    {
        sItemRenderDisplayLists[2] = MemoryTracker::genLists(1);
        GlStateManager::newList(sItemRenderDisplayLists[2], 0);
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->begin();
        builder->normal(0.0f, 0.0f, -1.0f);
        builder->vertexUV(0.0f, 1.0f, -0.0625f, 0.0625f, 0.0f);
        builder->vertexUV(1.0f, 1.0f, -0.0625f, 0.0f, 0.0f);
        builder->vertexUV(1.0f, 0.0f, -0.0625f, 0.0f, 0.05882353f);
        builder->vertexUV(0.0f, 0.0f, -0.0625f, 0.0625f, 0.05882353f);
        builder->end();
        GlStateManager::endList(0);
    }

    if (sItemRenderDisplayLists[3] == -1)
    {
        sItemRenderDisplayLists[3] = MemoryTracker::genLists(1);
        GlStateManager::newList(sItemRenderDisplayLists[3], 0);
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->begin();
        for (int row = 0; row < 16; ++row)
        {
            float y0 = static_cast<float>(row) * 0.0625f;
            for (int column = 0; column < 16; ++column)
            {
                float y1 = 0.0625f + y0;
                float v = 0.00091911765f + static_cast<float>(15 - row) / 544.0f;
                float x0 = static_cast<float>(column) * 0.0625f;
                float u = static_cast<float>(15 - column) * 0.00390625f + 0.001953125f;
                float x1 = x0 + 0.0625f;
                builder->normal(0.0f, 0.0f, 1.0f);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->normal(0.0f, 0.0f, -1.0f);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->normal(-1.0f, 0.0f, 0.0f);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->normal(1.0f, 0.0f, 0.0f);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->normal(0.0f, 1.0f, 0.0f);
                builder->vertexUV(x1, y0, 0.0f, u, v);
                builder->vertexUV(x0, y0, 0.0f, u, v);
                builder->vertexUV(x0, y0, -0.0625f, u, v);
                builder->vertexUV(x1, y0, -0.0625f, u, v);
                builder->normal(0.0f, -1.0f, 0.0f);
                builder->vertexUV(x1, y1, -0.0625f, u, v);
                builder->vertexUV(x0, y1, -0.0625f, u, v);
                builder->vertexUV(x0, y1, 0.0f, u, v);
                builder->vertexUV(x1, y1, 0.0f, u, v);
            }
        }
        builder->end();
        GlStateManager::endList(0);
    }

    if (sItemRenderDisplayLists[4] == -1)
    {
        sItemRenderDisplayLists[4] = MemoryTracker::genLists(1);
        GlStateManager::newList(sItemRenderDisplayLists[4], 0);
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        builder->begin();
        builder->normal(0.0f, 0.0f, 1.0f);
        builder->vertexUV(0.0f, 0.0f, 0.0f, 0.0625f, 0.029411765f);
        builder->vertexUV(1.0f, 0.0f, 0.0f, 0.0f, 0.029411765f);
        builder->vertexUV(1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
        builder->vertexUV(0.0f, 1.0f, 0.0f, 0.0625f, 0.0f);
        builder->normal(0.0f, 0.0f, -1.0f);
        builder->vertexUV(0.0f, 1.0f, -0.0625f, 0.0625f, 0.0f);
        builder->vertexUV(1.0f, 1.0f, -0.0625f, 0.0f, 0.0f);
        builder->vertexUV(1.0f, 0.0f, -0.0625f, 0.0f, 0.029411765f);
        builder->vertexUV(0.0f, 0.0f, -0.0625f, 0.0625f, 0.029411765f);
        builder->normal(-1.0f, 0.0f, 0.0f);
        builder->vertexUV(0.0f, 0.0f, -0.0625f, 0.0625f, 0.029411765f);
        builder->vertexUV(0.0f, 0.0f, 0.0f, 0.0625f, 0.029411765f);
        builder->vertexUV(0.0f, 1.0f, 0.0f, 0.05859375f, 0.0f);
        builder->vertexUV(0.0f, 1.0f, -0.0625f, 0.05859375f, 0.0f);
        builder->normal(1.0f, 0.0f, 0.0f);
        builder->vertexUV(1.0f, 1.0f, -0.0625f, 0.0f, 0.0f);
        builder->vertexUV(1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
        builder->vertexUV(1.0f, 0.0f, 0.0f, 0.00390625f, 0.029411765f);
        builder->vertexUV(1.0f, 0.0f, -0.0625f, 0.00390625f, 0.029411765f);
        builder->normal(0.0f, 1.0f, 0.0f);
        builder->vertexUV(1.0f, 0.0f, 0.0f, 0.0f, 0.02757353f);
        builder->vertexUV(0.0f, 0.0f, 0.0f, 0.0625f, 0.029411765f);
        builder->vertexUV(0.0f, 0.0f, -0.0625f, 0.0625f, 0.029411765f);
        builder->vertexUV(1.0f, 0.0f, -0.0625f, 0.0f, 0.02757353f);
        builder->normal(0.0f, -1.0f, 0.0f);
        builder->vertexUV(1.0f, 1.0f, -0.0625f, 0.0f, 0.0f);
        builder->vertexUV(0.0f, 1.0f, -0.0625f, 0.0625f, 0.0018382353f);
        builder->vertexUV(0.0f, 1.0f, 0.0f, 0.0625f, 0.0018382353f);
        builder->vertexUV(1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
        builder->end();
        GlStateManager::endList(0);
    }
}

unsigned long long ItemInHandRenderer::renderItem3D(Tesselator*, float x, float y, float, float, int, int, float,
                                                    eItemRenderMode mode)
{
    int listId = -1;
    switch (mode)
    {
    case ITEM_RENDER_MODE_NONE:
        listId = sItemRenderDisplayLists[3];
        break;
    case ITEM_RENDER_MODE_GUI:
        listId = sItemRenderDisplayLists[4];
        break;
    case ITEM_RENDER_MODE_3D:
        listId = sItemRenderDisplayLists[0];
        break;
    case ITEM_RENDER_MODE_FLAT:
        listId = sItemRenderDisplayLists[2];
        break;
    case ITEM_RENDER_MODE_EXTRUDED:
        listId = sItemRenderDisplayLists[1];
        break;
    case ITEM_RENDER_MODE_EXTRUDED_HIGH_RES:
        listId = sItemRenderDisplayLists[1] + 1;
        break;
    default:
        return 0;
    }
    GlStateManager::matrixMode(2);
    GlStateManager::loadIdentity();
    GlStateManager::translatef(x, y, 0.0f);
    GlStateManager::callList(listId);
    GlStateManager::loadIdentity();
    GlStateManager::matrixMode(0);
    return 0;
}

void ItemInHandRenderer::Reset()
{
    mMainHandItem = ItemInstance::EMPTY;
    mOffHandItem = ItemInstance::EMPTY;
    mMainHandHeight = 0.0f;
    mMainHandHeightOld = 0.0f;
    mOffHandHeight = 0.0f;
    mOffHandHeightOld = 0.0f;
    mListId = -1;
}

void ItemInHandRenderer::renderItem(boost::shared_ptr<LivingEntity> entity, not_null_ptr<ItemInstance> item,
                                    ItemTransforms::TransformType transform, bool leftHand, int, bool gui3d)
{
    if (item->isEmpty())
    {
        return;
    }
    Block* block = Block::byItem(item->getItem());
    GlStateManager::pushMatrix();
    if (Block::getId(block) == BlockId::OBSERVER && mMinecraft->mPlayer->GetThirdPersonView(false) == 0)
    {
        GlStateManager::rotatef(-90.0f, 0.0f, 1.0f, 0.0f);
    }
    bool wasGui3d = mItemRenderer->mRenderWithColor;
    mItemRenderer->mRenderWithColor = gui3d;
    mItemRenderer->renderStatic(item, entity, transform, 1.0f, leftHand, 0);
    mItemRenderer->mRenderWithColor = wasGui3d;
    GlStateManager::popMatrix();
}

bool ItemInHandRenderer::isTransparentBlock(Block* block)
{
    return (block != 0 && block->getRenderLayer() == 1) ? true : false;
}

void ItemInHandRenderer::enableLight(float x, float y)
{
    GlStateManager::pushMatrix();
    GlStateManager::rotatef(x, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(y, 0.0f, 1.0f, 0.0f);
    Lighting::turnOn();
    GlStateManager::popMatrix();
}

void ItemInHandRenderer::setLightValue(boost::shared_ptr<AbstractClientPlayer> player)
{
    int light = mMinecraft->mLevel->getLightColor(BlockPos(player->mX, player->mY + player->getEyeHeight(), player->mZ),
                                                  0, -1);
    GlStateManager::multiTexCoord2f((float)(light & 0xFFFF), (float)(light >> 16));
}

void ItemInHandRenderer::setPlayerBob(boost::shared_ptr<LocalPlayer> player, float partialTick)
{
    float bobX = player->mViewXRotO + (player->mViewXRot - player->mViewXRotO) * partialTick;
    float bobY = player->mViewYRotO + (player->mViewYRot - player->mViewYRotO) * partialTick;
    float xRot = player->mXRotO + (player->mXRot - player->mXRotO) * partialTick;
    float yRot = player->mYRotO + (player->mYRot - player->mYRotO) * partialTick;
    GlStateManager::rotatef((xRot - bobX) * 0.1f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef((yRot - bobY) * 0.1f, 0.0f, 1.0f, 0.0f);
}

float ItemInHandRenderer::calculateMapTilt(float xRot)
{
    float tilt = Mth::clamp(1.1f - xRot / 45.0f, 0.0f, 1.0f);
    return -std::cos(tilt * 3.1415927f) * 0.5f + 0.5f;
}

void ItemInHandRenderer::renderMapHand(const HumanoidArm* arm)
{
    int texture = mMinecraft->mTextures->loadMemTexture(mMinecraft->mPlayer->mSkinLocation,
                                                        mMinecraft->mPlayer->getTexture());
    GlStateManager::bindTexture(texture);
    mMinecraft->mTextures->clearLastBoundId();
    PlayerRenderer* renderer = static_cast<PlayerRenderer*>(mEntityRenderDispatcher->getRenderer(mMinecraft->mPlayer));
    GlStateManager::pushMatrix();
    float side = arm == HumanoidArm::RIGHT ? 1.0f : -1.0f;
    GlStateManager::rotatef(92.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(45.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(side * -41.0f, 0.0f, 0.0f, 1.0f);
    GlStateManager::translatef(side * 0.2f, -1.0f, 0.45f);
    if (arm == HumanoidArm::RIGHT)
    {
        renderer->renderRightHand(mMinecraft->mPlayer);
    }
    else
    {
        renderer->renderLeftHand(mMinecraft->mPlayer);
    }
    GlStateManager::popMatrix();
}

void ItemInHandRenderer::renderMapHands()
{
    if (!mMinecraft->mPlayer->isInvisible())
    {
        GlStateManager::disableCull();
        GlStateManager::pushMatrix();
        GlStateManager::rotatef(90.0f, 0.0f, 1.0f, 0.0f);
        renderMapHand(HumanoidArm::RIGHT);
        renderMapHand(HumanoidArm::LEFT);
        GlStateManager::popMatrix();
        GlStateManager::enableCull();
    }
}

void ItemInHandRenderer::renderPlayerArm(float equip, float swing, const HumanoidArm* arm)
{
    bool right = arm != HumanoidArm::LEFT;
    float side = right ? 1.0f : -1.0f;
    float swingRoot = std::sqrt(swing);
    float swingBob = -0.3f * std::sin(swingRoot * 3.1415927f);
    float swingDrop = 0.4f * std::sin(swingRoot * 3.1415927f * 2.0f);
    float swingRoll = -0.4f * std::sin(swing * 3.1415927f);
    GlStateManager::translatef(side * (swingBob + 0.64000005f), swingDrop + -0.6f + equip * -0.6f,
                               swingRoll + -0.71999997f);
    GlStateManager::rotatef(side * 45.0f, 0.0f, 1.0f, 0.0f);
    float swingSquaredSin = std::sin(swing * swing * 3.1415927f);
    float swingRootSin = std::sin(swingRoot * 3.1415927f);
    GlStateManager::rotatef(side * swingRootSin * 70.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(side * swingSquaredSin * -20.0f, 0.0f, 0.0f, 1.0f);
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    int texture = mMinecraft->mTextures->loadMemTexture(mMinecraft->mPlayer->mSkinLocation,
                                                        mMinecraft->mPlayer->getTexture());
    GlStateManager::bindTexture(texture);
    mMinecraft->mTextures->clearLastBoundId();
    GlStateManager::translatef(-side, 3.6f, 3.5f);
    GlStateManager::rotatef(side * 120.0f, 0.0f, 0.0f, 1.0f);
    GlStateManager::rotatef(200.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(side * -135.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::translatef(side * 5.6f, 0.0f, 0.0f);
    PlayerRenderer* renderer = static_cast<PlayerRenderer*>(mEntityRenderDispatcher->getRenderer(mMinecraft->mPlayer));
    GlStateManager::disableCull();
    if (right)
    {
        renderer->renderRightHand(player);
    }
    else
    {
        renderer->renderLeftHand(player);
    }
    GlStateManager::enableCull();
}

void ItemInHandRenderer::renderMap(not_null_ptr<ItemInstance> item, int hand)
{
    GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
    GlStateManager::scalef(0.38f, 0.38f, 0.38f);
    GlStateManager::disableLighting();
    mMinecraft->mTextures->bindTexture(&MAP_BACKGROUND_LOCATION);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    GlStateManager::translatef(-0.5f, -0.5f, 0.0f);
    GlStateManager::scalef(0.0078125f, 0.0078125f, 0.0078125f);
    builder->begin();
    builder->vertexUV(-7.0f, 135.0f, 0.0f, 0.0f, 1.0f);
    builder->vertexUV(135.0f, 135.0f, 0.0f, 1.0f, 1.0f);
    builder->vertexUV(135.0f, -7.0f, 0.0f, 1.0f, 0.0f);
    builder->vertexUV(-7.0f, -7.0f, 0.0f, 0.0f, 0.0f);
    builder->end();
    boost::shared_ptr<MapItemSavedData> savedData =
        static_cast<MapItem*>(Items::filledMap)->getSavedData(item, mMinecraft->mLevel);
    if (savedData.get() != 0)
    {
        mMinecraft->mGameRenderer->getMapRenderer()->render(savedData, mMinecraft->mPlayer);
    }
    GlStateManager::enableLighting();
}

void ItemInHandRenderer::renderOneHandedMap(float equip, const HumanoidArm* arm, float swing,
                                            not_null_ptr<ItemInstance> item)
{
    float side = arm == HumanoidArm::RIGHT ? 1.0f : -1.0f;
    GlStateManager::translatef(side * 0.125f, -0.125f, 0.0f);
    if (!mMinecraft->mPlayer->isInvisible())
    {
        GlStateManager::pushMatrix();
        GlStateManager::rotatef(side * 10.0f, 0.0f, 0.0f, 1.0f);
        renderPlayerArm(equip, swing, arm);
        GlStateManager::popMatrix();
    }
    GlStateManager::pushMatrix();
    GlStateManager::translatef(side * 0.51f, -0.08f + equip * -1.2f, -0.75f);
    float swingRoot = Mth::sqrt(swing);
    float swingSin = Mth::sin(swingRoot * 3.1415927f);
    float offsetX = -0.5f * swingSin;
    float offsetY = 0.4f * Mth::sin(swingRoot * 6.2831855f);
    float offsetZ = -0.3f * Mth::sin(swing * 3.1415927f);
    GlStateManager::translatef(side * offsetX, offsetY - 0.3f * swingSin, offsetZ);
    GlStateManager::rotatef(swingSin * -45.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(side * swingSin * -30.0f, 0.0f, 1.0f, 0.0f);
    renderMap(item, arm == HumanoidArm::RIGHT ? 0 : 1);
    GlStateManager::popMatrix();
}

void ItemInHandRenderer::renderTwoHandedMap(float pitch, float equipProgress, float swingProgress)
{
    float swingRoot = Mth::sqrt(swingProgress);
    float swingY = -0.17f * Mth::sin(swingProgress * 3.1415927f);
    float swingZ = -0.17f * Mth::sin(swingRoot * 3.1415927f);
    GlStateManager::translatef(0.0f, -swingY / 2.0f, swingZ);
    float mapTilt = calculateMapTilt(pitch);
    GlStateManager::translatef(0.0f, 0.1f + equipProgress * -1.2f + mapTilt * -0.5f, -0.72f);
    GlStateManager::rotatef(mapTilt * -85.0f, 1.0f, 0.0f, 0.0f);
    renderMapHands();
    float swingRootSin = Mth::sin(swingRoot * 3.1415927f);
    GlStateManager::rotatef(swingRootSin * 20.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::scalef(1.75f, 1.75f, 1.75f);
    renderMap(mMainHandItem, 0);
}

void ItemInHandRenderer::applyEatTransform(float partialTick, const HumanoidArm* arm, not_null_ptr<ItemInstance> item)
{
    float remainingTicks = (float)mMinecraft->mPlayer->getUseItemRemainingTicks() - partialTick + 1.0f;
    float useProgress = remainingTicks / (float)item->getUseDuration();
    float wobble = Mth::cos(remainingTicks * 0.25f * 3.1415927f);
    float bob = std::fabs(wobble * 0.1f);
    GlStateManager::translatef(0.0f, useProgress < 0.8f ? bob : 0.0f, 0.0f);
    float pull = 1.0f - std::pow(useProgress, 27.0f);
    int side = arm == HumanoidArm::RIGHT ? 1 : -1;
    GlStateManager::translatef(pull * 0.6f * (float)side, pull * -0.5f, pull * 0.0f);
    GlStateManager::rotatef((float)side * pull * 90.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(pull * 10.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef((float)side * pull * 30.0f, 0.0f, 0.0f, 1.0f);
}

void ItemInHandRenderer::applyItemArmAttackTransform(const HumanoidArm* arm, float swingProgress)
{
    int side = arm == HumanoidArm::RIGHT ? 1 : -1;
    float swing = Mth::sin(swingProgress * swingProgress * 3.1415927f);
    GlStateManager::rotatef((float)side * (45.0f + swing * -20.0f), 0.0f, 1.0f, 0.0f);
    float swingRoot = Mth::sin(Mth::sqrt(swingProgress) * 3.1415927f);
    GlStateManager::rotatef((float)side * swingRoot * -20.0f, 0.0f, 0.0f, 1.0f);
    GlStateManager::rotatef(swingRoot * -80.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef((float)side * -45.0f, 0.0f, 1.0f, 0.0f);
}

void ItemInHandRenderer::applyItemArmTransform(const HumanoidArm* arm, float equipProgress)
{
    int side = arm == HumanoidArm::RIGHT ? 1 : -1;
    GlStateManager::translatef((float)side * 0.56f, -0.52f + equipProgress * -0.6f, -0.72f);
}

void ItemInHandRenderer::renderArmWithItem(boost::shared_ptr<LocalPlayer> player, float partialTick, float pitch,
                                           InteractionHand::EInteractionHand hand, float swingProgress,
                                           not_null_ptr<ItemInstance> item, float equipProgress)
{
    bool mainHand = hand == InteractionHand::eInteractionHand_MainHand;
    const HumanoidArm* arm;
    if (mainHand)
        arm = player->getMainArm();
    else
        arm = player->getMainArm()->getOpposite();

    float offsetX = 0.0f;
    float offsetY = 0.0f;
    bool splitScreenVertical = false;
    if (player->mViewport == 2 || player->mViewport == 1)
    {
        splitScreenVertical = true;
        offsetY = 0.08f;
    }
    else if (player->mViewport == 3 || player->mViewport == 4 || !gRenderer->IsWidescreen())
    {
        if (arm == HumanoidArm::LEFT)
            offsetX = 0.3f;
        else
            offsetX = -0.3f;
    }

    GlStateManager::pushMatrix();
    if (item->isEmpty())
    {
        if (mainHand && !player->isInvisible())
        {
            GlStateManager::translatef(offsetX, offsetY, 0.0f);
            renderPlayerArm(equipProgress, swingProgress, arm);
        }
        GlStateManager::popMatrix();
        return;
    }

    if (item->getItem() == Items::filledMap)
    {
        if (splitScreenVertical)
            GlStateManager::translatef(0.0f, 0.0f, -0.3f);
        if (mainHand && mOffHandItem->isEmpty())
        {
            renderTwoHandedMap(pitch, equipProgress, swingProgress);
        }
        else
        {
            GlStateManager::translatef(offsetX, offsetY, 0.0f);
            renderOneHandedMap(equipProgress, arm, swingProgress, item);
        }
        GlStateManager::popMatrix();
        return;
    }

    bool rightArm = arm == HumanoidArm::RIGHT;
    if (player->isUsingItem() && player->getUsedItemHand() == hand)
    {
        int side = rightArm ? 1 : -1;
        switch (item->getUseAnimation())
        {
        case UseAnim_none:
            GlStateManager::translatef(offsetX, offsetY, 0.0f);
            applyItemArmTransform(arm, equipProgress);
            break;
        case UseAnim_eat:
        case UseAnim_drink:
            applyEatTransform(partialTick, arm, item);
            applyItemArmTransform(arm, equipProgress);
            break;
        case UseAnim_block:
        {
            applyItemArmTransform(arm, equipProgress);
            if (item->getItem() == Items::woodenSword || item->getItem() == Items::stoneSword
                || item->getItem() == Items::ironSword || item->getItem() == Items::goldenSword
                || item->getItem() == Items::diamondSword)
            {
                GlStateManager::translatef((float)side * -0.2f, 0.08f, 0.0f);
                GlStateManager::rotatef(-80.0f, 1.0f, 0.0f, 0.0f);
                GlStateManager::rotatef((float)side * 10.0f, 0.0f, 1.0f, 0.0f);
                GlStateManager::rotatef((float)side * 60.0f, 0.0f, 0.0f, 1.0f);
            }
            break;
        }
        case UseAnim_bow:
        {
            GlStateManager::translatef(offsetX, offsetY, 0.0f);
            applyItemArmTransform(arm, equipProgress);
            GlStateManager::translatef((float)side * -0.2785682f, 0.18344387f, 0.15731531f);
            GlStateManager::rotatef(-13.935f, 1.0f, 0.0f, 0.0f);
            GlStateManager::rotatef((float)side * 35.3f, 0.0f, 1.0f, 0.0f);
            GlStateManager::rotatef((float)side * -9.785f, 0.0f, 0.0f, 1.0f);
            float drawTime = (float)item->getUseDuration()
                             - ((float)mMinecraft->mPlayer->getUseItemRemainingTicks() - partialTick + 1.0f);
            float power = drawTime / (float)BowItem::GetMaxBowDuration();
            power = (power * power + power * 2.0f) / 3.0f;
            if (power > 1.0f)
                power = 1.0f;
            if (power > 0.1f)
            {
                float pull = Mth::sin((drawTime - 0.1f) * 1.3f);
                float shake = pull * (power - 0.1f);
                GlStateManager::translatef(shake * 0.0f, shake * 0.004f, shake * 0.0f);
            }
            GlStateManager::translatef(power * 0.0f, power * 0.0f, power * 0.04f);
            GlStateManager::scalef(1.0f, 1.0f, 1.0f + power * 0.2f);
            GlStateManager::rotatef((float)side * 45.0f, 0.0f, -1.0f, 0.0f);
            break;
        }
        case UseAnim_spear:
        {
            applyItemArmTransform(arm, equipProgress);
            EnchantmentHelper::getEnchantmentLevel(Enchantments::RIPTIDE, player);
            float drawTime = (float)item->getUseDuration()
                             - ((float)mMinecraft->mPlayer->getUseItemRemainingTicks() - partialTick + 1.0f);
            float power = Mth::clamp(drawTime / 10.0f, 0.0f, 1.0f);
            GlStateManager::translatef(0.0f, 0.85f, 0.0f);
            GlStateManager::translatef(0.0f, 0.0f, power * 0.3f - 0.2f);
            GlStateManager::rotatef(-60.0f, 1.0f, 0.0f, 0.0f);
            GlStateManager::rotatef(-10.0f, 0.0f, 1.0f, 0.0f);
            if (power > 0.1f)
            {
                float pull = Mth::sin((drawTime - 0.1f) * 1.3f);
                GlStateManager::translatef(0.0f, pull * 0.01f * (power - 0.1f), 0.0f);
            }
            break;
        }
        default:
            break;
        }
    }
    else if (player->getDamageNearbyMobs())
    {
        applyItemArmTransform(arm, equipProgress);
        GlStateManager::translatef(0.25f, 1.0f, -0.5f);
        GlStateManager::rotatef(20.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(-90.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(-45.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::scalef(2.5f, 2.5f, 2.5f);
    }
    else
    {
        if (swingProgress == 0.0f)
            GlStateManager::translatef(offsetX, offsetY, 0.0f);
        float swingX = -0.4f * Mth::sin(Mth::sqrt(swingProgress) * 3.1415927f);
        float swingY = 0.2f * Mth::sin(Mth::sqrt(swingProgress) * 6.2831855f);
        float swingZ = -0.2f * Mth::sin(swingProgress * 3.1415927f);
        int side = rightArm ? 1 : -1;
        GlStateManager::translatef((float)side * swingX, swingY, swingZ);
        applyItemArmTransform(arm, equipProgress);
        applyItemArmAttackTransform(arm, swingProgress);
    }

    renderItem(player, item,
               rightArm ? ItemTransforms::FIRST_PERSON_RIGHT_HAND : ItemTransforms::FIRST_PERSON_LEFT_HAND, !rightArm,
               0, true);
    GlStateManager::popMatrix();
}

#ifdef MINECRAFT_PC
static const float MOD_ARM_SCALE = 0.5f;
static const float MOD_FIST_Y = 10.0f * 0.0625f;

void ItemInHandRenderer::renderModItem(const boost::shared_ptr<ItemInstance>& item, const float* viewFromItem)
{
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    if (player == NULL || item == NULL || item->isEmpty())
        return;
    not_null_ptr<ItemInstance> shared(item);
    setLightValue(player);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::pushMatrix();
    GlStateManager::loadIdentity();
    GlStateManager::glMultMatrixf(const_cast<float*>(viewFromItem));
    renderItem(player, shared, ItemTransforms::FIXED, false, 0, true);
    GlStateManager::popMatrix();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void ItemInHandRenderer::renderModHand(const HumanoidArm* arm, const float* viewFromHand)
{
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    const bool right = arm == HumanoidArm::RIGHT;
    const bool mainArm = player->getMainArm() == arm;
    not_null_ptr<ItemInstance> item = mainArm ? mMainHandItem : mOffHandItem;
    PlayerRenderer* renderer = static_cast<PlayerRenderer*>(mEntityRenderDispatcher->getRenderer(mMinecraft->mPlayer));
    setLightValue(player);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::pushMatrix();
    GlStateManager::loadIdentity();
    GlStateManager::glMultMatrixf(const_cast<float*>(viewFromHand));
    GlStateManager::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::scalef(MOD_ARM_SCALE, MOD_ARM_SCALE, MOD_ARM_SCALE);
    GlStateManager::translatef((right ? 6.0f : -6.0f) * 0.0625f, -12.0f * 0.0625f, 0.0f);
    if (!player->isInvisible())
    {
        int texture = mMinecraft->mTextures->loadMemTexture(player->mSkinLocation, player->getTexture());
        GlStateManager::bindTexture(texture);
        mMinecraft->mTextures->clearLastBoundId();
        GlStateManager::disableCull();
        if (right)
            renderer->renderRightHand(player);
        else
            renderer->renderLeftHand(player);
        GlStateManager::enableCull();
    }
    const bool heldMap = !item->isEmpty() && item->getItem() == Items::filledMap;
    if (!item->isEmpty() && !heldMap)
    {
        if (item->getItem() != NULL)
        {
            int tint = item->getItem()->getColor(item, 0);
            GlStateManager::color4f((float)((tint >> 16) & 0xFF) / 255.0f, (float)((tint >> 8) & 0xFF) / 255.0f,
                                    (float)(tint & 0xFF) / 255.0f, 1.0f);
        }
        static_cast<HumanoidModel*>(renderer->getModel())->translateToHandItem(0.0625f, arm);
        Block* block = item->getItem() != NULL ? Block::byItem(item->getItem()) : NULL;
        if (block != NULL && item->getItem()->getIconType() == 0
            && BlockRenderer::canRender(block->defaultBlockState()->getRenderShape()))
        {
            const float middle = (right ? -1.0f : 1.0f) * 0.0625f;
            GlStateManager::translatef(middle, 0.0f, 0.0f);
            GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
            GlStateManager::translatef(-middle, 0.0f, 0.0f);
        }
        GlStateManager::translatef(0.0f, MOD_FIST_Y, 0.0f);
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(0.0f, -MOD_FIST_Y, 0.0f);
        GlStateManager::rotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::translatef((right ? 1.0f : -1.0f) * 0.0625f, 0.125f, -0.625f);
        renderItem(player, item, right ? ItemTransforms::THIRD_PERSON_RIGHT_HAND : ItemTransforms::THIRD_PERSON_LEFT_HAND,
                   !right, 0, true);
    }
    GlStateManager::popMatrix();
    if (heldMap)
    {
        GlStateManager::pushMatrix();
        GlStateManager::loadIdentity();
        GlStateManager::glMultMatrixf(const_cast<float*>(viewFromHand));
        GlStateManager::translatef(0.0f, 0.06f, -0.14f);
        GlStateManager::rotatef(-45.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::scalef(0.7f, 0.7f, 0.7f);
        GlStateManager::disableCull();
        renderMap(item, right ? 1 : 0);
        GlStateManager::enableCull();
        GlStateManager::popMatrix();
    }
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}
#endif

void ItemInHandRenderer::render(float partialTick)
{
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    float attackProgress = player->getAttackAnim(partialTick);
    InteractionHand::EInteractionHand swingingArm = player->mSwingingArm != InteractionHand::eInteractionHand_MainHand
                                                        ? player->mSwingingArm
                                                        : InteractionHand::eInteractionHand_MainHand;
    float pitch = player->mXRotO + (player->mXRot - player->mXRotO) * partialTick;
    float yaw = player->mYRotO + (player->mYRot - player->mYRotO) * partialTick;
    bool renderMainHand = true;
    bool renderOffHand = true;
    if (player->isUsingItem())
    {
        not_null_ptr<ItemInstance> useItem = player->getUseItem();
        if (useItem->getItem() == Items::bow)
        {
            InteractionHand::EInteractionHand usedHand = player->getUsedItemHand();
            bool usedMainHand = usedHand == InteractionHand::eInteractionHand_MainHand;
            renderMainHand = usedMainHand;
            renderOffHand = !usedMainHand;
        }
    }
    enableLight(pitch, yaw);
    setLightValue(player);
    setPlayerBob(player, partialTick);
    GlStateManager::enableRescaleNormal();
    if (renderMainHand)
    {
        if (!mMainHandItem->isEmpty())
        {
            int tint = mMainHandItem->getItem()->getColor(mMainHandItem, 0);
            GlStateManager::color4f((float)((tint >> 16) & 0xFF) / 255.0f, (float)((tint >> 8) & 0xFF) / 255.0f,
                                    (float)(tint & 0xFF) / 255.0f, 1.0f);
        }
        else
        {
            GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
        float mainSwing = swingingArm == InteractionHand::eInteractionHand_MainHand ? attackProgress : 0.0f;
        float mainHandHeight = mMainHandHeightOld + (mMainHandHeight - mMainHandHeightOld) * partialTick;
        float mainEquip = 1.0f - mainHandHeight;
        renderArmWithItem(player, partialTick, pitch, InteractionHand::eInteractionHand_MainHand, mainSwing,
                          mMainHandItem, mainEquip);
    }
    if (renderOffHand)
    {
        if (!mOffHandItem->isEmpty())
        {
            int tint = mOffHandItem->getItem()->getColor(mOffHandItem, 0);
            GlStateManager::color4f((float)((tint >> 16) & 0xFF) / 255.0f, (float)((tint >> 8) & 0xFF) / 255.0f,
                                    (float)(tint & 0xFF) / 255.0f, 1.0f);
        }
        else
        {
            GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
        float offSwing = swingingArm == InteractionHand::eInteractionHand_OffHand ? attackProgress : 0.0f;
        float offHandHeight = mOffHandHeightOld + (mOffHandHeight - mOffHandHeightOld) * partialTick;
        float offEquip = 1.0f - offHandHeight;
        renderArmWithItem(player, partialTick, pitch, InteractionHand::eInteractionHand_OffHand, offSwing, mOffHandItem,
                          offEquip);
    }
    GlStateManager::disableRescaleNormal();
    Lighting::turnOff();
}

void ItemInHandRenderer::renderTex(TextureAtlasSprite* sprite)
{
    mMinecraft->mTextures->bindTexture(&BLOCKS_LOCATION);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    GlStateManager::color4f(0.1f, 0.1f, 0.1f, 0.5f);
    GlStateManager::pushMatrix();
    float u0 = (float)sprite->getU0(false);
    float u1 = (float)sprite->getU1(false);
    float v0 = (float)sprite->getV0(false);
    float v1 = (float)sprite->getV1(false);
    builder->begin();
    builder->vertexUV(-1.0f, -1.0f, -0.5f, u1, v1);
    builder->vertexUV(1.0f, -1.0f, -0.5f, u0, v1);
    builder->vertexUV(1.0f, 1.0f, -0.5f, u0, v0);
    builder->vertexUV(-1.0f, 1.0f, -0.5f, u1, v0);
    builder->end();
    GlStateManager::popMatrix();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void ItemInHandRenderer::renderWater(float)
{
    mMinecraft->mTextures->bindTexture(&WATER_OVERLAY_LOCATION);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    float brightness = mMinecraft->mPlayer->getBrightness();
    GlStateManager::color4f(brightness, brightness, brightness, 0.1f);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 1);
    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
    GlStateManager::pushMatrix();
    float uvRepeat = 4.0f;
    float x0 = -1.0f;
    float x1 = 1.0f;
    float y0 = -1.0f;
    float y1 = 1.0f;
    float z = -0.5f;
    float u = -(mMinecraft->mPlayer->mYRot * 0.015625f);
    float v = mMinecraft->mPlayer->mXRot * 0.015625f;
    builder->begin();
    builder->vertexUV(x0, y0, z, uvRepeat + u, uvRepeat + v);
    builder->vertexUV(x1, y0, z, u, uvRepeat + v);
    builder->vertexUV(x1, y1, z, u, v);
    builder->vertexUV(x0, y1, z, uvRepeat + u, v);
    tesselator->end();
    GlStateManager::popMatrix();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
}

void ItemInHandRenderer::renderFire()
{
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Fire_Overlay);
    float r = (float)((colour >> 16) & 0xFF) / 255.0f;
    float g = (float)((colour >> 8) & 0xFF) / 255.0f;
    float b = (float)(colour & 0xFF) / 255.0f;
    float a = (float)(colour >> 24) / 255.0f;
    GlStateManager::color4f(r, g, b, a);
    GlStateManager::depthFunc(7);
    GlStateManager::depthMask(false);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    for (int i = 0; i < 2; ++i)
    {
        GlStateManager::pushMatrix();
        TextureAtlasSprite* sprite = static_cast<FireBlock*>(Blocks::fire)->getTextureLayer(1);
        mMinecraft->mTextures->bindTexture(&BLOCKS_LOCATION);
        float u0 = (float)sprite->getU0(true);
        float u1 = (float)sprite->getU1(true);
        float v0 = (float)sprite->getV0(true);
        float v1 = (float)sprite->getV1(true);
        float x0 = -0.5f;
        float x1 = 0.5f;
        float y0 = -0.5f;
        float y1 = 0.5f;
        float z = -0.5f;
        GlStateManager::translatef((float)(-(i * 2 - 1)) * 0.24f, -0.3f, 0.0f);
        GlStateManager::rotatef((float)(i * 2 - 1) * 10.0f, 0.0f, 1.0f, 0.0f);
        builder->begin();
        builder->vertexUV(x0, y0, z, u1, v1);
        builder->vertexUV(x1, y0, z, u0, v1);
        builder->vertexUV(x1, y1, z, u0, v0);
        builder->vertexUV(x0, y1, z, u1, v0);
        builder->end();
        GlStateManager::popMatrix();
    }
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::depthFunc(3);
}

void ItemInHandRenderer::renderScreenEffect(float partialTick)
{
    GlStateManager::disableAlphaTest();
    if (mMinecraft->mPlayer->isInWall())
    {
        const BlockState* state = mMinecraft->mLevel->getBlockState(BlockPos(mMinecraft->mPlayer));
        boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
        for (int sample = 0; sample < 8; ++sample)
        {
            double x = player->mX + (static_cast<float>((sample >> 0) % 2) - 0.5f) * player->mBbWidth * 0.8f;
            double y = player->mY + (static_cast<float>((sample >> 1) % 2) - 0.5f) * 0.1f;
            double z = player->mZ + (static_cast<float>((sample >> 2) % 2) - 0.5f) * player->mBbWidth * 0.8f;
            const BlockPos samplePos(x, y + player->getEyeHeight(), z);
            const BlockState* sampleState = mMinecraft->mLevel->getBlockState(samplePos);
            if (sampleState->isViewBlocking())
            {
                state = sampleState;
            }
        }
        if (state->getBlock()->defaultBlockState()->getRenderShape() != BLOCK_RENDER_INVISIBLE)
        {
            renderTex(state->getBlock()->getIconTexture(Direction::UP, state));
        }
    }
    if (!mMinecraft->mPlayer->isSpectator())
    {
        if (mMinecraft->mPlayer->isUnderLiquid(Material::WATER))
        {
            renderWater(partialTick);
        }
        if (mMinecraft->mPlayer->isOnFire())
        {
            renderFire();
        }
    }
    GlStateManager::enableAlphaTest();
}

void ItemInHandRenderer::CalculateNewHandHeights(bool compareTags)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mPlayer;
    not_null_ptr<ItemInstance> mainHandItem = player->getMainHandItem();
    not_null_ptr<ItemInstance> offHandItem = player->getOffhandItem();
    float attackStrength = player->getAttackStrengthScale(1.0f);
    float mainHandTarget = ItemInstance::matches(mMainHandItem, mainHandItem, compareTags)
                               ? attackStrength * attackStrength * attackStrength
                               : 0.0f;
    mMainHandHeight += Mth::clamp(mainHandTarget - mMainHandHeight, -0.4f, 0.4f);
    float offHandTarget = ItemInstance::matches(mOffHandItem, offHandItem, compareTags) ? 1.0f : 0.0f;
    mOffHandHeight += Mth::clamp(offHandTarget - mOffHandHeight, -0.4f, 0.4f);
}

void ItemInHandRenderer::tick()
{
    mMainHandHeightOld = mMainHandHeight;
    mOffHandHeightOld = mOffHandHeight;
    boost::shared_ptr<LocalPlayer> player = mMinecraft->mPlayer;
    not_null_ptr<ItemInstance> mainHandItem = player->getMainHandItem();
    not_null_ptr<ItemInstance> offHandItem = player->getOffhandItem();
    if (player->isHandsBusy())
    {
        mMainHandHeight = Mth::clamp(mMainHandHeight - 0.4f, 0.0f, 1.0f);
        mOffHandHeight = Mth::clamp(mOffHandHeight - 0.4f, 0.0f, 1.0f);
    }
    else
    {
        bool bothMaps = mMainHandItem->getItem() == Items::filledMap && mainHandItem->getItem() == Items::filledMap;
        bool specialMapAux = mMainHandItem->getAuxValue() == 255 || mMainHandItem->getAuxValue() == 254
                             || mMainHandItem->getAuxValue() == 253;
        if (bothMaps && specialMapAux)
        {
            CalculateNewHandHeights(false);
            mMainHandItem = mainHandItem;
            mOffHandItem = offHandItem;
        }
        else
        {
            CalculateNewHandHeights(true);
        }
    }
    if (mMainHandHeight < 0.1f)
    {
        mMainHandItem = mainHandItem;
    }
    if (mOffHandHeight < 0.1f)
    {
        mOffHandItem = offHandItem;
    }
}

void ItemInHandRenderer::itemUsed(InteractionHand::EInteractionHand hand)
{
    if (hand == InteractionHand::eInteractionHand_MainHand)
    {
        mMainHandHeight = 0.0f;
    }
    else
    {
        mOffHandHeight = 0.0f;
    }
}
