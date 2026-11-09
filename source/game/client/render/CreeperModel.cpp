#include "game/client/render/CreeperModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Mth.hpp"

CreeperModel::CreeperModel(const ModelDefinition& modelDefinition)
{
    _init(0.0f, modelDefinition);
}

CreeperModel::CreeperModel(float delta, const ModelDefinition& modelDefinition)
{
    _init(delta, modelDefinition);
}

void CreeperModel::_init(float delta, const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, delta, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, delta, NULL);
        mLeg0 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, delta, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, delta, NULL);
        mLeg2 = new ModelPartSkin(this, L"leg2", modelDefinition, NULL, delta, NULL);
        mLeg3 = new ModelPartSkin(this, L"leg3", modelDefinition, NULL, delta, NULL);

        compileSkinnedDrawlist();
    }
    else
    {
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, delta);
        mHead->setPos(0.0f, 6.0f, 0.0f);

        mBody = new ModelPartSkin(this, 16, 16, NULL);
        mBody->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, delta);
        mBody->setPos(0.0f, 6.0f, 0.0f);

        mLeg0 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg0->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, delta);
        mLeg0->setPos(-2.0f, 18.0f, 4.0f);

        mLeg1 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg1->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, delta);
        mLeg1->setPos(2.0f, 18.0f, 4.0f);

        mLeg2 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg2->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, delta);
        mLeg2->setPos(-2.0f, 18.0f, -4.0f);

        mLeg3 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg3->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, delta);
        mLeg3->setPos(2.0f, 18.0f, -4.0f);

        compileSkinnedDrawlist();
    }
}

void CreeperModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                          float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mHead->render(scale, useDisplayList, false);
    mBody->render(scale, useDisplayList, false);
    mLeg0->render(scale, useDisplayList, false);
    mLeg1->render(scale, useDisplayList, false);
    mLeg2->render(scale, useDisplayList, false);
    mLeg3->render(scale, useDisplayList, false);

    callSkinnedDrawlist();
}

void CreeperModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                             float headPitch, float scale, boost::shared_ptr<Entity> entity,
                             _SkinAdjustments skinAdjustments)
{
    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;

    mLeg0->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount;
    mLeg1->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount;
    mLeg2->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount;
    mLeg3->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount;
}
