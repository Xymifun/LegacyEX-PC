#include "game/client/render/ShulkerModel.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Shulker.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/ModelPartsSkinned.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"

ShulkerModel::ShulkerModel(const ModelDefinition& modelDefinition) : mSkinnedParts()
{
    mTexWidth = 64;
    mTexHeight = 128;

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mLid = new ModelPartSkin(this, L"lid", modelDefinition, NULL, 0.0f, NULL);
        mBase = new ModelPartSkin(this, L"base", modelDefinition, NULL, 0.0f, NULL);
        mHeadSkin = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, &mSkinnedParts);
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Shulker);
        compileSkinnedDrawlist();
        mSkinnedParts.compile(0.0625f);
        builder->useDefaultShaders();
    }
    else
    {
        mLid = new ModelPart(this);
        mBase = new ModelPart(this);
        mHead = new ModelPart(this);
        mHeadSkin = NULL;
        mLid->texOffs(0, 0)->addBox(-8.0f, -16.0f, -8.0f, 16, 12, 16);
        mLid->setPos(0.0f, 24.0f, 0.0f);
        mBase->texOffs(0, 28)->addBox(-8.0f, -8.0f, -8.0f, 16, 8, 16);
        mBase->setPos(0.0f, 24.0f, 0.0f);
        mHead->texOffs(0, 116)->addBox(-3.0f, 0.0f, -3.0f, 6, 6, 6);
        mHead->setPos(0.0f, 12.0f, 0.0f);
        mLid->compile(0.0625f);
        mBase->compile(0.0625f);
        mHead->compile(0.0625f);
    }
}

ShulkerModel::~ShulkerModel()
{
    delete mLid;
    delete mBase;
    delete mHead;
    delete mHeadSkin;
}

void ShulkerModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                             float headPitch, float scale, boost::shared_ptr<Entity> entity,
                             _SkinAdjustments skinAdjustments)
{
    boost::shared_ptr<Shulker> shulker = boost::static_pointer_cast<Shulker>(entity);
    const float time = ageInTicks - (float)shulker->mTickCount;
    const float peekAngle = (shulker->getClientPeekAmount(time) + 0.5f) * 3.1415927f;
    const float lidTwist = sinf(peekAngle) + -1.0f;
    float bob = 0.0f;
    if (peekAngle > 3.1415927f)
        bob = sinf(ageInTicks * 0.1f) * 0.7f;
    mLid->setPos(0.0f, sinf(peekAngle) * 8.0f + 16.0f + bob, 0.0f);
    if (shulker->getClientPeekAmount(time) > 0.3f)
        mLid->mYRot = lidTwist * lidTwist * lidTwist * lidTwist * 3.1415927f * 0.125f;
    else
        mLid->mYRot = 0.0f;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    if (mHeadSkin != NULL)
    {
        mHeadSkin->mXRot = headPitch * Mth::DEG_TO_RAD;
        mHeadSkin->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    }
}

void ShulkerModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                          float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    boost::shared_ptr<Shulker> shulker = boost::static_pointer_cast<Shulker>(entity);
    const int colour = Minecraft::GetInstance()->getColourTable()->getColour(shulker->getColor()->getShulkerColor());
    if (mHasOverriddenParts)
    {
        GlStateManager::color3f((float)((colour >> 16) & 0xFF) / 255.0f, (float)((colour >> 8) & 0xFF) / 255.0f,
                                (float)(colour & 0xFF) / 255.0f);
        mBase->render(scale, useDisplayList, false);
        mLid->render(scale, useDisplayList, false);
        mHead->render(scale, useDisplayList, false);
        callSkinnedDrawlist();
        GlStateManager::color3f(1.0f, 1.0f, 1.0f);
    }
    else
    {
        GlStateManager::color3f((float)((colour >> 16) & 0xFF) / 255.0f, (float)((colour >> 8) & 0xFF) / 255.0f,
                                (float)(colour & 0xFF) / 255.0f);
        mBase->render(scale, useDisplayList, false);
        mLid->render(scale, useDisplayList, false);
        GlStateManager::color3f(1.0f, 1.0f, 1.0f);
        GlStateManager::matrixMode(2);
        GlStateManager::loadIdentity();
        GlStateManager::translatef(0.0f, 0.5f, 0.0f);
        GlStateManager::matrixMode(0);
        mBase->render(scale, useDisplayList, false);
        mLid->render(scale, useDisplayList, false);
        GlStateManager::matrixMode(2);
        GlStateManager::loadIdentity();
        GlStateManager::translatef(0.0f, 0.0f, 0.0f);
        GlStateManager::matrixMode(0);
    }
}

void ShulkerModel::renderWhenInvisible(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                                       float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                       bool useDisplayList)
{
    if (mHasOverriddenParts)
    {
        boost::shared_ptr<Shulker> shulker = boost::static_pointer_cast<Shulker>(entity);
        const int colour = Minecraft::GetInstance()->getColourTable()->getColour(
            shulker->getColor()->getShulkerColor());
        GlStateManager::color3f((float)((colour >> 16) & 0xFF) / 255.0f, (float)((colour >> 8) & 0xFF) / 255.0f,
                                (float)(colour & 0xFF) / 255.0f);
        mHeadSkin->render(scale, useDisplayList, false);
        mSkinnedParts.render();
        GlStateManager::color3f(1.0f, 1.0f, 1.0f);
    }
}
