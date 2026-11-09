#include "game/client/render/ParrotModel.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Parrot.hpp"
#include "game/entity/mob/Entity.hpp"

ParrotModel::ParrotModel(const ModelDefinition& modelDefinition) : Model(), mAnimationState(1)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mTail = new ModelPartSkin(this, L"tail", modelDefinition, NULL, 0.0f, NULL);
        mWing0 = new ModelPartSkin(this, L"wing0", modelDefinition, NULL, 0.0f, NULL);
        mWing1 = new ModelPartSkin(this, L"wing1", modelDefinition, NULL, 0.0f, NULL);
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mHead2 = new ModelPartSkin(this, L"head2", modelDefinition, mHead, 0.0f, NULL);
        mBeak1 = new ModelPartSkin(this, L"beak1", modelDefinition, mHead, 0.0f, NULL);
        mBeak2 = new ModelPartSkin(this, L"beak2", modelDefinition, mHead, 0.0f, NULL);
        mFeather = new ModelPartSkin(this, L"feather", modelDefinition, mHead, 0.0f, NULL);
        mLeg0 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, NULL);
        mHeadDefaultX = mHead->mX;
        mHeadDefaultY = mHead->mY;
        mHeadDefaultZRot = mHead->mZRot;
        mFeatherDefaultXRot = mFeather->mXRot;
        mBodyDefaultX = mBody->mX;
        mBodyDefaultY = mBody->mY;
        mBodyDefaultXRot = mBody->mXRot;
        mWing0DefaultX = mWing0->mX;
        mWing0DefaultY = mWing0->mY;
        mWing0DefaultXRot = mWing0->mXRot;
        mWing0DefaultYRot = mWing0->mYRot;
        mWing1DefaultX = mWing1->mX;
        mWing1DefaultY = mWing1->mY;
        mWing1DefaultXRot = mWing1->mXRot;
        mWing1DefaultYRot = mWing1->mYRot;
        mLeg0DefaultY = mLeg0->mY;
        mLeg0DefaultXRot = mLeg0->mXRot;
        mLeg0DefaultZRot = mLeg0->mZRot;
        mLeg1DefaultY = mLeg1->mY;
        mLeg1DefaultXRot = mLeg1->mXRot;
        mLeg1DefaultZRot = mLeg1->mZRot;
        mTailDefaultX = mTail->mX;
        mTailDefaultY = mTail->mY;
    }
    else
    {
        mTexWidth = 32;
        mTexHeight = 32;
        mBody = new ModelPartSkin(this, 2, 8, NULL);
        mBody->addBox(-1.5f, 0.0f, -1.5f, 3, 6, 3);
        mBody->setPos(0.0f, 16.5f, -3.0f);
        mTail = new ModelPartSkin(this, 22, 1, NULL);
        mTail->addBox(-1.5f, -1.0f, -1.0f, 3, 4, 1);
        mTail->setPos(0.0f, 21.07f, 1.16f);
        mWing0 = new ModelPartSkin(this, 19, 8, NULL);
        mWing0->addBox(-0.5f, 0.0f, -1.5f, 1, 5, 3);
        mWing0->setPos(1.5f, 16.94f, -2.76f);
        mWing1 = new ModelPartSkin(this, 19, 8, NULL);
        mWing1->addBox(-0.5f, 0.0f, -1.5f, 1, 5, 3);
        mWing1->setPos(-1.5f, 16.94f, -2.76f);
        mHead = new ModelPartSkin(this, 2, 2, NULL);
        mHead->addBox(-1.0f, -1.5f, -1.0f, 2, 3, 2);
        mHead->setPos(0.0f, 15.69f, -2.76f);
        mHead2 = new ModelPartSkin(this, 10, 0, NULL);
        mHead2->addBox(-1.0f, -0.5f, -2.0f, 2, 1, 4);
        mHead2->setPos(0.0f, -2.0f, -1.0f);
        mHead->addChild(mHead2);
        mBeak1 = new ModelPartSkin(this, 11, 7, NULL);
        mBeak1->addBox(-0.5f, -1.0f, -0.5f, 1, 2, 1);
        mBeak1->setPos(0.0f, -0.5f, -1.5f);
        mHead->addChild(mBeak1);
        mBeak2 = new ModelPartSkin(this, 16, 7, NULL);
        mBeak2->addBox(-0.5f, 0.0f, -0.5f, 1, 2, 1);
        mBeak2->setPos(0.0f, -1.75f, -2.45f);
        mHead->addChild(mBeak2);
        mFeather = new ModelPartSkin(this, 2, 18, NULL);
        mFeather->addBox(0.0f, -4.0f, -2.0f, 0, 5, 4);
        mFeather->setPos(0.0f, -2.15f, 0.15f);
        mHead->addChild(mFeather);
        mLeg0 = new ModelPartSkin(this, 14, 18, NULL);
        mLeg0->addBox(-0.5f, 0.0f, -0.5f, 1, 2, 1);
        mLeg0->setPos(1.0f, 22.0f, -1.05f);
        mLeg1 = new ModelPartSkin(this, 14, 18, NULL);
        mLeg1->addBox(-0.5f, 0.0f, -0.5f, 1, 2, 1);
        mLeg1->setPos(-1.0f, 22.0f, -1.05f);

        mHeadDefaultZRot = 0.0f;
        mBodyDefaultX = 0.0f;
        mFeatherDefaultXRot = -0.2214f;
        mHeadDefaultX = 0.0f;
        mWing1DefaultX = -1.5f;
        mWing0DefaultY = 16.94f;
        mBodyDefaultY = 16.5f;
        mLeg1DefaultY = 22.0f;
        mTailDefaultY = 21.07f;
        mBodyDefaultXRot = 0.4927f;
        mHeadDefaultY = 15.69f;
        mLeg1DefaultXRot = -0.0299f;
        mWing1DefaultYRot = 0.0f;
        mWing0DefaultYRot = 0.0f;
        mWing0DefaultXRot = 0.6981f;
        mLeg0DefaultXRot = -0.0299f;
        mTailDefaultX = 0.0f;
        mLeg0DefaultZRot = 0.0f;
        mWing1DefaultXRot = 0.6981f;
        mLeg1DefaultZRot = 0.0f;
        mLeg0DefaultY = 22.0f;
        mWing0DefaultX = 1.5f;
        mWing1DefaultY = 16.94f;
    }
    compileSkinnedDrawlist();
}

void ParrotModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                         float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    mBody->render(scale, useDisplayList, false);
    mWing0->render(scale, useDisplayList, false);
    mWing1->render(scale, useDisplayList, false);
    mTail->render(scale, useDisplayList, false);
    mHead->render(scale, useDisplayList, false);
    mLeg0->render(scale, useDisplayList, false);
    mLeg1->render(scale, useDisplayList, false);
    callSkinnedDrawlist();
}

void ParrotModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                            float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHead->mZRot = mHeadDefaultZRot;
    mHead->mX = mHeadDefaultX;
    mBody->mX = mBodyDefaultX;
    mTail->mX = mTailDefaultX;
    mWing0->mX = mWing0DefaultX;
    mWing1->mX = mWing1DefaultX;
    const float bob = ageInTicks * 0.3f;

    switch (mAnimationState)
    {
    case 0:
    {
        const float swing = limbSwing * 0.6662f;
        mLeg0->mXRot += Mth::cos(swing) * 1.4f * limbSwingAmount;
        mLeg1->mXRot += Mth::cos(swing + 3.1415927f) * 1.4f * limbSwingAmount;
        mHead->mY = mHeadDefaultY + bob;
        const float tailRot = Mth::cos(swing) * 0.3f * limbSwingAmount + 1.015f;
        mTail->mXRot = tailRot;
        mTail->mY = mTailDefaultY + bob;
        mBody->mY = mBodyDefaultY + bob;
        mWing0->mZRot = -0.0873f - ageInTicks;
        mWing0->mY = mWing0DefaultY + bob;
        mWing1->mZRot = 0.0873f + ageInTicks;
        mWing1->mY = mWing1DefaultY + bob;
        mLeg0->mY = mLeg0DefaultY + bob;
        mLeg1->mY = mLeg1DefaultY + bob;
        break;
    }
    case 2:
        break;
    case 3:
    {
        const float danceX = Mth::cos(static_cast<float>(entity->mTickCount));
        const float danceY = Mth::sin(static_cast<float>(entity->mTickCount));
        mHead->mX = danceX;
        mHead->mY = mHeadDefaultY + danceY;
        mHead->mXRot = 0.0f;
        mHead->mYRot = 0.0f;
        mHead->mZRot = Mth::sin(static_cast<float>(entity->mTickCount)) * 0.4f;
        mBody->mX = danceX;
        mBody->mY = mBodyDefaultY + danceY;
        mWing0->mZRot = -0.0873f - ageInTicks;
        mWing0->mX = danceX + 1.5f;
        mWing0->mY = mWing0DefaultY + danceY;
        mWing1->mZRot = 0.0873f + ageInTicks;
        mWing1->mX = -1.5f + danceX;
        mWing1->mY = mWing1DefaultY + danceY;
        mTail->mX = danceX;
        mTail->mY = mTailDefaultY + danceY;
        break;
    }
    default:
    {
        mHead->mY = mHeadDefaultY + bob;
        const float tailRot = Mth::cos(limbSwing * 0.6662f) * 0.3f * limbSwingAmount + 1.015f;
        mTail->mXRot = tailRot;
        mTail->mY = mTailDefaultY + bob;
        mBody->mY = mBodyDefaultY + bob;
        mWing0->mZRot = -0.0873f - ageInTicks;
        mWing0->mY = mWing0DefaultY + bob;
        mWing1->mZRot = 0.0873f + ageInTicks;
        mWing1->mY = mWing1DefaultY + bob;
        mLeg0->mY = mLeg0DefaultY + bob;
        mLeg1->mY = mLeg1DefaultY + bob;
        break;
    }
    }
}

void ParrotModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                  float partialTicks)
{
    mFeather->mXRot = mFeatherDefaultXRot;
    mBody->mXRot = mBodyDefaultXRot;
    mWing0->mXRot = mWing0DefaultXRot;
    mWing0->mYRot = mWing0DefaultYRot;
    mWing1->mXRot = mWing1DefaultXRot;
    mWing1->mYRot = mWing1DefaultYRot;
    mLeg0->mXRot = mLeg0DefaultXRot;
    mLeg1->mXRot = mLeg1DefaultXRot;
    mLeg0->mY = mLeg0DefaultY;
    mLeg1->mY = mLeg1DefaultY;

    const bool isParrot = eTYPE_DERIVED_FROM(EntityType::PARROT, entity->GetType());
    if (isParrot)
    {
        boost::shared_ptr<Parrot> parrot = boost::static_pointer_cast<Parrot>(entity);
        if (parrot->isPartyParrot())
        {
            mAnimationState = 3;
            mLeg0->mZRot = -(Mth::DEG_TO_RAD * 20.0f);
            mLeg1->mZRot = Mth::DEG_TO_RAD * 20.0f;
            return;
        }
        else if (parrot->isSitting())
        {
            mHead->mY = mHeadDefaultY + 1.9f;
            mTail->mXRot = Mth::DEG_TO_RAD * 30.0f + 1.0153f;
            mTail->mY = mTailDefaultY + 1.9f;
            mBody->mY = mBodyDefaultY + 1.9f;
            mWing0->mZRot = -0.0873f;
            mWing0->mY = mWing0DefaultY + 1.9f;
            mWing1->mZRot = 0.0873f;
            mWing1->mY = mWing1DefaultY + 1.9f;
            mLeg0->mY += 1.9f;
            mLeg1->mY += 1.9f;
            mLeg0->mXRot += Mth::DEG_TO_RAD * 90.0f;
            mLeg1->mXRot += Mth::DEG_TO_RAD * 90.0f;
            mLeg0->mZRot = mLeg0DefaultZRot;
            mAnimationState = 2;
            mLeg1->mZRot = mLeg1DefaultZRot;
        }
        else if (parrot->isFlying())
        {
            mLeg0->mZRot = mLeg0DefaultZRot;
            mAnimationState = 0;
            mLeg1->mZRot = mLeg1DefaultZRot;
        }
        else
        {
            const float legAngle = Mth::DEG_TO_RAD * 20.0f;
            mLeg0->mXRot += legAngle;
            mAnimationState = 1;
            mLeg1->mXRot += legAngle;
            mLeg0->mZRot = mLeg0DefaultZRot;
            mLeg1->mZRot = mLeg1DefaultZRot;
        }
    }
    else if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
        {
            const float legAngle = Mth::DEG_TO_RAD * 40.0f;
            mLeg0->mXRot += legAngle;
            mAnimationState = 0;
            mLeg1->mXRot += legAngle;
        }
    }
}
