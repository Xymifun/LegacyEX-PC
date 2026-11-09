#include "game/client/render/IllagerModel.hpp"

#include <math.h>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/AbstractIllager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/LivingEntity.hpp"

IllagerModel::IllagerModel(float scale, float yOffset, int texWidth, int texHeight,
                           const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mArms = new ModelPartSkin(this, L"arms", modelDefinition, NULL, 0.0f, NULL);
        mLeg0 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, NULL);
        mNose = new ModelPartSkin(this, L"nose", modelDefinition, NULL, 0.0f, NULL);
        mRightArm = new ModelPartSkin(this, L"rightarm", modelDefinition, NULL, 0.0f, NULL);
        mLeftArm = new ModelPartSkin(this, L"leftarm", modelDefinition, NULL, 0.0f, NULL);

        mHead->addChild(mNose);

        compileSkinnedDrawlist();
    }
    else
    {
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

        ModelPart* mirroredArm = new ModelPartSkin(this, 44, 22, NULL);
        mirroredArm->setTexSize(texWidth, texHeight);
        mirroredArm->mMirror = true;
        mirroredArm->addBox(4.0f, -2.0f, -2.0f, 4, 8, 4, scale);
        mArms->addChild(mirroredArm);

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

        mRightArm = new ModelPartSkin(this, 40, 46, NULL);
        mRightArm->setTexSize(texWidth, texHeight);
        mRightArm->addBox(-3.0f, -2.0f, -2.0f, 4, 12, 4, scale);
        mRightArm->setPos(-5.0f, yOffset + 2.0f, 0.0f);

        mLeftArm = new ModelPartSkin(this, 40, 46, NULL);
        mLeftArm->setTexSize(texWidth, texHeight);
        mLeftArm->mMirror = true;
        mLeftArm->addBox(-1.0f, -2.0f, -2.0f, 4, 12, 4, scale);
        mLeftArm->setPos(5.0f, yOffset + 2.0f, 0.0f);

        mHead->addChild(mNose);

        compileSkinnedDrawlist();
    }
}

IllagerModel::~IllagerModel()
{
    delete mHead;
    delete mNose;
    delete mBody;
    delete mArms;
    delete mLeg0;
    delete mLeg1;
    delete mRightArm;
    delete mLeftArm;
}

void IllagerModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                          float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mHead->render(scale, useDisplayList, false);
    mBody->render(scale, useDisplayList, false);
    mLeg0->render(scale, useDisplayList, false);
    mLeg1->render(scale, useDisplayList, false);

    boost::shared_ptr<AbstractIllager> illager = boost::static_pointer_cast<AbstractIllager>(entity);
    if (illager->getArmPose() == ILLAGER_ARM_POSE_CROSSED)
    {
        mArms->mShow = true;
        mRightArm->mShow = false;
        mLeftArm->mShow = false;
    }
    else
    {
        mArms->mShow = false;
        mRightArm->mShow = true;
        mLeftArm->mShow = true;
    }

    mArms->render(scale, useDisplayList, false);
    mRightArm->render(scale, useDisplayList, false);
    mLeftArm->render(scale, useDisplayList, false);

    callSkinnedDrawlist();
}

void IllagerModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                             float headPitch, float scale, boost::shared_ptr<Entity> entity,
                             _SkinAdjustments skinAdjustments)
{
    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;

    mArms->mY = 3.0f;
    mArms->mXRot = -0.75f;
    mArms->mZ = -1.0f;

    mLeg0->mXRot = cosf(limbSwing * 0.6662f) * 1.4f * limbSwingAmount * 0.5f;
    mLeg1->mXRot = cosf(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount * 0.5f;
    mLeg0->mYRot = 0.0f;
    mLeg1->mYRot = 0.0f;

    int armPose = boost::static_pointer_cast<AbstractIllager>(entity)->getArmPose();

    if (armPose == ILLAGER_ARM_POSE_ATTACKING)
    {
        float swing = Mth::sin(mAttackTime * 3.1415927f);
        float recover = Mth::sin((1.0f - (1.0f - mAttackTime) * (1.0f - mAttackTime)) * 3.1415927f);

        mRightArm->mZRot = 0.0f;
        mLeftArm->mZRot = 0.0f;
        mRightArm->mYRot = 0.15707964f;
        mLeftArm->mYRot = -0.15707964f;

        if (boost::static_pointer_cast<LivingEntity>(entity)->getMainArm() == HumanoidArm::RIGHT)
        {
            mRightArm->mXRot = -1.8849558f + Mth::cos(ageInTicks * 0.09f) * 0.15f;
            mLeftArm->mXRot = Mth::cos(ageInTicks * 0.19f) * 0.5f;
            mRightArm->mXRot += swing * 2.2f - recover * 0.4f;
            mLeftArm->mXRot += swing * 1.2f - recover * 0.4f;
        }
        else
        {
            mRightArm->mXRot = Mth::cos(ageInTicks * 0.19f) * 0.5f;
            mLeftArm->mXRot = -1.8849558f + Mth::cos(ageInTicks * 0.09f) * 0.15f;
            mRightArm->mXRot += swing * 1.2f - recover * 0.4f;
            mLeftArm->mXRot += swing * 2.2f - recover * 0.4f;
        }

        mRightArm->mZRot += Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
        mLeftArm->mZRot -= Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
        mRightArm->mXRot += Mth::sin(ageInTicks * 0.067f) * 0.05f;
        mLeftArm->mXRot -= Mth::sin(ageInTicks * 0.067f) * 0.05f;
    }
    else if (armPose == ILLAGER_ARM_POSE_SPELLCASTING)
    {
        mRightArm->mX = -5.0f;
        mRightArm->mZ = 0.0f;
        mLeftArm->mX = 5.0f;
        mLeftArm->mZ = 0.0f;

        mRightArm->mXRot = Mth::cos(ageInTicks * 0.6662f) * 0.25f;
        mLeftArm->mXRot = Mth::cos(ageInTicks * 0.6662f) * 0.25f;
        mRightArm->mZRot = 2.3561945f;
        mLeftArm->mZRot = -2.3561945f;
        mRightArm->mYRot = 0.0f;
        mLeftArm->mYRot = 0.0f;
    }
    else if (armPose == ILLAGER_ARM_POSE_BOW_AND_ARROW)
    {
        mRightArm->mYRot = -0.1f + mHead->mYRot;
        mRightArm->mXRot = -1.5707964f + mHead->mXRot;
        mLeftArm->mXRot = -0.9424779f + mHead->mXRot;
        mLeftArm->mZRot = 1.5707964f;
        mLeftArm->mYRot = mHead->mYRot - 0.4f;
    }
}

ModelPart* IllagerModel::getArm(const HumanoidArm* arm)
{
    if (arm == HumanoidArm::LEFT)
    {
        return mLeftArm;
    }

    return mRightArm;
}
