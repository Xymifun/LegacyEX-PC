#include "game/client/render/AbstractHorseModel.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Math.hpp"

static const float EATING_HEAD_X_ROT = 110.0f * Mth::DEG_TO_RAD;

AbstractHorseModel::AbstractHorseModel(bool horse, const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mBody = new ModelPartSkin(this, L"Body", modelDefinition, NULL, 0.0f, NULL);
        mNeck = new ModelPartSkin(this, L"Neck", modelDefinition, NULL, 0.0f, NULL);

        mHead = new ModelPartSkin(this, L"Head", modelDefinition, mNeck, 0.0f, NULL);
        mHead->mXRot = 0.0f;
        mHead->mYRot = 0.0f;
        mHead->mZRot = 0.0f;

        mUMouth = new ModelPartSkin(this, L"UMouth", modelDefinition, mHead, 0.0f, NULL);
        mMane = new ModelPartSkin(this, L"Mane", modelDefinition, NULL, 0.0f, NULL);

        mEar1 = new ModelPartSkin(this, L"Ear1", modelDefinition, mHead, 0.0f, NULL);
        mEar1->mXRot = 0.0f;
        mEar1->mYRot = 0.0f;
        mEar1->mZRot = 0.08726647f;

        mEar2 = new ModelPartSkin(this, L"Ear2", modelDefinition, mHead, 0.0f, NULL);
        mEar2->mXRot = 0.0f;
        mEar2->mYRot = 0.0f;
        mEar2->mZRot = -0.08726647f;

        mMuleEarL = new ModelPartSkin(this, L"MuleEarL", modelDefinition, mHead, 0.0f, NULL);
        mMuleEarL->mXRot = 0.0f;
        mMuleEarL->mYRot = 0.0f;
        mMuleEarL->mZRot = 0.2617994f;

        mMuleEarR = new ModelPartSkin(this, L"MuleEarR", modelDefinition, mHead, 0.0f, NULL);
        mMuleEarR->mXRot = 0.0f;
        mMuleEarR->mYRot = 0.0f;
        mMuleEarR->mZRot = -0.2617994f;

        mLeg1A = new ModelPartSkin(this, L"Leg1A", modelDefinition, NULL, 0.0f, NULL);
        mLeg2A = new ModelPartSkin(this, L"Leg2A", modelDefinition, NULL, 0.0f, NULL);
        mLeg3A = new ModelPartSkin(this, L"Leg3A", modelDefinition, NULL, 0.0f, NULL);
        mLeg4A = new ModelPartSkin(this, L"Leg4A", modelDefinition, NULL, 0.0f, NULL);

        mTailA = new ModelPartSkin(this, L"TailA", modelDefinition, mBody, 0.0f, NULL);

        mBag1 = new ModelPartSkin(this, L"Bag1", modelDefinition, NULL, 0.0f, NULL);
        mBag2 = new ModelPartSkin(this, L"Bag2", modelDefinition, NULL, 0.0f, NULL);

        mSaddle = new ModelPartSkin(this, L"Saddle", modelDefinition, mBody, 0.0f, NULL);
        mSaddleMouthL = new ModelPartSkin(this, L"SaddleMouthL", modelDefinition, mHead, 0.0f, NULL);
        mSaddleMouthR = new ModelPartSkin(this, L"SaddleMouthR", modelDefinition, mHead, 0.0f, NULL);
        mSaddleMouthLine = new ModelPartSkin(this, L"SaddleMouthLine", modelDefinition, NULL, 0.0f, NULL);
        mSaddleMouthLineR = new ModelPartSkin(this, L"SaddleMouthLineR", modelDefinition, NULL, 0.0f, NULL);

        mSaddleMouthL->mXRot = 0.0f;
        mSaddleMouthL->mYRot = 0.0f;
        mSaddleMouthL->mZRot = 0.0f;
        mSaddleMouthR->mXRot = 0.0f;
        mSaddleMouthR->mYRot = 0.0f;
        mSaddleMouthR->mZRot = 0.0f;
        mSaddleMouthLine->mXRot = 0.0f;
        mSaddleMouthLine->mYRot = 0.0f;
        mSaddleMouthLine->mZRot = 0.0f;
        mSaddleMouthLineR->mXRot = 0.0f;
        mSaddleMouthLineR->mYRot = 0.0f;
        mSaddleMouthLineR->mZRot = 0.0f;

        mHeadSaddle = new ModelPartSkin(this, L"HeadSaddle", modelDefinition, mHead, 0.0f, NULL);
        mHeadSaddle->mXRot = 0.0f;
        mHeadSaddle->mYRot = 0.0f;
        mHeadSaddle->mZRot = 0.0f;

        mLeg3ADefaultY = mLeg3A->mY;
        mLeg3ADefaultZ = mLeg3A->mZ;
        mBag2DefaultY = mBag2->mY;
        mBag2DefaultZ = mBag2->mZ;

        compileSkinnedDrawlist();
    }
    else
    {
        mTexWidth = 64;
        mTexHeight = 64;

        mBody = new ModelPartSkin(this, 0, 32, NULL);
        mBody->setPos(0.0f, 11.0f, 9.0f);
        mBody->addBox(-5.0f, -8.0f, -20.0f, 10, 10, 22);

        mNeck = new ModelPartSkin(this, 0, 35, NULL);
        mNeck->setPos(0.0f, 7.0f, -8.0f);
        mNeck->addBox(-2.0f, -11.0f, -3.0f, 4, 12, 7);
        mNeck->mXRot = 0.5235988f;

        mHead = new ModelPartSkin(this, 0, 13, NULL);
        mHead->setPos(0.0f, -11.0f, -3.0f);
        mHead->addBox(-3.0f, -5.0f, 0.0f, 6, 5, 7);

        mMane = new ModelPartSkin(this, 56, 36, NULL);
        mMane->setPos(0.0f, 7.0f, -8.0f);
        mMane->addBox(-1.0f, -16.0f, 4.0f, 2, 16, 2);
        mMane->mXRot = 0.5235988f;

        mUMouth = new ModelPartSkin(this, 0, 25, NULL);
        mUMouth->setPos(0.0f, 0.0f, 0.0f);
        mUMouth->addBox(-2.0f, -5.0f, -5.0f, 4, 5, 5);
        mUMouth->mXRot = 0.5235988f;

        mEar1 = new ModelPartSkin(this, 19, 16, NULL);
        mEar1->setPos(0.0f, 11.0f, 3.0f);
        mEar1->addBox(-0.5f, -18.0f, 2.9f, 2, 3, 1);
        mEar1->mZRot = 0.08726647f;

        mEar2 = new ModelPartSkin(this, 19, 16, NULL);
        mEar2->setPos(0.0f, 11.0f, 3.0f);
        mEar2->addBox(-1.5f, -18.0f, 2.9f, 2, 3, 1);
        mEar2->mZRot = -0.08726647f;

        mMuleEarL = new ModelPartSkin(this, 0, 12, NULL);
        mMuleEarL->setPos(0.0f, 11.0f, 3.0f);
        mMuleEarL->addBox(-3.0f, -22.0f, 2.9f, 2, 7, 1);
        mMuleEarL->mZRot = 0.2617994f;

        mMuleEarR = new ModelPartSkin(this, 0, 12, NULL);
        mMuleEarR->setPos(0.0f, 11.0f, 3.0f);
        mMuleEarR->addBox(1.0f, -22.0f, 2.9f, 2, 7, 1);
        mMuleEarR->mZRot = -0.2617994f;

        mLeg1A = new ModelPartSkin(this, 48, 21, NULL);
        mLeg1A->mMirror = true;
        mLeg1A->setPos(3.0f, 13.0f, 9.0f);
        mLeg1A->addBox(-2.0f, 0.0f, -2.0f, 4, 11, 4);

        mLeg2A = new ModelPartSkin(this, 48, 21, NULL);
        mLeg2A->setPos(-3.0f, 13.0f, 9.0f);
        mLeg2A->addBox(-2.0f, 0.0f, -2.0f, 4, 11, 4);

        mLeg3A = new ModelPartSkin(this, 48, 21, NULL);
        mLeg3A->mMirror = true;
        mLeg3A->setPos(3.0f, 13.0f, -9.0f);
        mLeg3A->addBox(-2.0f, 0.0f, -2.0f, 4, 11, 4);

        mLeg4A = new ModelPartSkin(this, 48, 21, NULL);
        mLeg4A->setPos(-3.0f, 13.0f, -9.0f);
        mLeg4A->addBox(-2.0f, 0.0f, -2.0f, 4, 11, 4);

        mTailA = new ModelPartSkin(this, 42, 36, NULL);
        mTailA->setPos(0.0f, -7.0f, 2.0f);
        mTailA->addBox(-1.5f, 0.0f, -2.0f, 3, 14, 4);
        mTailA->mXRot = 0.5235988f;

        mSaddle = new ModelPartSkin(this, 26, 0, NULL);
        mSaddle->setPos(0.0f, -9.0f, -7.0f);
        mSaddle->addBox(-5.0f, 1.0f, -6.0f, 10, 9, 9, 0.5f);

        mSaddleMouthL = new ModelPartSkin(this, 29, 5, NULL);
        mSaddleMouthL->setPos(0.0f, 11.0f, 3.0f);
        mSaddleMouthL->addBox(2.0f, -14.0f, -6.0f, 1, 2, 2);

        mSaddleMouthR = new ModelPartSkin(this, 29, 5, NULL);
        mSaddleMouthR->setPos(0.0f, 11.0f, 3.0f);
        mSaddleMouthR->addBox(-3.0f, -14.0f, -6.0f, 1, 2, 2);

        mSaddleMouthLine = new ModelPartSkin(this, 32, 2, NULL);
        mSaddleMouthLine->setPos(0.0f, 7.0f, -8.0f);
        mSaddleMouthLine->addBox(3.1f, -10.0f, -11.5f, 0, 3, 16);
        mSaddleMouthLine->mXRot = 1.5707964f;

        mSaddleMouthLineR = new ModelPartSkin(this, 32, 2, NULL);
        mSaddleMouthLineR->setPos(0.0f, 7.0f, -8.0f);
        mSaddleMouthLineR->addBox(-3.1f, -10.0f, -11.5f, 0, 3, 16);
        mSaddleMouthLineR->mXRot = 1.5707964f;

        mHeadSaddle = new ModelPartSkin(this, 19, 0, NULL);
        mHeadSaddle->setPos(0.0f, 11.0f, 3.0f);
        mHeadSaddle->addBox(-2.0f, -16.0f, -5.0f, 4, 5, 2, 0.25f);
        mHeadSaddle->addBoxMapTex(-3.0f, -16.0f, -3.0f, 6, 5, 7, 0, 0, 0.25f);

        mBag1 = new ModelPartSkin(this, 26, 21, NULL);
        mBag1->setPos(-5.0f, 3.0f, 11.0f);
        mBag1->addBox(-9.0f, 0.0f, 0.0f, 8, 8, 3);
        mBag1->mYRot = -1.5707964f;

        mBag2 = new ModelPartSkin(this, 26, 21, NULL);
        mBag2->setPos(5.0f, 3.0f, 11.0f);
        mBag2->addBox(1.0f, 0.0f, 0.0f, 8, 8, 3);
        mBag2->mYRot = 1.5707964f;

        mNeck->addChild(mHead);
        mHead->addChild(mUMouth);
        mHead->addChild(mEar1);
        mHead->addChild(mEar2);
        mHead->addChild(mMuleEarL);
        mHead->addChild(mMuleEarR);
        mHead->addChild(mHeadSaddle);
        mBody->addChild(mSaddle);
        mBody->addChild(mTailA);
        mHead->addChild(mSaddleMouthL);
        mHead->addChild(mSaddleMouthR);

        mLeg3ADefaultY = mLeg3A->mY;
        mLeg3ADefaultZ = mLeg3A->mZ;
        mBag2DefaultY = mBag2->mY;
        mBag2DefaultZ = mBag2->mZ;

        compileSkinnedDrawlist();
    }
}

