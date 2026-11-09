#include "game/client/render/SkeletonModel.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/MaterialTable.hpp"

SkeletonModel::SkeletonModel(float scale, bool armourModel, const ModelDefinition& definition)
    : HumanoidModel(definition, scale, 0.0f, 64, 32, armourModel, true, true)
{
    _init(scale, armourModel, eSkeletonType_Skeleton, definition);
}

SkeletonModel::SkeletonModel(SkeletonType skeletonType, const ModelDefinition& definition)
    : HumanoidModel(definition, 0.0f, 0.0f, 64, 32, false, false, true)
{
    _init(0.0f, false, skeletonType, definition);
}

void SkeletonModel::_init(float scale, bool armourModel, SkeletonType skeletonType, const ModelDefinition& definition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (!armourModel)
    {
        if (definition.DoesOverrideModel())
        {
            mHasOverriddenParts = true;

            mHead = createModelPart(true, L"head", definition, NULL, 0.0f);
            mHat = createModelPart(true, L"hat", definition, NULL, 0.0f);
            mBody = createModelPart(true, L"body", definition, NULL, 0.0f);
            mRightArm = createModelPart(true, L"rightArm", definition, NULL, 0.0f);
            mLeftArm = createModelPart(true, L"leftArm", definition, NULL, 0.0f);
            mRightLeg = createModelPart(true, L"rightLeg", definition, NULL, 0.0f);
            mLeftLeg = createModelPart(true, L"leftLeg", definition, NULL, 0.0f);

            if (definition.hasModelPart(L"rightItem"))
            {
                mRightItem = createModelPart(true, L"rightItem", definition, mRightArm, 0.0f);
            }

            if (definition.hasModelPart(L"leftItem"))
            {
                mLeftItem = createModelPart(true, L"leftItem", definition, mLeftArm, 0.0f);
            }

            if (skeletonType == eSkeletonType_Stray)
            {
                builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Stray);
            }
            else if (skeletonType == eSkeletonType_WitherSkeleton)
            {
                builder->setVertexAndPixelShader(eMinecraftEntityMaterial_WitherSkeleton);
            }
            else if (skeletonType == eSkeletonType_Skeleton)
            {
                builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Skeleton);
            }
        }
        else
        {
            mHead = new ModelPartSkin(this, 0, 0, NULL);
            mHead->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale);
            mHead->setPos(0.0f, 0.0f, 0.0f);

            mHat = new ModelPartSkin(this, 32, 0, NULL);
            mHat->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale + HAT_INFLATE);
            mHat->setPos(0.0f, 0.0f, 0.0f);

            mBody = new ModelPartSkin(this, 16, 16, NULL);
            mBody->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, scale);
            mBody->setPos(0.0f, 0.0f, 0.0f);

            mRightArm = new ModelPartSkin(this, 40, 16, NULL);
            mRightArm->addBox(-1.0f, -2.0f, -1.0f, 2, 12, 2, scale);
            mRightArm->setPos(-5.0f, 2.0f, 0.0f);

            mLeftArm = new ModelPartSkin(this, 40, 16, NULL);
            mLeftArm->mMirror = true;
            mLeftArm->addBox(-1.0f, -2.0f, -1.0f, 2, 12, 2, scale);
            mLeftArm->setPos(5.0f, 2.0f, 0.0f);

            mRightLeg = new ModelPartSkin(this, 0, 16, NULL);
            mRightLeg->addBox(-1.0f, 0.0f, -1.0f, 2, 12, 2, scale);
            mRightLeg->setPos(-2.0f, 12.0f, 0.0f);

            mLeftLeg = new ModelPartSkin(this, 0, 16, NULL);
            mLeftLeg->mMirror = true;
            mLeftLeg->addBox(-1.0f, 0.0f, -1.0f, 2, 12, 2, scale);
            mLeftLeg->setPos(2.0f, 12.0f, 0.0f);
        }

        if (mRightItem == NULL)
        {
            mRightItem = createModelPart(true, 0, 0);
            mRightItem->setPos(-6.0f - mRightArm->mX, 9.0f - mRightArm->mY, 1.0f - mRightArm->mZ);
        }

        if (mLeftItem == NULL)
        {
            mLeftItem = createModelPart(true, 0, 0);
            mLeftItem->setPos(mRightItem->mX, mRightItem->mY, mRightItem->mZ);
        }

        mRightArm->addChild(mRightItem);
        mLeftArm->addChild(mLeftItem);

        compileSkinnedDrawlist();
    }

    builder->useDefaultShaders();
}

void SkeletonModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                    float partialTicks)
{
    mRightArmPose = eArmPose_Empty;
    mLeftArmPose = eArmPose_Empty;

    not_null_ptr<ItemInstance> itemInHand = entity->getItemInHand(InteractionHand::eInteractionHand_MainHand);

    if (itemInHand->getItem() == Items::bow && boost::static_pointer_cast<AbstractSkeleton>(entity)->isAggressive())
    {
        if (entity->getMainArm() == HumanoidArm::RIGHT)
        {
            mRightArmPose = eArmPose_BowAndArrow;
        }
        else
        {
            mLeftArmPose = eArmPose_BowAndArrow;
        }
    }

    Model::prepareMobModel(entity, limbSwing, limbSwingAmount, partialTicks);
}

void SkeletonModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                              float headPitch, float scale, boost::shared_ptr<Entity> entity,
                              _SkinAdjustments skinAdjustments)
{
    HumanoidModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                             _SkinAdjustments());

    not_null_ptr<ItemInstance> mainHandItem = boost::static_pointer_cast<LivingEntity>(entity)->getMainHandItem();
    boost::shared_ptr<AbstractSkeleton> skeleton = boost::static_pointer_cast<AbstractSkeleton>(entity);

    if (skeleton->isAggressive() && mainHandItem->getItem() != Items::bow)
    {
        float attackSwing = Mth::sin(mAttackTime * 3.1415927f);
        float attackSwell = Mth::sin((1.0f - (1.0f - mAttackTime) * (1.0f - mAttackTime)) * 3.1415927f);

        mRightArm->mZRot = 0.0f;
        mLeftArm->mZRot = 0.0f;

        mRightArm->mYRot = -(0.1f - attackSwing * 0.6f);
        mLeftArm->mYRot = 0.1f - attackSwing * 0.6f;

        mRightArm->mXRot = -1.5707964f;
        mLeftArm->mXRot = -1.5707964f;

        mRightArm->mXRot -= attackSwing * 1.2f - attackSwell * 0.4f;
        mLeftArm->mXRot -= attackSwing * 1.2f - attackSwell * 0.4f;

        mRightArm->mZRot += Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
        mLeftArm->mZRot -= Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;

        mRightArm->mXRot += Mth::sin(ageInTicks * 0.067f) * 0.05f;
        mLeftArm->mXRot -= Mth::sin(ageInTicks * 0.067f) * 0.05f;
    }
}

void SkeletonModel::translateToHandItem(float scale, const HumanoidArm* arm)
{
    float side = arm == HumanoidArm::RIGHT ? 1.0f : -1.0f;
    ModelPart* part = getArm(arm);

    part->mX += side;
    part->translateTo(scale);
    part->mX -= side;
}
