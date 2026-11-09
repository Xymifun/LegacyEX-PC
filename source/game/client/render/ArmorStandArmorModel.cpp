#include "game/client/render/ArmorStandArmorModel.hpp"
#include "game/core/Mth.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ArmorStand.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"

namespace
{

const float RADIANS_PER_DEGREE = 0.017453292f;

}

ArmorStandArmorModel::ArmorStandArmorModel(float scale, int texWidth, int texHeight, const ModelDefinition& definition)
    : HumanoidModel(definition, scale, 0.0f, texWidth, texHeight, true, true, true)
{
}

ArmorStandArmorModel::~ArmorStandArmorModel()
{
}

void ArmorStandArmorModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                     float headPitch, float scale, boost::shared_ptr<Entity> entity,
                                     _SkinAdjustments skinAdjustments)
{
    if (eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, entity.get()->GetType()))
    {
        boost::shared_ptr<ArmorStand> armorStand = boost::dynamic_pointer_cast<ArmorStand>(entity);

        mHead->mXRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getHeadPose()->x;
        mHead->mYRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getHeadPose()->y;
        mHead->mZRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getHeadPose()->z;
        mHead->setPos(0.0f, 1.0f, 0.0f);

        mBody->mXRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getBodyPose()->x;
        mBody->mYRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getBodyPose()->y;
        mBody->mZRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getBodyPose()->z;

        mLeftArm->mXRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getLeftArmPose()->x;
        mLeftArm->mYRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getLeftArmPose()->y;
        mLeftArm->mZRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getLeftArmPose()->z;

        mRightArm->mXRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getRightArmPose()->x;
        mRightArm->mYRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getRightArmPose()->y;
        mRightArm->mZRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getRightArmPose()->z;

        mLeftLeg->mXRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getLeftLegPose()->x;
        mLeftLeg->mYRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getLeftLegPose()->y;
        mLeftLeg->mZRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getLeftLegPose()->z;
        mLeftLeg->setPos(1.9f, 12.0f, 0.0f);

        mRightLeg->mXRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getRightLegPose()->x;
        mRightLeg->mYRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getRightLegPose()->y;
        mRightLeg->mZRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getRightLegPose()->z;
        mRightLeg->setPos(-1.9f, 12.0f, 0.0f);

        mRightItem->mXRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getRightItemPose()->x;
        mRightItem->mYRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getRightItemPose()->y;
        mRightItem->mZRot = RADIANS_PER_DEGREE * (float)armorStand.get()->getRightItemPose()->z;

        if (armorStand.get()->getRightItemPose()->y == 180.0)
        {
            mRightItem->mX = -2.0f;
        }
        else
        {
            mRightItem->mX = 0.0f;
        }

        Model::copyModelPart(mHead, mHat);
    }
}
