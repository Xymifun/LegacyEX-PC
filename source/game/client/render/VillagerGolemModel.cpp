#include "game/client/render/VillagerGolemModel.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/world/generation/VillagerGolem.hpp"

VillagerGolemModel::VillagerGolemModel(float scale, float yOffset, const ModelDefinition& modelDefinition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;

        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mArm1 = new ModelPartSkin(this, L"arm1", modelDefinition, NULL, 0.0f, NULL);
        mArm0 = new ModelPartSkin(this, L"arm0", modelDefinition, NULL, 0.0f, NULL);
        mLeg0 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, NULL);

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_IronGolem);

        compileSkinnedDrawlist();

        builder->useDefaultShaders();
    }
    else
    {
        mHead = new ModelPartSkin(this);
        mHead->setTexSize(128, 128);
        mHead->setPos(0.0f, yOffset, -2.0f);
        mHead->texOffs(0, 0)->addBox(-4.0f, -12.0f, -5.5f, 8, 10, 8, scale);
        mHead->texOffs(24, 0)->addBox(-1.0f, -5.0f, -7.5f, 2, 4, 2, scale);

        mBody = new ModelPartSkin(this);
        mBody->setTexSize(128, 128);
        mBody->setPos(0.0f, yOffset, 0.0f);
        mBody->texOffs(0, 40)->addBox(-9.0f, -2.0f, -6.0f, 18, 12, 11, scale);
        mBody->texOffs(0, 70)->addBox(-4.5f, 10.0f, -3.0f, 9, 5, 6, scale + 0.5f);

        mArm1 = new ModelPartSkin(this);
        mArm1->setTexSize(128, 128);
        mArm1->setPos(0.0f, -7.0f, 0.0f);
        mArm1->texOffs(60, 21)->addBox(-13.0f, -2.5f, -3.0f, 4, 30, 6, scale);

        mArm0 = new ModelPartSkin(this);
        mArm0->setTexSize(128, 128);
        mArm0->setPos(0.0f, -7.0f, 0.0f);
        mArm0->texOffs(60, 58)->addBox(9.0f, -2.5f, -3.0f, 4, 30, 6, scale);

        mLeg0 = new ModelPartSkin(this, 0, 22, NULL);
        mLeg0->setTexSize(128, 128);
        mLeg0->setPos(-4.0f, yOffset + 18.0f, 0.0f);
        mLeg0->texOffs(37, 0)->addBox(-3.5f, -3.0f, -3.0f, 6, 16, 5, scale);

        mLeg1 = new ModelPartSkin(this, 0, 22, NULL);
        mLeg1->setTexSize(128, 128);
        mLeg1->mMirror = true;
        mLeg1->texOffs(60, 0)->setPos(5.0f, yOffset + 18.0f, 0.0f);
        mLeg1->addBox(-3.5f, -3.0f, -3.0f, 6, 16, 5, scale);

        compileSkinnedDrawlist();

        builder->useDefaultShaders();
    }
}

void VillagerGolemModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                                float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mHead->render(scale, useDisplayList, false);
    mBody->render(scale, useDisplayList, false);
    mLeg0->render(scale, useDisplayList, false);
    mLeg1->render(scale, useDisplayList, false);
    mArm1->render(scale, useDisplayList, false);
    mArm0->render(scale, useDisplayList, false);

    callSkinnedDrawlist();
}

float VillagerGolemModel::triangleWave(float time, float period)
{
    return (fabsf(fmodf(time, period) - period * 0.5f) - period * 0.25f) / (period * 0.25f);
}

void VillagerGolemModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                   float headPitch, float scale, boost::shared_ptr<Entity> entity,
                                   _SkinAdjustments skinAdjustments)
{
    mHead->mYRot = netHeadYaw / 57.295776f;
    mHead->mXRot = headPitch / 57.295776f;

    mLeg0->mXRot = -1.5f * triangleWave(limbSwing, 13.0f) * limbSwingAmount;
    mLeg1->mXRot = 1.5f * triangleWave(limbSwing, 13.0f) * limbSwingAmount;

    mLeg0->mYRot = 0.0f;
    mLeg1->mYRot = 0.0f;
}

void VillagerGolemModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing,
                                         float limbSwingAmount, float partialTicks)
{
    boost::shared_ptr<VillagerGolem> golem = boost::dynamic_pointer_cast<VillagerGolem>(entity);

    int attackAnimationTick = golem->getAttackAnimationTick();

    if (attackAnimationTick > 0)
    {
        mArm1->mXRot = -2.0f + 1.5f * triangleWave((float)attackAnimationTick - partialTicks, 10.0f);
        mArm0->mXRot = -2.0f + 1.5f * triangleWave((float)attackAnimationTick - partialTicks, 10.0f);
    }
    else
    {
        int offerFlowerTick = golem->getOfferFlowerTick();

        if (offerFlowerTick > 0)
        {
            mArm1->mXRot = -0.8f + 0.025f * triangleWave((float)offerFlowerTick, 70.0f);
            mArm0->mXRot = 0.0f;
        }
        else
        {
            mArm1->mXRot = (-0.2f + 1.5f * triangleWave(limbSwing, 13.0f)) * limbSwingAmount;
            mArm0->mXRot = (-0.2f - 1.5f * triangleWave(limbSwing, 13.0f)) * limbSwingAmount;
        }
    }
}