float AbstractHorseModel::rotlerp(float from, float to, float delta)
{
    float difference = to - from;

    while (difference < -180.0f)
    {
        difference += 360.0f;
    }

    while (difference >= 180.0f)
    {
        difference -= 360.0f;
    }

    return from + delta * difference;
}

void AbstractHorseModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                         float limbSwingAmount, float partialTicks)
{
    Model::prepareMobModel(entity, limbSwing, limbSwingAmount, partialTicks);

    float bodyRot = rotlerp(entity.get()->mYBodyRotO, entity.get()->mYBodyRot, partialTicks);
    float headRot = rotlerp(entity.get()->mYHeadRotO, entity.get()->mYHeadRot, partialTicks);
    float xRot = rotlerp(entity.get()->mXRotO, entity.get()->mXRot, partialTicks);

    float netHeadYaw = Math::wrapDegrees(headRot - bodyRot);
    float headPitch = xRot * Mth::DEG_TO_RAD;

    if (netHeadYaw > 20.0f)
    {
        netHeadYaw = 20.0f;
    }
    else if (netHeadYaw < -20.0f)
    {
        netHeadYaw = -20.0f;
    }

    if (limbSwingAmount > 0.2f)
    {
        headPitch += Mth::cos(limbSwing * 0.2f) * 0.2f * limbSwingAmount;
        headPitch += 0.2f * limbSwingAmount;
    }

    boost::shared_ptr<AbstractHorse> horse = boost::static_pointer_cast<AbstractHorse>(entity);

    float eatAnim = horse.get()->getEatAnim(partialTicks);
    float standAnim = horse.get()->getStandAnim(partialTicks);
    float notStandAnim = 1.0f - standAnim;
    float mouthAnim = horse.get()->getMouthAnim(partialTicks);
    bool tailSwinging = horse.get()->getTailCounter() != 0;
    bool saddled = horse.get()->isSaddled();

    float ageInTicks = (float)entity.get()->mTickCount + partialTicks;
    float legCos = Mth::cos(limbSwing * 0.6662f + 3.1415927f);

    mBody->mXRot = 0.0f;
    mNeck->mY = 7.0f;
    mNeck->mZ = -8.0f;

    float legSwing = legCos * 0.8f * limbSwingAmount;

    mNeck->mXRot = headPitch + 0.5235988f;
    mNeck->mYRot = netHeadYaw * Mth::DEG_TO_RAD;

    float bob = Mth::sin(ageInTicks);
    float notAnim = 1.0f - std::max(standAnim, eatAnim);

    mNeck->mXRot = notAnim * (mouthAnim * bob * 0.05f + mNeck->mXRot)
                   + (standAnim * (headPitch + 0.2617994f) + eatAnim * (bob * 0.05f + EATING_HEAD_X_ROT));
    mNeck->mYRot = standAnim * netHeadYaw * Mth::DEG_TO_RAD + notAnim * mNeck->mYRot;
    mNeck->mY = notAnim * mNeck->mY + (standAnim * -4.0f + eatAnim * 8.0f);
    mNeck->mZ = notAnim * mNeck->mZ + (eatAnim * -10.0f - standAnim);

    mBody->mXRot = standAnim * (-45.0f * Mth::DEG_TO_RAD) + notStandAnim * mBody->mXRot;

    mBag2->mY = standAnim * 5.5f + notStandAnim * mBag2DefaultY;
    mBag2->mZ = standAnim * 15.0f + notStandAnim * mBag2DefaultZ;

    mMane->mY = mNeck->mY;
    mMane->mZ = mNeck->mZ;
    mMane->mXRot = mNeck->mXRot;
    mMane->mYRot = mNeck->mYRot;

    mUMouth->mXRot = -(mouthAnim * 0.09424778f);
    mUMouth->mYRot = 0.0f;

    float bagXRot = legSwing / 5.0f;

    mBag1->mXRot = bagXRot;
    mBag2->mXRot = -bagXRot;
    mBag1->mY = mBag2->mY;
    mBag1->mZ = mBag2->mZ;

    float standCos = Mth::cos(ageInTicks * 0.6f + 3.1415927f);

    mLeg3A->mY = mLeg3ADefaultY * notStandAnim;
    mLeg3A->mZ = -5.0f * standAnim + mLeg3ADefaultZ * notStandAnim;
    mLeg4A->mY = mLeg3A->mY;
    mLeg4A->mZ = mLeg3A->mZ;

    float legFrontSwing = legCos * 0.5f * limbSwingAmount * notStandAnim;
    float legStandRot = 5.0f * Mth::DEG_TO_RAD * standAnim;

    mLeg1A->mXRot = legStandRot - legFrontSwing;
    mLeg2A->mXRot = legStandRot + legFrontSwing;
    mLeg3A->mXRot = (-60.0f * Mth::DEG_TO_RAD + standCos) * standAnim + legSwing * notStandAnim;
    mLeg4A->mXRot = (-60.0f * Mth::DEG_TO_RAD - standCos) * standAnim - legSwing * notStandAnim;

    if (saddled)
    {
        mSaddleMouthLine->mY = mNeck->mY;
        mSaddleMouthLineR->mY = mNeck->mY;
        mSaddleMouthLine->mZ = mNeck->mZ;
        mSaddleMouthLineR->mZ = mNeck->mZ;
        mSaddleMouthLine->mXRot = headPitch;
        mSaddleMouthLineR->mXRot = headPitch;
        mSaddleMouthLine->mYRot = mNeck->mYRot;
        mSaddleMouthLineR->mYRot = mNeck->mYRot;
    }

    mTailA->mXRot = standAnim * (80.0f * Mth::DEG_TO_RAD) + notStandAnim * (25.0f * Mth::DEG_TO_RAD);

    if (tailSwinging)
    {
        mTailA->mYRot = Mth::cos(ageInTicks * 0.7f);
    }
    else
    {
        mTailA->mYRot = 0.0f;
    }
}

void AbstractHorseModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                                float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    boost::shared_ptr<AbstractHorse> horse = boost::static_pointer_cast<AbstractHorse>(entity);

    bool baby = horse.get()->isBaby();
    unsigned long long type = entity.get()->GetType();

    bool saddled = !baby && horse.get()->isSaddled();
    bool chestedHorse = type == EntityType::DONKEY || type == EntityType::MULE;
    bool hasChest = !baby && chestedHorse && boost::static_pointer_cast<AbstractChestedHorse>(horse).get()->hasChest();

    float foalScale = horse.get()->getFoalScale();
    bool ridden = horse.get()->isVehicle();

    mSaddle->mShow = saddled;
    mSaddleMouthLine->mShow = saddled && ridden;
    mSaddleMouthLineR->mShow = saddled && ridden;
    mSaddleMouthL->mShow = saddled;
    mSaddleMouthR->mShow = saddled;
    mHeadSaddle->mShow = saddled;

    mSaddleMouthLine->render(scale, useDisplayList, false);
    mSaddleMouthLineR->render(scale, useDisplayList, false);

    mMuleEarL->mShow = chestedHorse;
    mMuleEarR->mShow = chestedHorse;
    mEar1->mShow = !chestedHorse;
    mEar2->mShow = !chestedHorse;

    if (baby)
    {
        GlStateManager::pushMatrix();
        GlStateManager::scalef(foalScale, foalScale * 0.5f + 0.5f, foalScale);
        GlStateManager::translatef(0.0f, 0.99f * (1.0f - foalScale), 0.0f);
    }

    mLeg1A->render(scale, useDisplayList, false);
    mLeg2A->render(scale, useDisplayList, false);
    mLeg3A->render(scale, useDisplayList, false);
    mLeg4A->render(scale, useDisplayList, false);

    if (baby)
    {
        GlStateManager::popMatrix();
        GlStateManager::pushMatrix();
        GlStateManager::scalef(foalScale, foalScale, foalScale);
        GlStateManager::translatef(0.0f, 2.3f * (1.0f - foalScale), 0.0f);
        GlStateManager::pushMatrix();

        float headScale = 0.5f * (1.0f - foalScale) + 1.0f;
        GlStateManager::scalef(headScale, headScale, headScale);
    }

    mNeck->render(scale, useDisplayList, false);
    mMane->render(scale, useDisplayList, false);

    if (baby)
    {
        GlStateManager::popMatrix();
    }

    mBody->render(scale, useDisplayList, false);

    if (baby)
    {
        GlStateManager::popMatrix();
    }

    if (hasChest)
    {
        mBag1->mShow = true;
        mBag2->mShow = true;

        mBag1->render(scale, useDisplayList, false);
        mBag2->render(scale, useDisplayList, false);

        callSkinnedDrawlist();
    }
    else
    {
        mBag1->mShow = false;
        mBag2->mShow = false;

        mBag1->render(scale, useDisplayList, false);
        mBag2->render(scale, useDisplayList, false);

        callSkinnedDrawlist();
    }
}
