#include "game/client/render/PolarBearModel.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/PolarBear.hpp"
#include "game/entity/mob/Entity.hpp"

PolarBearModel::PolarBearModel(const ModelDefinition& modelDefinition) : QuadrupedModel(12, 0.0f, modelDefinition)
{
    if (!modelDefinition.DoesOverrideModel())
    {
        mTexWidth = 128;
        mTexHeight = 64;

        delete mHead;
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-3.5f, -3.0f, -3.0f, 7, 7, 7, 0.0f);
        mHead->setPos(0.0f, 10.0f, -16.0f);
        mHead->texOffs(0, 44)->addBox(-2.5f, 1.0f, -6.0f, 5, 3, 3, 0.0f);
        mHead->texOffs(26, 0)->addBox(-4.5f, -4.0f, -1.0f, 2, 2, 1, 0.0f);
        mHead->addBoxMapTex(2.5f, -4.0f, -1.0f, 2, 2, 1, 26, 0, true, 0.0f);

        delete mBody;
        mBody = new ModelPartSkin(this);
        mBody->texOffs(0, 19)->addBox(-5.0f, -13.0f, -7.0f, 14, 14, 11, 0.0f);
        mBody->texOffs(39, 0)->addBox(-4.0f, -25.0f, -7.0f, 12, 12, 10, 0.0f);
        mBody->setPos(-2.0f, 9.0f, 12.0f);

        delete mLeg0;
        mLeg0 = new ModelPartSkin(this, 50, 22, NULL);
        mLeg0->addBox(-2.0f, 0.0f, -2.0f, 4, 10, 8, 0.0f);
        mLeg0->setPos(-3.5f, 14.0f, 6.0f);

        delete mLeg1;
        mLeg1 = new ModelPartSkin(this, 50, 22, NULL);
        mLeg1->addBox(-2.0f, 0.0f, -2.0f, 4, 10, 8, 0.0f);
        mLeg1->setPos(3.5f, 14.0f, 6.0f);

        delete mLeg2;
        mLeg2 = new ModelPartSkin(this, 50, 40, NULL);
        mLeg2->addBox(-2.0f, 0.0f, -2.0f, 4, 10, 6, 0.0f);
        mLeg2->setPos(-2.5f, 14.0f, -7.0f);

        delete mLeg3;
        mLeg3 = new ModelPartSkin(this, 50, 40, NULL);
        mLeg3->addBox(-2.0f, 0.0f, -2.0f, 4, 10, 6, 0.0f);
        mLeg3->setPos(2.5f, 14.0f, -7.0f);

        mLeg0->mX -= 1.0f;
        mLeg1->mX += 1.0f;
        mLeg2->mX -= 1.0f;
        mLeg3->mX += 1.0f;
        mLeg2->mZ -= 1.0f;
        mLeg3->mZ -= 1.0f;
        mZHeadOffs += 2.0f;
    }

    compileSkinnedDrawlist();
}

void PolarBearModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                            float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    if (mYoung)
    {
        mYHeadOffs = 16.0f;
        mZHeadOffs = 4.0f;
        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.6666667f, 0.6666667f, 0.6666667f);
        GlStateManager::translatef(0.0f, mYHeadOffs * scale, mZHeadOffs * scale);
        mHead->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();

        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
        callSkinnedDrawlist();
    }
    else
    {
        mHead->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        callSkinnedDrawlist();
    }
}

void PolarBearModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                               float headPitch, float scale, boost::shared_ptr<Entity> entity,
                               _SkinAdjustments skinAdjustments)
{
    QuadrupedModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                              _SkinAdjustments());

    float partialTicks = ageInTicks - entity->mTickCount;
    float standingScale = boost::static_pointer_cast<PolarBear>(entity)->getStandingAnimationScale(partialTicks);
    float standingScaleSquared = standingScale * standingScale;
    float standingScaleComplement = 1.0f - standingScaleSquared;

    const float standingAngle = standingScaleSquared * 3.1415927f;

    mBody->mXRot = 1.5707964f - standingAngle * 0.35f;
    mBody->mY = 9.0f * standingScaleComplement + 11.0f * standingScaleSquared;
    mLeg2->mY = 14.0f * standingScaleComplement + -6.0f * standingScaleSquared;
    mLeg2->mZ = -8.0f * standingScaleComplement + -4.0f * standingScaleSquared;
    mLeg2->mXRot -= standingAngle * 0.45f;
    mLeg3->mY = mLeg2->mY;
    mLeg3->mZ = mLeg2->mZ;
    mLeg3->mXRot -= standingAngle * 0.45f;
    mHead->mY = 10.0f * standingScaleComplement + -12.0f * standingScaleSquared;
    mHead->mZ = -16.0f * standingScaleComplement + -3.0f * standingScaleSquared;
    mHead->mXRot += standingAngle * 0.15f;
}
