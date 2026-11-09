#include "game/client/render/DrownedModel.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/ZombieModel.hpp"
#include "game/core/Drowned.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/MaterialTable.hpp"

#ifndef MCWIIU_EMISSIVE_MODEL_NO_MIPMAPS
#define MCWIIU_EMISSIVE_MODEL_NO_MIPMAPS 1
#endif

DrownedModel::DrownedModel(const ModelDefinition& definition)
    : ZombieModel(C4JRender::eVertexType_Entity, C4JRender::ePixelShaderType_EntityEmissiveAlpha, definition, 0.0f,
                  0.0f, 64, 64, false, false, true)
{
    init(0.0f, definition);
}

void DrownedModel::init(float scale, const ModelDefinition& definition)
{
    if (!definition.DoesOverrideModel())
    {
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale + 0.5f);
        mHead->setPos(0.0f, 0.0f, 0.0f);

        mHat = new ModelPartSkin(this, 32, 0, NULL);
        mHat->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale + 1.0f);
        mHat->setPos(0.0f, 0.0f, 0.0f);

        mBody = new ModelPartSkin(this, 16, 16, NULL);
        mBody->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, scale);
        mBody->setPos(0.0f, 0.0f, 0.0f);

        mRightArm = new ModelPartSkin(this, 0, 16, NULL);
        mRightArm->addBox(-2.0f, -2.0f, -2.0f, 4, 12, 4, scale);
        mRightArm->setPos(-5.0f, 2.0f, 0.0f);

        mLeftArm = new ModelPartSkin(this, 40, 16, NULL);
        mLeftArm->mMirror = true;
        mLeftArm->addBox(-1.0f, -2.0f, -2.0f, 4, 12, 4, scale);
        mLeftArm->setPos(5.0f, 2.0f, 0.0f);

        mRightLeg = new ModelPartSkin(this, 16, 48, NULL);
        mRightLeg->addBox(-2.15f, 0.0f, -2.0f, 4, 12, 4, scale);
        mRightLeg->setPos(-1.9f, 12.0f, 0.0f);

        mLeftLeg = new ModelPartSkin(this, 32, 48, NULL);
        mLeftLeg->mMirror = true;
        mLeftLeg->addBox(-1.85f, 0.0f, -2.0f, 4, 12, 4, scale);
        mLeftLeg->setPos(1.9f, 12.0f, 0.0f);

        mLeftSleeve = new ModelPartSkin(this, 40, 32, NULL);
        mLeftSleeve->mirror();
        mLeftSleeve->addBox(-1.0f, -2.0f, -2.0f, 4, 12, 4, scale + 0.52f);
        mLeftSleeve->setWorldPos(5.0f, 2.0f, 0.0f);

        mRightSleeve = new ModelPartSkin(this, 48, 48, NULL);
        mRightSleeve->addBox(-2.0f, -2.0f, -2.0f, 4, 12, 4, scale + 0.52f);
        mRightSleeve->setWorldPos(-5.0f, 2.0f, 0.0f);

        mLeftPants = new ModelPartSkin(this, 0, 32, NULL);
        mLeftLeg->mMirror = true;
        mLeftPants->addBox(-1.65f, 0.0f, -2.0f, 4, 12, 4, scale + 0.25f);
        mLeftPants->setWorldPos(1.9f, 12.0f, 0.0f);

        mRightPants = new ModelPartSkin(this, 0, 48, NULL);
        mRightPants->addBox(-2.35f, 0.0f, -2.0f, 4, 12, 4, scale + 0.25f);
        mRightPants->setWorldPos(-1.9f, 12.0f, 0.0f);

        mJacket = new ModelPartSkin(this, 16, 32, NULL);
        mJacket->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, scale + 0.52f);
        mJacket->setWorldPos(0.0f, 0.0f, 0.0f);

        mLeftArm->addChild(mLeftSleeve);
        mRightArm->addChild(mRightSleeve);
        mLeftLeg->addChild(mLeftPants);
        mRightLeg->addChild(mRightPants);
        mBody->addChild(mJacket);

        if (mRightItem == NULL)
        {
            mRightItem = new ModelPartSkin(this, 0, 0, NULL);
            mRightItem->setPos(-6.0f - mRightArm->mX, 9.0f - mRightArm->mY, 1.0f - mRightArm->mZ);
        }

        if (mLeftItem == NULL)
        {
            mLeftItem = new ModelPartSkin(this, 0, 0, NULL);
            mLeftItem->setPos(mRightItem->mX, mRightItem->mY, mRightItem->mZ);
        }

        mRightArm->addChild(mRightItem);
        mLeftArm->addChild(mLeftItem);

        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Drowned);

        const bool mipmap = builder->setMipmapEnable(!MCWIIU_EMISSIVE_MODEL_NO_MIPMAPS);
        compileSkinnedDrawlist();
        builder->setMipmapEnable(mipmap);

        builder->useDefaultShaders();
    }
    else
    {
        mHead = new ModelPartSkin(this, L"head", definition, NULL, scale, NULL);
        mHat = new ModelPartSkin(this, L"hat", definition, NULL, scale, NULL);
        mBody = new ModelPartSkin(this, L"body", definition, NULL, scale, NULL);
        mRightArm = new ModelPartSkin(this, L"rightArm", definition, NULL, scale, NULL);
        mLeftArm = new ModelPartSkin(this, L"leftArm", definition, NULL, scale, NULL);
        mRightLeg = new ModelPartSkin(this, L"rightLeg", definition, NULL, scale, NULL);
        mLeftLeg = new ModelPartSkin(this, L"leftLeg", definition, NULL, scale, NULL);

        if (definition.hasModelPart(L"rightItem"))
        {
            mRightItem = new ModelPartSkin(this, L"rightItem", definition, mRightArm, scale, NULL);
        }

        if (definition.hasModelPart(L"leftItem"))
        {
            mLeftItem = new ModelPartSkin(this, L"leftItem", definition, mLeftArm, scale, NULL);
        }

        mLeftSleeve = new ModelPartSkin(this, L"leftSleeve", definition, mLeftArm, scale, NULL);
        mRightSleeve = new ModelPartSkin(this, L"rightSleeve", definition, mRightArm, scale, NULL);
        mLeftPants = new ModelPartSkin(this, L"leftPants", definition, mLeftLeg, scale, NULL);
        mRightPants = new ModelPartSkin(this, L"rightPants", definition, mRightLeg, scale, NULL);
        mJacket = new ModelPartSkin(this, L"jacket", definition, mBody, scale, NULL);

        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Drowned);

        const bool mipmap = builder->setMipmapEnable(!MCWIIU_EMISSIVE_MODEL_NO_MIPMAPS);
        compileSkinnedDrawlist();
        builder->setMipmapEnable(mipmap);

        builder->useDefaultShaders();
    }
}

void DrownedModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                             float headPitch, float scale, boost::shared_ptr<Entity> entity,
                             _SkinAdjustments skinAdjustments)
{
    ZombieModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                           skinAdjustments);

    float swimAmount = eTYPE_DERIVED_FROM(EntityType::DROWNED, entity->GetType())
                       && boost::static_pointer_cast<Drowned>(entity)->getSwimAmount(limbSwingAmount) != 0.0f;

    float swing = Mth::sin(mAttackTime * 3.1415927f);
    float swingBack = Mth::sin((1.0f - (1.0f - mAttackTime) * (1.0f - mAttackTime)) * 3.1415927f);

    if (swimAmount > 0.0f)
    {
        mRightArm->mYRot = Mth::Lerp(mRightArm->mYRot, 0.25f, swimAmount);
        mLeftArm->mYRot = Mth::Lerp(mLeftArm->mYRot, 0.25f, swimAmount);
        mRightArm->mXRot = Mth::Lerp(mRightArm->mXRot, -3.1415927f, swimAmount);
        mLeftArm->mXRot = Mth::Lerp(mLeftArm->mXRot, -3.1415927f, swimAmount);
        mRightArm->mZRot = Mth::Lerp(mRightArm->mZRot, -0.25f, swimAmount);
        mLeftArm->mZRot = Mth::Lerp(mLeftArm->mZRot, 0.25f, swimAmount);

        float armSwing = swimAmount * (swing * 1.2f - swingBack * 0.4f);
        mRightArm->mXRot -= armSwing;
        mLeftArm->mXRot -= armSwing;

        float zBob = swimAmount * (Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f);
        float xBob = swimAmount * (Mth::sin(ageInTicks * 0.067f) * 0.05f);
        mRightArm->mZRot += swimAmount * zBob;
        mLeftArm->mZRot -= swimAmount * zBob;
        mRightArm->mXRot += swimAmount * xBob;
        mLeftArm->mXRot -= swimAmount * xBob;
    }
}

void DrownedModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                   float partialTicks)
{
    mRightArmPose = eArmPose_Empty;
    mLeftArmPose = eArmPose_Empty;

    not_null_ptr<ItemInstance> itemInHand = entity->getItemInHand(InteractionHand::eInteractionHand_MainHand);

    if (itemInHand->getItem() == Items::trident && boost::static_pointer_cast<Drowned>(entity)->isAggressive())
    {
        if (entity->getMainArm() == HumanoidArm::RIGHT)
        {
            mRightArmPose = eArmPose_ThrowSpear;
        }
        else
        {
            mLeftArmPose = eArmPose_ThrowSpear;
        }
    }

    Model::prepareMobModel(entity, limbSwing, limbSwingAmount, partialTicks);
}

void DrownedModel::setAllVisible(bool visible)
{
    HumanoidModel::setAllVisible(visible);

    mLeftSleeve->mShow = visible;
    mRightSleeve->mShow = visible;
    mLeftPants->mShow = visible;
    mRightPants->mShow = visible;
    mJacket->mShow = visible;
}
