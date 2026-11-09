#include "game/client/render/ShulkerBoxRenderer.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/ShulkerModel.hpp"

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"

static ResourceLocation SHULKER_BOX_LOCATION(TN_MOB_SHULKER_ENDERGOLEM);

ShulkerBoxRenderer::ShulkerBoxRenderer(ShulkerModel* model) : mModel(model), mShulkerModel(model)
{
}

ShulkerBoxRenderer::~ShulkerBoxRenderer()
{
}

void ShulkerBoxRenderer::setupForTexturePack(TexturePack* texturePack)
{
    if (mShulkerModel == NULL)
    {
        mShulkerModel = static_cast<ShulkerModel*>(mModel);
    }

    if (mModel != mShulkerModel)
    {
        delete mModel;
        mModel = NULL;
    }

    if (texturePack->HasCustomModel(mShulkerModel->getName()))
    {
        mModel = new ShulkerModel(*texturePack->GetCustomModel(mShulkerModel->getName()));
    }
    else
    {
        mModel = mShulkerModel;
    }
}

void ShulkerBoxRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                                float partialTick, int breakProgress, bool translucent, float alpha,
                                bool useDisplayList)
{
    boost::shared_ptr<ShulkerBoxBlockEntity> shulker = boost::static_pointer_cast<ShulkerBoxBlockEntity>(blockEntity);

    const Direction* direction = Direction::UP;
    if (shulker->hasLevel())
    {
        const BlockState* state = getLevel()->getBlockState(shulker->getBlockPos());
        if (dynamic_cast<ShulkerBoxBlock*>(state->getBlock()) != NULL)
            direction = state->getValue(DirectionalBlock::FACING);
    }

    GlStateManager::enableDepthTest();
    GlStateManager::depthFunc(3);
    GlStateManager::depthMask(true);
    GlStateManager::disableCull();

    if (breakProgress >= 0)
    {
        bindTexture(&BREAKING_LOCATIONS[breakProgress]);
        GlStateManager::matrixMode(2);
        GlStateManager::pushMatrix();
        GlStateManager::scalef(4.0F, 4.0F, 1.0F);
        GlStateManager::translatef(0.0625F, 0.0625F, 0.0625F);
        GlStateManager::matrixMode(0);
    }
    else
    {
        bindTexture(&SHULKER_BOX_LOCATION);
    }

    GlStateManager::pushMatrix();
    GlStateManager::enableRescaleNormal();
    GlStateManager::translatef((float)x + 0.5F, (float)y + 1.5F, (float)z + 0.5F);
    GlStateManager::scalef(1.0F, -1.0F, -1.0F);
    GlStateManager::translatef(0.0F, 1.0F, 0.0F);
    GlStateManager::scalef(0.9995F, 0.9995F, 0.9995F);
    GlStateManager::translatef(0.0F, -1.0F, 0.0F);

    switch (direction->get3DDataValue())
    {
    case 0:
        GlStateManager::translatef(0.0F, 2.0F, 0.0F);
        GlStateManager::rotatef(180.0F, 1.0F, 0.0F, 0.0F);
        break;
    case 2:
        GlStateManager::translatef(0.0F, 1.0F, 1.0F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager::rotatef(180.0F, 0.0F, 0.0F, 1.0F);
        break;
    case 3:
        GlStateManager::translatef(0.0F, 1.0F, -1.0F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        break;
    case 4:
        GlStateManager::translatef(-1.0F, 1.0F, 0.0F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager::rotatef(-90.0F, 0.0F, 0.0F, 1.0F);
        break;
    case 5:
        GlStateManager::translatef(1.0F, 1.0F, 0.0F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        GlStateManager::rotatef(90.0F, 0.0F, 0.0F, 1.0F);
        break;
    default:
        break;
    }

    if (breakProgress >= 0)
    {
        if (mModel->hasOverriddenParts())
        {
            BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
            builder->setVertexAndPixelShader((C4JRender::eVertexType)0, (C4JRender::ePixelShaderType)0);
            static_cast<ShulkerModel*>(mModel)->mBase->render(0.0625F, false, false);
            GlStateManager::pushMatrix();
            GlStateManager::translatef(
                0.0F, -shulker->getProgress(partialTick) * ShulkerBoxBlockEntity::LID_HEIGHT_FACTOR, 0.0F);
            GlStateManager::rotatef(ShulkerBoxBlockEntity::LID_ROTATION_DEGREES * shulker->getProgress(partialTick),
                                    0.0F, 1.0F, 0.0F);
            static_cast<ShulkerModel*>(mModel)->mLid->render(0.0625F, false, false);
            GlStateManager::popMatrix();
            builder->useDefaultShaders();
        }
        else
        {
            static_cast<ShulkerModel*>(mModel)->mBase->render(0.0625F, useDisplayList, false);
            GlStateManager::translatef(
                0.0F, -shulker->getProgress(partialTick) * ShulkerBoxBlockEntity::LID_HEIGHT_FACTOR, 0.0F);
            GlStateManager::rotatef(ShulkerBoxBlockEntity::LID_ROTATION_DEGREES * shulker->getProgress(partialTick),
                                    0.0F, 1.0F, 0.0F);
            static_cast<ShulkerModel*>(mModel)->mLid->render(0.0625F, useDisplayList, false);
        }
    }
    else
    {
        const int colour = Minecraft::GetInstance()->getColourTable()->getColour(
            shulker->getColor()->getShulkerColor());
        GlStateManager::color4f((float)((colour >> 16) & 0xFF) / 255.0F, (float)((colour >> 8) & 0xFF) / 255.0F,
                                (float)(colour & 0xFF) / 255.0F, alpha);
        if (mModel->hasOverriddenParts())
        {
            static_cast<ShulkerModel*>(mModel)->mHead->mShow = false;
            static_cast<ShulkerModel*>(mModel)->mBase->render(0.0625F, useDisplayList, false);
            GlStateManager::pushMatrix();
            GlStateManager::translatef(
                0.0F, -shulker->getProgress(partialTick) * ShulkerBoxBlockEntity::LID_HEIGHT_FACTOR, 0.0F);
            GlStateManager::rotatef(ShulkerBoxBlockEntity::LID_ROTATION_DEGREES * shulker->getProgress(partialTick),
                                    0.0F, 1.0F, 0.0F);
            static_cast<ShulkerModel*>(mModel)->mLid->render(0.0625F, useDisplayList, false);
            GlStateManager::popMatrix();
            static_cast<ShulkerModel*>(mModel)->mHead->render(0.0625F, useDisplayList, false);
            mModel->callSkinnedDrawlist();
            GlStateManager::color3f(1.0F, 1.0F, 1.0F);
        }
        else
        {
            static_cast<ShulkerModel*>(mModel)->mBase->render(0.0625F, useDisplayList, false);
            GlStateManager::pushMatrix();
            GlStateManager::translatef(
                0.0F, -shulker->getProgress(partialTick) * ShulkerBoxBlockEntity::LID_HEIGHT_FACTOR, 0.0F);
            GlStateManager::rotatef(ShulkerBoxBlockEntity::LID_ROTATION_DEGREES * shulker->getProgress(partialTick),
                                    0.0F, 1.0F, 0.0F);
            static_cast<ShulkerModel*>(mModel)->mLid->render(0.0625F, useDisplayList, false);
            GlStateManager::popMatrix();
            GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
            GlStateManager::matrixMode(2);
            GlStateManager::loadIdentity();
            GlStateManager::translatef(0.0F, 0.5F, 0.0F);
            GlStateManager::matrixMode(0);
            static_cast<ShulkerModel*>(mModel)->mBase->render(0.0625F, useDisplayList, false);
            GlStateManager::translatef(
                0.0F, -shulker->getProgress(partialTick) * ShulkerBoxBlockEntity::LID_HEIGHT_FACTOR, 0.0F);
            GlStateManager::rotatef(ShulkerBoxBlockEntity::LID_ROTATION_DEGREES * shulker->getProgress(partialTick),
                                    0.0F, 1.0F, 0.0F);
            static_cast<ShulkerModel*>(mModel)->mLid->render(0.0625F, useDisplayList, false);
            GlStateManager::matrixMode(2);
            GlStateManager::loadIdentity();
            GlStateManager::translatef(0.0F, 0.0F, 0.0F);
            GlStateManager::matrixMode(0);
        }
    }

    GlStateManager::enableCull();
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    if (breakProgress >= 0)
    {
        GlStateManager::matrixMode(2);
        GlStateManager::popMatrix();
        GlStateManager::matrixMode(0);
    }
}
