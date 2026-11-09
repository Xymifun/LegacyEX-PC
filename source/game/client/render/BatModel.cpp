#include "game/client/render/BatModel.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/Bat.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"

BatModel::BatModel(const ModelDefinition& modelDefinition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;

        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mRightEar = new ModelPartSkin(this, L"rightEar", modelDefinition, mHead, 0.0f, NULL);
        mLeftEar = new ModelPartSkin(this, L"leftEar", modelDefinition, mHead, 0.0f, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mRightWing = new ModelPartSkin(this, L"rightWing", modelDefinition, NULL, 0.0f, NULL);
        mLeftWing = new ModelPartSkin(this, L"leftWing", modelDefinition, NULL, 0.0f, NULL);
        mRightWingTip = new ModelPartSkin(this, L"rightWingTip", modelDefinition, NULL, 0.0f, NULL);
        mLeftWingTip = new ModelPartSkin(this, L"leftWingTip", modelDefinition, NULL, 0.0f, NULL);

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Bat);

        mHead->addChild(mRightEar);
        mHead->addChild(mLeftEar);
        mBody->addChild(mRightWing);
        mBody->addChild(mLeftWing);
        mRightWing->addChild(mRightWingTip);
        mLeftWing->addChild(mLeftWingTip);

        compileSkinnedDrawlist();

        builder->useDefaultShaders();
    }
    else
    {
        mTexWidth = 64;
        mTexHeight = 64;

        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-3.0f, -3.0f, -3.0f, 6, 6, 6);

        mRightEar = new ModelPartSkin(this, 24, 0, NULL);
        mRightEar->addBox(-4.0f, -6.0f, -2.0f, 3, 4, 1);

        mLeftEar = new ModelPartSkin(this, 24, 0, NULL);
        mLeftEar->mMirror = true;
        mLeftEar->addBox(1.0f, -6.0f, -2.0f, 3, 4, 1);

        mBody = new ModelPartSkin(this, 0, 16, NULL);
        mBody->addBox(-3.0f, 4.0f, -3.0f, 6, 12, 6);
        mBody->texOffs(0, 34)->addBox(-5.0f, 16.0f, 0.0f, 10, 6, 1);

        mRightWing = new ModelPartSkin(this, 42, 0, NULL);
        mRightWing->addBox(-12.0f, 1.0f, 1.5f, 10, 16, 1);

        mRightWingTip = new ModelPartSkin(this, 24, 16, NULL);
        mRightWingTip->setPos(-12.0f, 1.0f, 1.5f);
        mRightWingTip->addBox(-8.0f, 1.0f, 0.0f, 8, 12, 1);

        mLeftWing = new ModelPartSkin(this, 42, 0, NULL);
        mLeftWing->mMirror = true;
        mLeftWing->addBox(2.0f, 1.0f, 1.5f, 10, 16, 1);

        mLeftWingTip = new ModelPartSkin(this, 24, 16, NULL);
        mLeftWingTip->mMirror = true;
        mLeftWingTip->setPos(12.0f, 1.0f, 1.5f);
        mLeftWingTip->addBox(0.0f, 1.0f, 0.0f, 8, 12, 1);

        mHead->addChild(mRightEar);
        mHead->addChild(mLeftEar);
        mBody->addChild(mRightWing);
        mBody->addChild(mLeftWing);
        mRightWing->addChild(mRightWingTip);
        mLeftWing->addChild(mLeftWingTip);

        compileSkinnedDrawlist();

        builder->useDefaultShaders();
    }
}

void BatModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                      float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mHead->render(scale, useDisplayList, false);
    mBody->render(scale, useDisplayList, false);

    callSkinnedDrawlist();
}

void BatModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                         float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    boost::shared_ptr<Bat> bat = boost::dynamic_pointer_cast<Bat>(entity);

    if (bat && bat->isResting())
    {
        mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
        mHead->mYRot = 3.1415927f - netHeadYaw * Mth::DEG_TO_RAD;
        mHead->mZRot = 3.1415927f;
        mHead->setPos(0.0f, -2.0f, 0.0f);

        mRightWing->setPos(-3.0f, 0.0f, 3.0f);
        mLeftWing->setPos(3.0f, 0.0f, 3.0f);

        mBody->mXRot = 3.1415927f;

        mRightWing->mXRot = -0.15707964f;
        mRightWing->mYRot = -1.2566371f;
        mRightWingTip->mYRot = -1.7278761f;
        mLeftWing->mXRot = mRightWing->mXRot;
        mLeftWing->mYRot = -mRightWing->mYRot;
        mLeftWingTip->mYRot = -mRightWingTip->mYRot;
    }
    else
    {
        mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
        mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
        mHead->mZRot = 0.0f;
        mHead->setPos(0.0f, 0.0f, 0.0f);

        mRightWing->setPos(0.0f, 0.0f, 0.0f);
        mLeftWing->setPos(0.0f, 0.0f, 0.0f);

        mBody->mXRot = cosf(ageInTicks * 0.1f) * 0.15f + 0.7853982f;
        mBody->mYRot = 0.0f;

        mRightWing->mYRot = cosf(ageInTicks * 1.3f) * 3.1415927f * 0.25f;
        mLeftWing->mYRot = -mRightWing->mYRot;

        mRightWingTip->mYRot = mRightWing->mYRot * 0.5f;
        mLeftWingTip->mYRot = -mRightWing->mYRot * 0.5f;
    }
}
