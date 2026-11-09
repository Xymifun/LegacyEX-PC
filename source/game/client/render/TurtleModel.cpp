#include "game/client/render/TurtleModel.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Turtle.hpp"

TurtleModel::TurtleModel(float scale, const ModelDefinition& modelDefinition)
    : QuadrupedModel(12, scale, modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mEggBelly = new ModelPartSkin(this, L"eggBelly", modelDefinition, NULL, 0.0f, NULL);
    }
    else
    {
        mTexWidth = 128;
        mTexHeight = 64;

        delete mHead;
        mHead = new ModelPartSkin(this, 2, 0, NULL);
        mHead->addBox(-3.0f, -1.0f, -3.0f, 6, 5, 6, 0.0f);
        mHead->setPos(0.0f, 19.0f, -10.0f);

        mEggBelly = new ModelPartSkin(this, 69, 33, NULL);
        mEggBelly->addBox(-4.5f, 3.0f, -12.0f, 9, 18, 1, 0.0f);
        mEggBelly->setPos(0.0f, 13.0f, -10.0f);

        delete mBody;
        mBody = new ModelPartSkin(this);
        mBody->texOffs(6, 37)->addBox(-9.5f, 3.0f, -10.0f, 19, 20, 6, 0.0f);
        mBody->texOffs(30, 1)->addBox(-5.5f, 3.0f, -13.0f, 11, 18, 3, 0.0f);
        mBody->setPos(0.0f, 11.0f, -10.0f);

        delete mLeg0;
        mLeg0 = new ModelPartSkin(this, 0, 23, NULL);
        mLeg0->addBox(-2.0f, 0.0f, 0.0f, 4, 1, 10, 0.0f);
        mLeg0->setPos(-3.5f, 22.0f, 11.0f);

        delete mLeg1;
        mLeg1 = new ModelPartSkin(this, 0, 12, NULL);
        mLeg1->addBox(-2.0f, 0.0f, 0.0f, 4, 1, 10, 0.0f);
        mLeg1->setPos(3.5f, 22.0f, 11.0f);

        delete mLeg2;
        mLeg2 = new ModelPartSkin(this, 26, 30, NULL);
        mLeg2->addBox(-13.0f, 0.0f, -2.0f, 13, 1, 5, 0.0f);
        mLeg2->setPos(-5.0f, 21.0f, -4.0f);

        delete mLeg3;
        mLeg3 = new ModelPartSkin(this, 26, 24, NULL);
        mLeg3->addBox(0.0f, 0.0f, -2.0f, 13, 1, 5, 0.0f);
        mLeg3->setPos(5.0f, 21.0f, -4.0f);
    }

    compileSkinnedDrawlist();
}

void TurtleModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                         float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    boost::shared_ptr<Turtle> turtle = boost::static_pointer_cast<Turtle>(entity);
    const bool show = !mYoung && turtle->hasEgg();
    const bool young = mYoung;
    mEggBelly->mShow = show;

    GlStateManager::pushMatrix();
    if (young)
    {
        GlStateManager::scalef(0.16666667f, 0.16666667f, 0.16666667f);
        GlStateManager::translatef(0.0f, 120.0f * scale, 0.0f);
        mHead->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        mEggBelly->render(scale, useDisplayList, false);
    }
    else
    {
        if (turtle->hasEgg())
        {
            GlStateManager::translatef(0.0f, -0.08f, 0.0f);
        }

        mHead->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        GlStateManager::pushMatrix();
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        mEggBelly->render(scale, useDisplayList, false);
    }
    GlStateManager::popMatrix();
    callSkinnedDrawlist();
}

void TurtleModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                            float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    QuadrupedModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                              _SkinAdjustments());

    boost::shared_ptr<Turtle> turtle = boost::static_pointer_cast<Turtle>(entity);
    const float halfPi = 1.5707964f;
    const float swing = limbSwing * 0.6662f * 0.6f;
    mLeg0->mXRot = Mth::clamp(Mth::cos(swing) * 0.5f * limbSwingAmount, -halfPi, halfPi);
    mLeg0->mYRot = 0.0f;
    mLeg1->mXRot = Mth::clamp(Mth::cos(swing + 3.1415927f) * 0.5f * limbSwingAmount, -halfPi, halfPi);
    mLeg1->mYRot = 0.0f;
    mLeg2->mYRot = 0.0f;
    mLeg2->mXRot = 0.0f;
    mLeg2->mZRot = Mth::clamp(Mth::cos(swing + 3.1415927f) * 0.5f * limbSwingAmount, -halfPi, halfPi);
    mLeg3->mYRot = 0.0f;
    mLeg3->mXRot = 0.0f;
    mLeg3->mZRot = Mth::clamp(Mth::cos(swing) * 0.5f * limbSwingAmount, -halfPi, halfPi);
    mEggBelly->mXRot = 90.0f * Mth::DEG_TO_RAD;

    if (!turtle->isInWater() && turtle->mOnGround)
    {
        float frontSwing = 3.0f;
        float backSwing = 8.0f;
        float speed = 5.0f;
        float pace = 1.0f;
        if (turtle->isVehicle())
        {
            speed = 2.0f;
            frontSwing = 0.5f;
            backSwing = 2.0f;
            pace = 0.39972f;
        }
        const float walk = limbSwing * pace * speed;
        if (!turtle->isLayingEgg())
        {
            mLeg2->mYRot = Mth::clamp(Mth::cos(walk + 3.1415927f) * backSwing * limbSwingAmount, -halfPi, halfPi);
            mLeg3->mYRot = Mth::clamp(Mth::cos(walk) * backSwing * limbSwingAmount, -halfPi, halfPi);
        }
        else
        {
            mLeg2->mYRot = Mth::cos(ageInTicks + 3.1415927f) * halfPi;
            mLeg3->mYRot = Mth::cos(ageInTicks) * halfPi;
        }
        mLeg0->mXRot = 0.0f;
        mLeg0->mYRot = Mth::clamp(Mth::cos(walk + 3.1415927f) * frontSwing * limbSwingAmount, -halfPi, halfPi);
        mLeg1->mXRot = 0.0f;
        mLeg1->mYRot = Mth::clamp(Mth::cos(walk) * frontSwing * limbSwingAmount, -halfPi, halfPi);
        mLeg2->mZRot = 0.0f;
        mLeg3->mZRot = 0.0f;
    }
}
