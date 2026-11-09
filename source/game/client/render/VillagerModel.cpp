#include "game/client/render/VillagerModel.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"

VillagerModel::VillagerModel(float scale, float yOffset, int texWidth, int texHeight,
                             const ModelDefinition& modelDefinition)
    : Model()
{
    _init(scale, yOffset, texWidth, texHeight, modelDefinition);
}

VillagerModel::VillagerModel(float scale, const ModelDefinition& modelDefinition) : Model()
{
    _init(scale, 0.0f, 64, 64, modelDefinition);
}

void VillagerModel::_init(float scale, float yOffset, int texWidth, int texHeight,
                          const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mHasCustomModel = true;
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mArms = new ModelPartSkin(this, L"arms", modelDefinition, NULL, 0.0f, NULL);
        mLeg0 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, NULL);
        mNose = new ModelPartSkin(this, L"nose", modelDefinition, NULL, 0.0f, NULL);
        mHead->addChild(mNose);
        compileSkinnedDrawlist();
    }
    else
    {
        mHasCustomModel = false;
        mHead = new ModelPartSkin(this);
        mHead->setTexSize(texWidth, texHeight);
        mHead->setPos(0.0f, yOffset, 0.0f);
        mHead->texOffs(0, 0)->addBox(-4.0f, -10.0f, -4.0f, 8, 10, 8, scale);
        mNose = new ModelPartSkin(this);
        mNose->setTexSize(texWidth, texHeight);
        mNose->setPos(0.0f, yOffset - 2.0f, 0.0f);
        mNose->texOffs(24, 0)->addBox(-1.0f, -1.0f, -6.0f, 2, 4, 2, scale);
        mBody = new ModelPartSkin(this);
        mBody->setTexSize(texWidth, texHeight);
        mBody->setPos(0.0f, yOffset, 0.0f);
        mBody->texOffs(16, 20)->addBox(-4.0f, 0.0f, -3.0f, 8, 12, 6, scale);
        mBody->texOffs(0, 38)->addBox(-4.0f, 0.0f, -3.0f, 8, 18, 6, scale + 0.5f);
        mArms = new ModelPartSkin(this);
        mArms->setTexSize(texWidth, texHeight);
        mArms->setPos(0.0f, yOffset + 2.0f, 0.0f);
        mArms->texOffs(44, 22)->addBox(-8.0f, -2.0f, -2.0f, 4, 8, 4, scale);
        mArms->texOffs(44, 22)->addBox(4.0f, -2.0f, -2.0f, 4, 8, 4, scale);
        mArms->texOffs(40, 38)->addBox(-4.0f, 2.0f, -2.0f, 8, 4, 4, scale);
        mLeg0 = new ModelPartSkin(this, 0, 22, NULL);
        mLeg0->setTexSize(texWidth, texHeight);
        mLeg0->setPos(-2.0f, yOffset + 12.0f, 0.0f);
        mLeg0->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
        mLeg1 = new ModelPartSkin(this, 0, 22, NULL);
        mLeg1->setTexSize(texWidth, texHeight);
        mLeg1->mMirror = true;
        mLeg1->setPos(2.0f, yOffset + 12.0f, 0.0f);
        mLeg1->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
        mHead->addChild(mNose);
        compileSkinnedDrawlist();
    }
}

void VillagerModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                           float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());
    if (mYoung)
    {
        GlStateManager::pushMatrix();
        GlStateManager::scalef(1.5f, 1.5f, 1.5f);
        mHead->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
    }
    else
    {
        mHead->render(scale, useDisplayList, false);
    }
    mBody->render(scale, useDisplayList, false);
    mLeg0->render(scale, useDisplayList, false);
    mLeg1->render(scale, useDisplayList, false);
    mArms->render(scale, useDisplayList, false);
    callSkinnedDrawlist();
}

void VillagerModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                              float headPitch, float scale, boost::shared_ptr<Entity> entity,
                              _SkinAdjustments skinAdjustments)
{
    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
    mArms->mY = 3.0f;
    mArms->mZ = -1.0f;
    mArms->mXRot = -0.75f;
    mLeg0->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount * 0.5f;
    mLeg1->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount * 0.5f;
    mLeg0->mYRot = 0.0f;
    mLeg1->mYRot = 0.0f;
}
