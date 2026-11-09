#include "game/client/render/VillagerZombieModel.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Zombie.hpp"

VillagerZombieModel::VillagerZombieModel(const ModelDefinition& definition)
    : HumanoidModel(definition, 0.0f, 0.0f, 64, 64, false, true, true)
{
    _init(0.0f, 0.0f, false, definition);
}

VillagerZombieModel::VillagerZombieModel(float scale, float yOffset, bool armourModel,
                                         const ModelDefinition& definition)
    : HumanoidModel(definition, scale, 0.0f, 64, armourModel ? 32 : 64, false, true, true)
{
    _init(scale, yOffset, armourModel, definition);
}

void VillagerZombieModel::_init(float scale, float yOffset, bool armourModel, const ModelDefinition& definition)
{
    if (!definition.DoesOverrideModel())
    {
        delete mHead;
        delete mBody;
        delete mRightLeg;
        delete mLeftLeg;

        if (armourModel)
        {
            mHead = new ModelPartSkin(this, 0, 0, NULL);
            mHead->addBox(-4.0f, -10.0f, -4.0f, 8, 8, 8, scale);
            mHead->setPos(0.0f, yOffset, 0.0f);

            mBody = new ModelPartSkin(this, 16, 16, NULL);
            mBody->setPos(0.0f, yOffset, 0.0f);
            mBody->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, scale + 0.1f);

            mRightLeg = new ModelPartSkin(this, 0, 16, NULL);
            mRightLeg->setPos(-2.0f, yOffset + 12.0f, 0.0f);
            mRightLeg->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale + 0.1f);

            mLeftLeg = new ModelPartSkin(this, 0, 16, NULL);
            mLeftLeg->mMirror = true;
            mLeftLeg->setPos(2.0f, yOffset + 12.0f, 0.0f);
            mLeftLeg->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale + 0.1f);
        }
        else
        {
            delete mRightArm;
            delete mLeftArm;

            mHead = new ModelPartSkin(this, 0, 0, NULL);
            mHead->setPos(0.0f, yOffset, 0.0f);
            mHead->texOffs(0, 0)->addBox(-4.0f, -10.0f, -4.0f, 8, 10, 8, scale);
            mHead->texOffs(24, 0)->addBox(-1.0f, -3.0f, -6.0f, 2, 4, 2, scale);

            mBody = new ModelPartSkin(this, 16, 20, NULL);
            mBody->setPos(0.0f, yOffset, 0.0f);
            mBody->addBox(-4.0f, 0.0f, -3.0f, 8, 12, 6, scale);
            mBody->texOffs(0, 38)->addBox(-4.0f, 0.0f, -3.0f, 8, 18, 6, scale + 0.05f);

            mRightArm = new ModelPartSkin(this, 44, 38, NULL);
            mRightArm->addBox(-3.0f, -2.0f, -2.0f, 4, 12, 4, scale);
            mRightArm->setPos(-5.0f, yOffset + 2.0f, 0.0f);

            mLeftArm = new ModelPartSkin(this, 44, 38, NULL);
            mLeftArm->mMirror = true;
            mLeftArm->addBox(-1.0f, -2.0f, -2.0f, 4, 12, 4, scale);
            mLeftArm->setPos(5.0f, yOffset + 2.0f, 0.0f);

            mRightLeg = new ModelPartSkin(this, 0, 22, NULL);
            mRightLeg->setPos(-2.0f, yOffset + 12.0f, 0.0f);
            mRightLeg->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);

            mLeftLeg = new ModelPartSkin(this, 0, 22, NULL);
            mLeftLeg->mMirror = true;
            mLeftLeg->setPos(2.0f, yOffset + 12.0f, 0.0f);
            mLeftLeg->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
        }
    }

    compileSkinnedDrawlist();
}

void VillagerZombieModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                    float headPitch, float scale, boost::shared_ptr<Entity> entity,
                                    _SkinAdjustments skinAdjustments)
{
    HumanoidModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                             _SkinAdjustments());

    boost::shared_ptr<Zombie> zombie = boost::static_pointer_cast<Zombie>(entity);
    float swing = Mth::sin(mAttackTime * 3.1415927f);
    float swingBack = Mth::sin((1.0f - (1.0f - mAttackTime) * (1.0f - mAttackTime)) * 3.1415927f);

    mRightArm->mZRot = 0.0f;
    mLeftArm->mZRot = 0.0f;
    mRightArm->mYRot = -(0.1f - swing * 0.6f);
    mLeftArm->mYRot = 0.1f - swing * 0.6f;

    float armXRot = -3.1415927f / (zombie->isAggressive() ? 1.5f : 2.25f);
    mRightArm->mXRot = armXRot;
    mLeftArm->mXRot = armXRot;
    mRightArm->mXRot += swing * 1.2f - swingBack * 0.4f;
    mLeftArm->mXRot += swing * 1.2f - swingBack * 0.4f;
    mRightArm->mZRot += Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
    mLeftArm->mZRot -= Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
    mRightArm->mXRot += Mth::sin(ageInTicks * 0.067f) * 0.05f;
    mLeftArm->mXRot -= Mth::sin(ageInTicks * 0.067f) * 0.05f;
}
