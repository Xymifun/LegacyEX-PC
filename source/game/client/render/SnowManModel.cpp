#include "game/client/render/SnowManModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Mth.hpp"

SnowManModel::SnowManModel(const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mPiece1 = new ModelPartSkin(this, L"piece1", modelDefinition, NULL, 0.0f, NULL);
        mPiece2 = new ModelPartSkin(this, L"piece2", modelDefinition, NULL, 0.0f, NULL);
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mArm1 = new ModelPartSkin(this, L"arm1", modelDefinition, NULL, 0.0f, NULL);
        mArm2 = new ModelPartSkin(this, L"arm2", modelDefinition, NULL, 0.0f, NULL);

        compileSkinnedDrawlist();
    }
    else
    {
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->setTexSize(64, 64);
        mHead->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, -0.5f);
        mHead->setPos(0.0f, 4.0f, 0.0f);

        mArm1 = new ModelPartSkin(this, 32, 0, NULL);
        mArm1->setTexSize(64, 64);
        mArm1->addBox(-1.0f, 0.0f, -1.0f, 12, 2, 2, -0.5f);
        mArm1->setPos(0.0f, 6.0f, 0.0f);

        mArm2 = new ModelPartSkin(this, 32, 0, NULL);
        mArm2->setTexSize(64, 64);
        mArm2->addBox(-1.0f, 0.0f, -1.0f, 12, 2, 2, -0.5f);
        mArm2->setPos(0.0f, 6.0f, 0.0f);

        mPiece1 = new ModelPartSkin(this, 0, 16, NULL);
        mPiece1->setTexSize(64, 64);
        mPiece1->addBox(-5.0f, -10.0f, -5.0f, 10, 10, 10, -0.5f);
        mPiece1->setPos(0.0f, 13.0f, 0.0f);

        mPiece2 = new ModelPartSkin(this, 0, 36, NULL);
        mPiece2->setTexSize(64, 64);
        mPiece2->addBox(-6.0f, -12.0f, -6.0f, 12, 12, 12, -0.5f);
        mPiece2->setPos(0.0f, 24.0f, 0.0f);

        compileSkinnedDrawlist();
    }
}

void SnowManModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                          float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mPiece1->render(scale, useDisplayList, false);
    mPiece2->render(scale, useDisplayList, false);
    mHead->render(scale, useDisplayList, false);
    mArm1->render(scale, useDisplayList, false);
    mArm2->render(scale, useDisplayList, false);

    callSkinnedDrawlist();
}

void SnowManModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                             float headPitch, float scale, boost::shared_ptr<Entity> entity,
                             _SkinAdjustments skinAdjustments)
{
    Model::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;

    mPiece1->mYRot = netHeadYaw * Mth::DEG_TO_RAD * 0.25f;

    float sinBody = Mth::sin(mPiece1->mYRot);
    float cosBody = Mth::cos(mPiece1->mYRot);

    mArm1->mZRot = 1.0f;
    mArm2->mZRot = -1.0f;

    mArm1->mYRot = mPiece1->mYRot;
    mArm2->mYRot = mPiece1->mYRot + 3.1415927f;

    mArm1->mX = cosBody * 5.0f;
    mArm1->mZ = -sinBody * 5.0f;
    mArm2->mX = -cosBody * 5.0f;
    mArm2->mZ = sinBody * 5.0f;
}
