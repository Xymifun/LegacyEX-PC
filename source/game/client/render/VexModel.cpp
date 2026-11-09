#include "game/client/render/VexModel.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Vex.hpp"

VexModel::VexModel(const ModelDefinition& modelDefinition)
    : HumanoidModel(modelDefinition, 0.0f, 0.0f, 64, 64, false, true, true)
{
    _init(0.0f, modelDefinition);
}

VexModel::~VexModel()
{
    delete mLeftWing;
    delete mRightWing;
}

void VexModel::_init(float scale, const ModelDefinition& modelDefinition)
{
    bool overridesModel = modelDefinition.DoesOverrideModel();
    mLeftLeg->mShow = false;
    if (overridesModel)
    {
        mHat->mShow = modelDefinition.hasModelPart(L"hat");
        delete mRightLeg;
        mRightLeg = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mRightWing = new ModelPartSkin(this, L"rightwing", modelDefinition, NULL, 0.0f, NULL);
        mLeftWing = new ModelPartSkin(this, L"leftwing", modelDefinition, NULL, 0.0f, NULL);
        compileSkinnedDrawlist();
    }
    else
    {
        mHat->mShow = false;
        delete mRightLeg;
        mRightLeg = new ModelPartSkin(this, 32, 0, NULL);
        mRightLeg->addBox(-1.0f, -1.0f, -2.0f, 6, 10, 4, 0.0f);
        mRightLeg->setPos(-1.9f, 12.0f, 0.0f);

        mRightWing = new ModelPartSkin(this, 0, 32, NULL);
        mRightWing->addBox(-20.0f, 0.0f, 0.0f, 20, 12, 1);

        mLeftWing = new ModelPartSkin(this, 0, 32, NULL);
        mLeftWing->mMirror = true;
        mLeftWing->addBox(0.0f, 0.0f, 0.0f, 20, 12, 1);

        compileSkinnedDrawlist();
    }
}

void VexModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                      float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupBoneMatrices(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, useDisplayList);
    mRightWing->render(scale, useDisplayList, false);
    mLeftWing->render(scale, useDisplayList, false);
    callSkinnedDrawlist();
}

void VexModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                         float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    HumanoidModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                             skinAdjustments);

    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || !eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        boost::shared_ptr<Vex> vex = boost::static_pointer_cast<Vex>(entity);
        if (vex->isCharging())
        {
            if (vex->getMainArm() == HumanoidArm::RIGHT)
            {
                mRightArm->mXRot = 3.7699115f;
            }
            else
            {
                mLeftArm->mXRot = 3.7699115f;
            }
        }
    }

    mRightLeg->mXRot += 0.62831855f;
    mRightWing->mZ = 2.0f;
    mLeftWing->mZ = 2.0f;
    mRightWing->mY = 1.0f;
    mLeftWing->mY = 1.0f;
    float wingAngle = 0.47123894f + std::cos(ageInTicks * 0.8f) * 3.1415927f * 0.05f;
    mRightWing->mYRot = wingAngle;
    mLeftWing->mYRot = -wingAngle;
    mLeftWing->mZRot = -0.47123894f;
    mLeftWing->mXRot = 0.47123894f;
    mRightWing->mXRot = 0.47123894f;
    mRightWing->mZRot = 0.47123894f;
}

unsigned int VexModel::getModelVersion()
{
    return 23;
}
