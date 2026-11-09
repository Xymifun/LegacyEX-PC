#include "game/client/render/QuadrupedModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"

QuadrupedModel::QuadrupedModel(int legHeight, float scale, const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mModelOverridden = true;
        mYHeadOffs = -4.0f;
        mZHeadOffs = 4.0f;

        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, scale, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, scale, NULL);
        mLeg0 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, scale, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, scale, NULL);
        mLeg2 = new ModelPartSkin(this, L"leg2", modelDefinition, NULL, scale, NULL);
        mLeg3 = new ModelPartSkin(this, L"leg3", modelDefinition, NULL, scale, NULL);
        mHasOverriddenParts = true;
    }
    else
    {
        mYHeadOffs = 8.0f;
        mModelOverridden = false;
        mZHeadOffs = 4.0f;

        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-4.0f, -4.0f, -8.0f, 8, 8, 8, scale);
        mHead->setPos(0.0f, (float)(18 - legHeight), -6.0f);

        mBody = new ModelPartSkin(this, 28, 8, NULL);
        mBody->addBox(-5.0f, -10.0f, -7.0f, 10, 16, 8, scale);
        mBody->setPos(0.0f, (float)(17 - legHeight), 2.0f);

        mLeg0 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg0->addBox(-2.0f, 0.0f, -2.0f, 4, legHeight, 4, scale);
        mLeg0->setPos(-3.0f, (float)(24 - legHeight), 7.0f);

        mLeg1 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg1->addBox(-2.0f, 0.0f, -2.0f, 4, legHeight, 4, scale);
        mLeg1->setPos(3.0f, (float)(24 - legHeight), 7.0f);

        mLeg2 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg2->addBox(-2.0f, 0.0f, -2.0f, 4, legHeight, 4, scale);
        mLeg2->setPos(-3.0f, (float)(24 - legHeight), -5.0f);

        mLeg3 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg3->addBox(-2.0f, 0.0f, -2.0f, 4, legHeight, 4, scale);
        mLeg3->setPos(3.0f, (float)(24 - legHeight), -5.0f);
    }

    compileSkinnedDrawlist();
}

void QuadrupedModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                            float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    if (mModelOverridden)
    {
        if (mYoung)
        {
            float ss = 2.0f;
            GlStateManager::pushMatrix();
            GlStateManager::translatef(0.0f, (mYHeadOffs + 12.0f) * scale, mZHeadOffs * scale);
            mHead->render(scale, useDisplayList, false);
            GlStateManager::popMatrix();
            GlStateManager::pushMatrix();
            GlStateManager::scalef(1.0f / ss, 1.0f / ss, 1.0f / ss);
            GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
            mBody->render(scale, useDisplayList, false);
            mLeg0->render(scale, useDisplayList, false);
            mLeg1->render(scale, useDisplayList, false);
            mLeg2->render(scale, useDisplayList, false);
            mLeg3->render(scale, useDisplayList, false);
        }
        else
        {
            mHead->render(scale, useDisplayList, false);
            mBody->render(scale, useDisplayList, false);
            mLeg0->render(scale, useDisplayList, false);
            mLeg1->render(scale, useDisplayList, false);
            mLeg2->render(scale, useDisplayList, false);
            mLeg3->render(scale, useDisplayList, false);
        }
        if (mYoung)
        {
            GlStateManager::popMatrix();
            callSkinnedDrawlist();
            return;
        }
    }
    else
    {
        if (mYoung)
        {
            float ss = 2.0f;
            GlStateManager::pushMatrix();
            GlStateManager::translatef(0.0f, mYHeadOffs * scale, mZHeadOffs * scale);
            mHead->render(scale, useDisplayList, false);
            GlStateManager::popMatrix();
            GlStateManager::pushMatrix();
            GlStateManager::scalef(1.0f / ss, 1.0f / ss, 1.0f / ss);
            GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
            mBody->render(scale, useDisplayList, false);
            mLeg0->render(scale, useDisplayList, false);
            mLeg1->render(scale, useDisplayList, false);
            mLeg2->render(scale, useDisplayList, false);
            mLeg3->render(scale, useDisplayList, false);
            GlStateManager::popMatrix();
            callSkinnedDrawlist();
            return;
        }
        mHead->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
    }
    callSkinnedDrawlist();
}

void QuadrupedModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                               float headPitch, float scale, boost::shared_ptr<Entity> entity,
                               _SkinAdjustments skinAdjustments)
{
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mBody->mXRot = 90.0f * Mth::DEG_TO_RAD;

    mLeg0->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount;
    mLeg1->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount;
    mLeg2->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount;
    mLeg3->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount;
}

void QuadrupedModel::render(QuadrupedModel* model, float scale, bool useDisplayList)
{
    mHead->mYRot = model->mHead->mYRot;
    mHead->mXRot = model->mHead->mXRot;
    mHead->mY = model->mHead->mY;
    mHead->mX = model->mHead->mX;
    mBody->mYRot = model->mBody->mYRot;
    mBody->mXRot = model->mBody->mXRot;
    mLeg0->mXRot = model->mLeg0->mXRot;
    mLeg1->mXRot = model->mLeg1->mXRot;
    mLeg2->mXRot = model->mLeg2->mXRot;
    mLeg3->mXRot = model->mLeg3->mXRot;

    if (mYoung)
    {
        float ss = 2.0f;
        GlStateManager::pushMatrix();
        GlStateManager::translatef(0.0f, 8.0f * scale, 4.0f * scale);
        mHead->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
        GlStateManager::pushMatrix();
        GlStateManager::scalef(1.0f / ss, 1.0f / ss, 1.0f / ss);
        GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
    }
    else
    {
        mHead->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
    }
    callSkinnedDrawlist();
}
