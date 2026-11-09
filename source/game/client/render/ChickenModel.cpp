#include "game/client/render/ChickenModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"

ChickenModel::ChickenModel(const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mLeg0 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, NULL);
        mWing0 = new ModelPartSkin(this, L"wing0", modelDefinition, NULL, 0.0f, NULL);
        mWing1 = new ModelPartSkin(this, L"wing1", modelDefinition, NULL, 0.0f, NULL);
        mBeak = new ModelPartSkin(this, L"beak", modelDefinition, NULL, 0.0f, NULL);
        mComb = new ModelPartSkin(this, L"comb", modelDefinition, NULL, 0.0f, NULL);

        compileSkinnedDrawlist();
    }
    else
    {
        int yo = 16;
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-2.0f, -6.0f, -2.0f, 4, 6, 3, 0.0f);
        mHead->setPos(0.0f, -1.0f + yo, -4.0f);

        mBeak = new ModelPartSkin(this, 14, 0, NULL);
        mBeak->addBox(-2.0f, -4.0f, -4.0f, 4, 2, 2, 0.0f);
        mBeak->setPos(0.0f, -1.0f + yo, -4.0f);

        mComb = new ModelPartSkin(this, 14, 4, NULL);
        mComb->addBox(-1.0f, -2.0f, -3.0f, 2, 2, 2, 0.0f);
        mComb->setPos(0.0f, -1.0f + yo, -4.0f);

        mBody = new ModelPartSkin(this, 0, 9, NULL);
        mBody->addBox(-3.0f, -4.0f, -3.0f, 6, 8, 6, 0.0f);
        mBody->setPos(0.0f, yo, 0.0f);

        mLeg0 = new ModelPartSkin(this, 26, 0, NULL);
        mLeg0->addBox(-1.0f, 0.0f, -3.0f, 3, 5, 3);
        mLeg0->setPos(-2.0f, 3.0f + yo, 1.0f);

        mLeg1 = new ModelPartSkin(this, 26, 0, NULL);
        mLeg1->addBox(-1.0f, 0.0f, -3.0f, 3, 5, 3);
        mLeg1->setPos(1.0f, 3.0f + yo, 1.0f);

        mWing0 = new ModelPartSkin(this, 24, 13, NULL);
        mWing0->addBox(0.0f, 0.0f, -3.0f, 1, 4, 6);
        mWing0->setPos(-4.0f, -3.0f + yo, 0.0f);

        mWing1 = new ModelPartSkin(this, 24, 13, NULL);
        mWing1->addBox(-1.0f, 0.0f, -3.0f, 1, 4, 6);
        mWing1->setPos(4.0f, -3.0f + yo, 0.0f);

        compileSkinnedDrawlist();
    }
}

void ChickenModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                          float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    if (mYoung)
    {
        float ss = 2.0f;
        GlStateManager::pushMatrix();
        GlStateManager::translatef(0.0f, 5.0f * scale, 2.0f * scale);
        mHead->render(scale, useDisplayList, false);
        mBeak->render(scale, useDisplayList, false);
        mComb->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();

        GlStateManager::pushMatrix();
        GlStateManager::scalef(1.0f / ss, 1.0f / ss, 1.0f / ss);
        GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mWing0->render(scale, useDisplayList, false);
        mWing1->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
    }
    else
    {
        mHead->render(scale, useDisplayList, false);
        mBeak->render(scale, useDisplayList, false);
        mComb->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mWing0->render(scale, useDisplayList, false);
        mWing1->render(scale, useDisplayList, false);
    }

    callSkinnedDrawlist();
}

void ChickenModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                             float headPitch, float scale, boost::shared_ptr<Entity> entity,
                             _SkinAdjustments skinAdjustments)
{
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mBeak->mXRot = mHead->mXRot;
    mBeak->mYRot = mHead->mYRot;
    mComb->mXRot = mHead->mXRot;
    mComb->mYRot = mHead->mYRot;
    mBody->mXRot = 90.0f * Mth::DEG_TO_RAD;

    mLeg0->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount;
    mLeg1->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount;
    mWing0->mZRot = ageInTicks;
    mWing1->mZRot = -ageInTicks;
}
