#include "game/client/render/DolphinModel.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Dolphin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"

DolphinModel::DolphinModel(const ModelDefinition& modelDefinition)
{
    mTexWidth = 64;
    mTexHeight = 64;

    if (modelDefinition.DoesOverrideModel())
    {
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mBackFin = new ModelPartSkin(this, L"back_fin", modelDefinition, mBody, 0.0f, NULL);
        mLeftFin = new ModelPartSkin(this, L"left_fin", modelDefinition, mBody, 0.0f, NULL);
        mRightFin = new ModelPartSkin(this, L"right_fin", modelDefinition, mBody, 0.0f, NULL);
        mTail = new ModelPartSkin(this, L"tail", modelDefinition, mBody, 0.0f, NULL);
        mTailFin = new ModelPartSkin(this, L"tail_fin", modelDefinition, mTail, 0.0f, NULL);
        mHead = new ModelPartSkin(this, L"head", modelDefinition, mBody, 0.0f, NULL);
        mNose = new ModelPartSkin(this, L"nose", modelDefinition, mHead, 0.0f, NULL);

        compileSkinnedDrawlist();
    }
    else
    {
        mBody = new ModelPartSkin(this, 0, 13, NULL);
        mBody->addBox(-4.0f, -7.0f, 0.0f, 8, 7, 13);
        mBody->setPos(0.0f, 24.0f, -3.0f);

        mBackFin = new ModelPartSkin(this, 29, 0, NULL);
        mBackFin->addBox(-0.5f, -5.0f, -1.0f, 1, 5, 4);
        mBackFin->setPos(0.0f, -7.0f, 7.0f);
        mBody->addChild(mBackFin);

        mLeftFin = new ModelPartSkin(this, 40, 0, NULL);
        mLeftFin->addBox(0.0f, -1.0f, -1.0f, 8, 1, 4);
        mLeftFin->setPos(3.0f, -1.0f, 2.0f);
        mBody->addChild(mLeftFin);

        mRightFin = new ModelPartSkin(this, 40, 6, NULL);
        mRightFin->addBox(-8.0f, -1.0f, -1.0f, 8, 1, 4);
        mRightFin->setPos(-3.0f, -1.0f, 2.0f);
        mBody->addChild(mRightFin);

        mTail = new ModelPartSkin(this, 0, 33, NULL);
        mTail->addBox(-2.0f, -2.5f, -1.0f, 4, 5, 11);
        mTail->setPos(0.0f, -2.5f, 14.0f);
        mBody->addChild(mTail);

        mTailFin = new ModelPartSkin(this, 0, 49, NULL);
        mTailFin->addBox(-5.0f, -0.5f, -1.0f, 10, 1, 6);
        mTailFin->setPos(0.0f, 0.0f, 9.0f);
        mTail->addChild(mTailFin);

        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-4.0f, -7.0f, -6.0f, 8, 7, 6);
        mHead->setPos(0.0f, 0.0f, 0.0f);

        mNose = new ModelPartSkin(this, 0, 13, NULL);
        mNose->addBox(-1.0f, -2.0f, 0.0f, 2, 2, 4);
        mNose->setPos(0.0f, 0.0f, -10.0f);
        mHead->addChild(mNose);

        mBody->addChild(mHead);

        compileSkinnedDrawlist();
    }
}

DolphinModel::~DolphinModel()
{
    delete mBody;
    delete mLeftFin;
    delete mRightFin;
    delete mTail;
    delete mTailFin;
    delete mHead;
    delete mNose;
}

void DolphinModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                          float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    if (mYoung)
    {
        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
    }

    mBody->render(scale, useDisplayList, false);

    if (mYoung)
    {
        GlStateManager::popMatrix();
    }

    callSkinnedDrawlist();
}

void DolphinModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                             float headPitch, float scale, boost::shared_ptr<Entity> entity,
                             _SkinAdjustments skinAdjustments)
{
    mBody->mXRot = headPitch * Mth::DEG_TO_RAD;
    mBody->mYRot = netHeadYaw * Mth::DEG_TO_RAD;

    if (eTYPE_DERIVED_FROM(EntityType::DOLPHIN, entity->GetType()))
    {
        boost::shared_ptr<Dolphin> dolphin = boost::static_pointer_cast<Dolphin>(entity);

        if (dolphin->mXd != 0.0 || dolphin->mZd != 0.0)
        {
            mBody->mXRot += -0.05f + -0.05f * Mth::cos(ageInTicks * 0.3f);
            mTail->mXRot = -0.1f * Mth::cos(ageInTicks * 0.3f);
            mTailFin->mXRot = -0.2f * Mth::cos(ageInTicks * 0.3f);
            mLeftFin->mZRot = 0.3f * Mth::cos(ageInTicks * 0.2f) + 0.4f;
            mRightFin->mZRot = -0.3f * Mth::cos(ageInTicks * 0.2f) - 0.4f;
        }
    }
}
