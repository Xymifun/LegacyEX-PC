#include "game/client/render/ZombieModel.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Zombie.hpp"

ZombieModel::ZombieModel(const ModelDefinition& definition)
    : HumanoidModel(definition, 0.0f, 0.0f, 64, 32, false, true, true)
{
}

ZombieModel::ZombieModel(float scale, bool armourModel, const ModelDefinition& definition)
    : HumanoidModel(definition, scale, 0.0f, 64, armourModel ? 32 : 64, armourModel, true, true)
{
}

ZombieModel::ZombieModel(C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                         const ModelDefinition& definition, float scale, float yOffset, int texWidth, int texHeight,
                         bool armourModel, bool createParts, bool skinned)
    : HumanoidModel(vertexType, pixelShaderType, definition, scale, yOffset, texWidth, texHeight, armourModel,
                    createParts, skinned)
{
}

void ZombieModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                            float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    HumanoidModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                             skinAdjustments);

    bool aggressive = false;

    if (eTYPE_DERIVED_FROM(EntityType::ZOMBIE, entity->GetType()))
    {
        boost::shared_ptr<Zombie> zombie = boost::dynamic_pointer_cast<Zombie>(entity);
        if (zombie->isAggressive())
        {
            aggressive = true;
        }
    }

    float attackSwing = Mth::sin(mAttackTime * 3.1415927f);
    float attackSwingBack = Mth::sin((1.0f - (1.0f - mAttackTime) * (1.0f - mAttackTime)) * 3.1415927f);

    mRightArm->mZRot = 0.0f;
    mLeftArm->mZRot = 0.0f;
    mRightArm->mYRot = -(0.1f - attackSwing * 0.6f);
    mLeftArm->mYRot = 0.1f - attackSwing * 0.6f;

    float armXRot = -3.1415927f / (aggressive ? 1.5f : 2.25f);

    if (mRightArmPose != eArmPose_ThrowSpear)
    {
        mRightArm->mXRot = armXRot;
    }

    mLeftArm->mXRot = armXRot;
    mRightArm->mXRot += attackSwing * 1.2f - attackSwingBack * 0.4f;
    mLeftArm->mXRot += attackSwing * 1.2f - attackSwingBack * 0.4f;
    mRightArm->mZRot += Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
    mLeftArm->mZRot -= Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
    mRightArm->mXRot += Mth::sin(ageInTicks * 0.067f) * 0.05f;
    mLeftArm->mXRot -= Mth::sin(ageInTicks * 0.067f) * 0.05f;
}
