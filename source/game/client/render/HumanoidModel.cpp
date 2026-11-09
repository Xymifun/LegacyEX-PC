#include "game/client/render/HumanoidModel.hpp"
#include "game/core/EntityType.hpp"

#include <math.h>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "vendor/SKIN_BOX.hpp"

const float HumanoidModel::HAT_INFLATE = 0.5f;
const float HumanoidModel::OVERLAY_INFLATE = 0.25f;

HumanoidModel::HumanoidModel(const ModelDefinition& definition, float scale, bool armourModel)
{
    _init(scale, 0.0f, 64, 32, armourModel, true, definition, true);
}

HumanoidModel::HumanoidModel(const ModelDefinition& definition, float scale, float yOffset, int texWidth, int texHeight,
                             bool armourModel, bool createParts, bool skinned)
{
    _init(scale, yOffset, texWidth, texHeight, armourModel, createParts, definition, skinned);
}

HumanoidModel::HumanoidModel(C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                             const ModelDefinition& definition, float scale, float yOffset, int texWidth, int texHeight,
                             bool armourModel, bool createParts, bool skinned)
{
    Tesselator::getInstance()->getBuilder()->setVertexAndPixelShader(vertexType, pixelShaderType);

    _init(scale, yOffset, texWidth, texHeight, armourModel, createParts, definition, skinned);
}

ModelPart* HumanoidModel::createModelPart(bool skinned, const std::wstring& name, const ModelDefinition& definition,
                                          ModelPart* parent, float scale)
{
    if (skinned)
    {
        return new ModelPartSkin(this, name, definition, parent, scale, NULL);
    }

    return new ModelPart(this, name, definition, parent, scale);
}

ModelPart* HumanoidModel::createModelPart(bool skinned, int xTexOffs, int yTexOffs)
{
    if (skinned)
    {
        return new ModelPartSkin(this, xTexOffs, yTexOffs, NULL);
    }

    return new ModelPart(this, xTexOffs, yTexOffs);
}

void HumanoidModel::_init(float scale, float yOffset, int texWidth, int texHeight, bool armourModel, bool createParts,
                          const ModelDefinition& definition, bool skinned)
{
    mTexWidth = texWidth;
    mTexHeight = texHeight;
    mYOffset = yOffset;
    mRightItem = NULL;
    mLeftItem = NULL;

    if (createParts)
    {
        if (definition.DoesOverrideModel())
        {
            mHead = createModelPart(skinned, L"head", definition, NULL, 0.0f);
            mHat = createModelPart(skinned, L"hat", definition, NULL, 0.0f);
            mBody = createModelPart(skinned, L"body", definition, NULL, 0.0f);
            mRightArm = createModelPart(skinned, L"rightArm", definition, NULL, 0.0f);
            mLeftArm = createModelPart(skinned, L"leftArm", definition, NULL, 0.0f);
            mRightLeg = createModelPart(skinned, L"rightLeg", definition, NULL, 0.0f);
            mLeftLeg = createModelPart(skinned, L"leftLeg", definition, NULL, 0.0f);

            if (definition.hasModelPart(L"rightItem"))
            {
                mRightItem = createModelPart(skinned, L"rightItem", definition, mRightArm, 0.0f);
            }

            if (definition.hasModelPart(L"leftItem"))
            {
                mLeftItem = createModelPart(skinned, L"leftItem", definition, mLeftArm, 0.0f);
            }
        }
        else
        {
            mHead = createModelPart(skinned, 0, 0);
            mHead->addHumanoidBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale);
            mHead->setPos(0.0f, yOffset, 0.0f);

            mHat = createModelPart(skinned, 32, 0);
            mHat->addHumanoidBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale + 0.5f);
            mHat->setPos(0.0f, yOffset, 0.0f);

            mBody = createModelPart(skinned, 16, 16);
            mBody->addHumanoidBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, scale);
            mBody->setPos(0.0f, yOffset, 0.0f);

            mRightArm = createModelPart(skinned, 40, 16);
            mRightArm->addHumanoidBox(-3.0f, -2.0f, -2.0f, 4, 12, 4, scale);
            mRightArm->setPos(-5.0f, yOffset + 2.0f, 0.0f);

            mLeftArm = createModelPart(skinned, 40, 16);
            mLeftArm->mMirror = true;
            mLeftArm->addHumanoidBox(-1.0f, -2.0f, -2.0f, 4, 12, 4, scale);
            mLeftArm->setPos(5.0f, yOffset + 2.0f, 0.0f);

            mRightLeg = createModelPart(skinned, 0, 16);
            mRightLeg->addHumanoidBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
            mRightLeg->setPos(-1.9f, yOffset + 12.0f, 0.0f);

            mLeftLeg = createModelPart(skinned, 0, 16);
            mLeftLeg->mMirror = true;
            mLeftLeg->addHumanoidBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
            mLeftLeg->setPos(1.9f, yOffset + 12.0f, 0.0f);
        }

        if (mRightItem == NULL)
        {
            mRightItem = createModelPart(skinned, 0, 0);
            mRightItem->setPos(-6.0f - mRightArm->mX, 9.0f - mRightArm->mY, 1.0f - mRightArm->mZ);
        }

        if (mLeftItem == NULL)
        {
            mLeftItem = createModelPart(skinned, 0, 0);
            mLeftItem->setPos(mRightItem->mX, mRightItem->mY, mRightItem->mZ);
        }

        mRightArm->addChild(mRightItem);
        mLeftArm->addChild(mLeftItem);

        compileSkinnedDrawlist();
    }

    mSwimAmount = 0.0f;
    mUseItemProgress = 0.0f;
    mCrouching = false;
    mEating = false;
    mRightArmPose = eArmPose_Empty;
    mUseItemRemainingTicks = 0.0f;
    mSwimming = false;
    mSpearThrowProgress = 0.0f;
    mSitting = false;
    mIsArmourModel = armourModel;
    mLeftArmPose = eArmPose_Empty;
    mArmourSlot = EquipmentSlot::CHEST;
}

bool HumanoidModel::IsBodyPartDisabled(animbits bodyPart)
{
    bool enabled = (mSkinAdjustments.mAnimFlags & (1 << bodyPart)) == 0;

    if (!enabled && mIsArmourModel)
    {
        bool isArmourPart = true;
        switch (bodyPart)
        {
        case eAnimBit_Head:
            bodyPart = eAnimBit_ArmourHead;
            break;
        case eAnimBit_RightArm:
            bodyPart = eAnimBit_ArmourRightArm;
            break;
        case eAnimBit_LeftArm:
            bodyPart = eAnimBit_ArmourLeftArm;
            break;
        case eAnimBit_Body:
            bodyPart = eAnimBit_ArmourBody;
            break;
        case eAnimBit_RightLeg:
            bodyPart = eAnimBit_ArmourRightLeg;
            break;
        case eAnimBit_LeftLeg:
            bodyPart = eAnimBit_ArmourLeftLeg;
            break;
        default:
            isArmourPart = false;
            break;
        }

        enabled = isArmourPart && (mSkinAdjustments.mAnimFlags & (1 << bodyPart)) != 0;
    }

    return !enabled;
}

const HumanoidArm* HumanoidModel::getAttackArm(boost::shared_ptr<Entity> entity)
{
    if (Entity::instanceof(entity, EntityType::LIVING_ENTITY))
    {
        boost::shared_ptr<LivingEntity> livingEntity = boost::static_pointer_cast<LivingEntity>(entity);

        const HumanoidArm* arm = livingEntity->getMainArm();

        if (livingEntity->mSwingingArm != InteractionHand::eInteractionHand_MainHand)
        {
            arm = arm->getOpposite();
        }

        return arm;
    }

    return HumanoidArm::RIGHT;
}

ModelPart* HumanoidModel::getArm(const HumanoidArm* arm)
{
    if (arm == HumanoidArm::LEFT)
    {
        return mLeftArm;
    }

    return mRightArm;
}

float HumanoidModel::quadraticArmUpdate(float value)
{
    return -65.0f * value + value * value;
}

void HumanoidModel::setArmPose(const HumanoidArm* arm, eArmPose pose)
{
    if (arm == HumanoidArm::LEFT)
    {
        mLeftArmPose = pose;
        return;
    }

    mRightArmPose = pose;
}

void HumanoidModel::setArmPose(boost::shared_ptr<Entity> entity, eArmPose pose, bool useAttackArm)
{
    if (useAttackArm)
    {
        setArmPose(getAttackArm(entity), pose);
    }
    else
    {
        setArmPose(getAttackArm(entity)->getOpposite(), pose);
    }
}

void HumanoidModel::setAllVisible(bool visible)
{
    mHead->mShow = visible;
    mHat->mShow = visible;
    mBody->mShow = visible;
    mRightArm->mShow = visible;
    mLeftArm->mShow = visible;
    mRightLeg->mShow = visible;
    mLeftLeg->mShow = visible;
}

void HumanoidModel::translateToHandItem(float scale, const HumanoidArm* arm)
{
    getArm(arm)->translateTo(scale);
}

void HumanoidModel::copyPropertiesFrom(Model* model)
{
    Model::copyPropertiesFrom(model);

    if (model != NULL && dynamic_cast<HumanoidModel*>(model) != NULL)
    {
        HumanoidModel* humanoidModel = static_cast<HumanoidModel*>(model);

        mLeftArmPose = humanoidModel->mLeftArmPose;
        mRightArmPose = humanoidModel->mRightArmPose;
        mCrouching = humanoidModel->mCrouching;
        mSwimming = humanoidModel->mSwimming;
        mSpearThrowProgress = humanoidModel->mSpearThrowProgress;
        mSitting = humanoidModel->mSitting;
        mEating = humanoidModel->mEating;
        mUseItemRemainingTicks = humanoidModel->mUseItemRemainingTicks;
        mUseItemProgress = humanoidModel->mUseItemProgress;
        mSkinAdjustments = humanoidModel->mSkinAdjustments;
        mYOffset = humanoidModel->mYOffset;
    }
}

ModelPart* HumanoidModel::AddOrRetrievePart(SKIN_BOX* skinBox, unsigned int unusedArmourFlags)
{
    ModelPart* parent = NULL;

    switch (skinBox->id)
    {
    case 1:
        parent = mHead;
        break;

    case 2:
        parent = mBody;
        break;

    case 3:
        parent = mRightArm;
        break;

    case 4:
        parent = mLeftArm;
        break;

    case 5:
        parent = mRightLeg;
        break;

    case 6:
        parent = mLeftLeg;
        break;
    }

    ModelPart* part = parent->retrieveChild(skinBox);

    if (part != NULL && part->getfU() == (int)skinBox->xTexOffs && part->getfV() == (int)skinBox->yTexOffs)
    {
        return part;
    }

    part = new ModelPart(this, (int)skinBox->xTexOffs, (int)skinBox->yTexOffs);
    part->mShow = false;
    part->mMirror = skinBox->mirror != 0;
    part->addHumanoidBox(skinBox->x, skinBox->y, skinBox->z, (int)skinBox->width, (int)skinBox->height,
                         (int)skinBox->depth, skinBox->delta);
    part->mArmourFlags = skinBox->armourFlags;

    parent->addChild(part);

    return part;
}

void HumanoidModel::overrideAnim()
{
    mHead->mXRot = -45.0f * Mth::DEG_TO_RAD;

    mBody->mYRot = 0.0f;

    mRightArm->mZ = 0.0f;
    mRightArm->mX = -5.0f;
    mLeftArm->mZ = 0.0f;
    mLeftArm->mX = 5.0f;

    mRightLeg->mYRot = 0.0f;
    mLeftLeg->mYRot = 0.0f;
    mRightLeg->mZRot = 0.0f;
    mLeftLeg->mZRot = 0.0f;

    mRightArm->mXRot = 0.0f;
    mLeftArm->mXRot = 0.0f;
    mRightArm->mZRot = 0.0f;
    mLeftArm->mZRot = 0.0f;
    mRightArm->mYRot = 0.0f;
    mLeftArm->mYRot = 0.0f;

    mBody->mXRot = 0.0f;
    mRightLeg->mZ = 0.1f;
    mLeftLeg->mZ = 0.1f;

    mRightLeg->mY = 22.0f;
    mRightLeg->mOriginalY = 22.0f;
    mLeftLeg->mY = 22.0f;
    mLeftLeg->mOriginalY = 22.0f;

    mBody->mZ = 0.0f;
    mBody->mY = 10.0f;
    mBody->mOriginalY = 10.0f;

    mRightArm->mY = 12.0f;
    mRightArm->mOriginalY = 12.0f;
    mLeftArm->mY = 12.0f;
    mLeftArm->mOriginalY = 12.0f;

    mHead->mY = 10.0f;
    mHead->mOriginalY = 10.0f;

    Model::copyModelPart(mHead, mHat);
}

void HumanoidModel::overrideRender(float scale, bool useDisplayList)
{
    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0f, -0.9f, 0.0f);

    mHead->render(scale, useDisplayList, false);
    mBody->render(scale, useDisplayList, false);
    mRightArm->render(scale, useDisplayList, false);
    mLeftArm->render(scale, useDisplayList, false);
    mRightLeg->render(scale, useDisplayList, false);
    mLeftLeg->render(scale, useDisplayList, false);
    mHat->render(scale, useDisplayList, false);

    callSkinnedDrawlist();

    GlStateManager::popMatrix();
}

void HumanoidModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                           float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupBoneMatrices(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, useDisplayList);

    callSkinnedDrawlist();
}

void HumanoidModel::render(HumanoidModel* model, float scale, bool useDisplayList)
{
    setupBoneMatrices(model, scale, useDisplayList);

    callSkinnedDrawlist();
}

void HumanoidModel::setupBoneMatrices(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                                      float ageInTicks, float netHeadYaw, float headPitch, float scale,
                                      bool useDisplayList)
{
    float yTranslation = 0.0f;

    if (entity)
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
            && boost::static_pointer_cast<Player>(entity)->isSpectator())
        {
            if (boost::static_pointer_cast<Player>(entity)->SpectateAsFloatingHead())
            {
                mSkinAdjustments = entity->getSkinAdjustments();
                yTranslation = 1.5f;
            }
            else
            {
                mSkinAdjustments = _SkinAdjustments();
            }
        }
        else
        {
            mSkinAdjustments = entity->getSkinAdjustments();
        }
    }

    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, mSkinAdjustments);

    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0f, yTranslation, 0.0f);

    if (mYoung)
    {
        GlStateManager::scalef(0.75f, 0.75f, 0.75f);
        GlStateManager::translatef(0.0f, 16.0f * scale, 0.0f);

        mHead->render(scale, useDisplayList, false);
        mHat->render(scale, useDisplayList, false);

        GlStateManager::popMatrix();

        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);

        mBody->render(scale, useDisplayList, false);
        mRightArm->render(scale, useDisplayList, false);
        mLeftArm->render(scale, useDisplayList, false);
        mRightLeg->render(scale, useDisplayList, false);
        mLeftLeg->render(scale, useDisplayList, false);

        GlStateManager::popMatrix();
    }
    else
    {
        if (entity)
        {
            bool fallFlying = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
                              && boost::static_pointer_cast<LivingEntity>(entity)->getFallFlyingTicks() > 4;

            if (entity && entity->isSneaking() && !fallFlying)
            {
                GlStateManager::translatef(0.0f, 0.2f, 0.0f);
            }
        }

        mHead->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_Head));
        mBody->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_Body));
        mRightArm->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightArm));
        mLeftArm->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftArm));
        mRightLeg->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightLeg));
        mLeftLeg->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftLeg));
        mHat->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_Hat));

        GlStateManager::popMatrix();
    }
}

void HumanoidModel::setupBoneMatrices(HumanoidModel* model, float scale, bool useDisplayList)
{
    mHead->mYRot = model->mHead->mYRot;
    mHead->mY = model->mHead->mY;
    mHead->mOriginalY = model->mHead->mY;
    mHead->mXRot = model->mHead->mXRot;

    mBody->mYRot = model->mBody->mYRot;

    mRightArm->mXRot = model->mRightArm->mXRot;
    mRightArm->mYRot = model->mRightArm->mYRot;
    mRightArm->mZRot = model->mRightArm->mZRot;

    mLeftArm->mXRot = model->mLeftArm->mXRot;
    mLeftArm->mYRot = model->mLeftArm->mYRot;
    mLeftArm->mZRot = model->mLeftArm->mZRot;

    mRightLeg->mXRot = model->mRightLeg->mXRot;
    mLeftLeg->mXRot = model->mLeftLeg->mXRot;

    Model::copyModelPart(mHead, mHat);

    mHead->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_Head));
    mBody->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_Body));
    mRightArm->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightArm));
    mLeftArm->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftArm));
    mRightLeg->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightLeg));
    mLeftLeg->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftLeg));
    mHat->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_Hat));
}

void HumanoidModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                              float headPitch, float scale, boost::shared_ptr<Entity> entity,
                              _SkinAdjustments skinAdjustments)
{
    bool fallFlying = false;

    if (entity)
    {
        mSwimAmount = entity->getSwimAmount(limbSwingAmount);

        fallFlying = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
                     && boost::static_pointer_cast<LivingEntity>(entity)->getFallFlyingTicks() > 4;

        bool fallFlyingDisabled = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
                                  && boost::static_pointer_cast<LivingEntity>(entity)->isFallFlyingStateDisabled();

        mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;

        if (fallFlying && !fallFlyingDisabled)
        {
            mHead->mXRot = -45.0f * Mth::DEG_TO_RAD;
        }
        else if (mSwimAmount > 0.0f)
        {
            mHead->mXRot = headPitch / Mth::RAD_TO_DEG;
            mHead->mXRot = Mth::Lerp(mHead->mXRot, -45.0f * Mth::DEG_TO_RAD, mSwimAmount);
        }
        else
        {
            mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
        }
    }

    mBody->mYRot = 0.0f;
    mRightArm->mZ = 0.0f;
    mRightArm->mX = -5.0f;
    mLeftArm->mX = 5.0f;
    mLeftArm->mZ = 0.0f;

    float speed = 1.0f;

    if (fallFlying)
    {
        speed = (float)(entity->mXd * entity->mXd + entity->mYd * entity->mYd + entity->mZd * entity->mZd) / 0.2f;
        speed = speed * speed * speed;

        if (speed < 1.0f)
        {
            speed = 1.0f;
        }
    }

    mRightArm->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 2.0f * limbSwingAmount * 0.5f / speed;
    mLeftArm->mXRot = Mth::cos(limbSwing * 0.6662f) * 2.0f * limbSwingAmount * 0.5f / speed;
    mRightArm->mZRot = 0.0f;
    mLeftArm->mZRot = 0.0f;
    mRightLeg->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount / speed;
    mLeftLeg->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount / speed;
    mRightLeg->mYRot = 0.0f;
    mLeftLeg->mYRot = 0.0f;
    mRightLeg->mZRot = 0.0f;
    mLeftLeg->mZRot = 0.0f;

    if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_ArmsDown)) != 0)
    {
        mRightArm->mXRot = 0.0f;
        mLeftArm->mXRot = 0.0f;
    }
    else if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_ArmsOut)) != 0)
    {
        mRightArm->mXRot = -1.5707964f;
        mLeftArm->mXRot = -1.5707964f;
    }
    else if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_ArmsInSync)) != 0)
    {
        mRightArm->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 2.0f * limbSwingAmount * 0.5f / speed;
        mLeftArm->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 2.0f * limbSwingAmount * 0.5f / speed;
    }
    else if (((skinAdjustments.mAnimFlags & (1u << eAnimBit_RightArmOverride)) != 0 || (fallFlying && mCrouching))
             && mRightArmPose == eArmPose_Empty && mAttackTime == 0.0f)
    {
        mRightArm->mXRot = -3.1415927f;
        mRightArm->mZRot = -0.3f;
        mLeftArm->mXRot = Mth::cos(limbSwing * 0.6662f) * 2.0f * limbSwingAmount * 0.5f / speed;
    }
    else
    {
        mRightArm->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 2.0f * limbSwingAmount * 0.5f / speed;
        mLeftArm->mXRot = Mth::cos(limbSwing * 0.6662f) * 2.0f * limbSwingAmount * 0.5f / speed;
    }

    mLeftArm->mZRot = 0.0f;
    mRightLeg->mYRot = 0.0f;
    mLeftLeg->mYRot = 0.0f;
    mRightLeg->mZRot = 0.0f;
    mLeftLeg->mZRot = 0.0f;
    mRightArm->mYRot = 0.0f;

    if (mRiding)
    {
        mRightArm->mXRot += -0.62831855f;
        mRightArm->mZRot = 0.0f;
        mLeftArm->mXRot += -0.62831855f;

        if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_Seated)) == 0)
        {
            if ((skinAdjustments.mAnimFlags & ((1u << eAnimBit_LegsStill) | (1u << eAnimBit_LegsInSync))) == 0)
            {
                mRightLeg->mYRot = 0.31415927f;
                mLeftLeg->mYRot = -0.31415927f;
            }

            mRightLeg->mXRot = -1.2566371f;
            mLeftLeg->mXRot = -1.2566371f;
        }
        else
        {
            if ((skinAdjustments.mAnimFlags & ((1u << eAnimBit_LegsStill) | (1u << eAnimBit_LegsInSync))) == 0)
            {
                mRightLeg->mYRot = 0.31415927f;
                mLeftLeg->mYRot = -0.31415927f;
            }

            mRightLeg->mXRot = -1.4137167f;
            mRightLeg->mZRot = 0.07853982f;
            mLeftLeg->mXRot = -1.4137167f;
            mLeftLeg->mZRot = -0.07853982f;
        }
    }
    else if (mRidingDolphin)
    {
        mRightArm->mXRot += 2.5132742f;
        mRightArm->mZRot = -0.7853982f;
        mLeftArm->mXRot = 0.0f;
        mLeftLeg->mXRot = 0.0f;
        mRightLeg->mXRot = 0.0f;
    }
    else
    {
        mRightArm->mZRot = 0.0f;

        if (mSitting && !mCrouching && !fallFlying)
        {
            mRightLeg->mXRot = -1.5707964f;
            mLeftLeg->mXRot = -1.5707964f;
            mRightLeg->mYRot = 0.31415927f;
            mLeftLeg->mYRot = -0.31415927f;
        }
        else
        {
            if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_LegsStill)) != 0)
            {
                mRightLeg->mZRot = 0.0f;
                mRightLeg->mXRot = 0.0f;
                mLeftLeg->mXRot = 0.0f;
                mLeftLeg->mZRot = 0.0f;
                mRightLeg->mYRot = 0.0f;
                mLeftLeg->mYRot = 0.0f;
            }
            else if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_LegsInSync)) != 0)
            {
                mRightLeg->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount / speed;
                mLeftLeg->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount / speed;
            }
            else if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_Seated)) != 0)
            {
                mRightLeg->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount / speed;
                mLeftLeg->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount / speed;
            }
            else
            {
                mRightLeg->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount / speed;
                mLeftLeg->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount / speed;
            }
        }
    }

    switch (mLeftArmPose)
    {
    case eArmPose_Empty:
        mLeftArm->mYRot = 0.0f;
        break;

    case eArmPose_Item:
        mLeftArm->mYRot = 0.0f;
        mLeftArm->mXRot = mLeftArm->mXRot * 0.5f - 0.31415927f;
        break;

    case eArmPose_Block:
        mLeftArm->mXRot = mLeftArm->mXRot * 0.5f - 0.9424779f;
        mLeftArm->mYRot = 30.0f * Mth::DEG_TO_RAD;
        break;

    case eArmPose_ThrowSpear:
    {
        float leftSpear = mLeftArm->mXRot * 0.5f - 3.1415927f;

        mLeftArm->mYRot = 0.0f;
        mLeftArm->mXRot = leftSpear * 0.8f + leftSpear * 0.2f * mSpearThrowProgress;
    }
    break;

    default:
        break;
    }

    switch (mRightArmPose)
    {
    case eArmPose_Empty:
        mRightArm->mYRot = 0.0f;
        break;

    case eArmPose_Item:
        mRightArm->mYRot = 0.0f;
        mRightArm->mXRot = mRightArm->mXRot * 0.5f - 0.31415927f;
        break;

    case eArmPose_Block:
        mRightArm->mXRot = mRightArm->mXRot * 0.5f - 0.9424779f;
        mRightArm->mYRot = -30.0f * Mth::DEG_TO_RAD;
        break;

    case eArmPose_ThrowSpear:
    {
        float rightSpear = mRightArm->mXRot * 0.5f - 3.1415927f;

        mRightArm->mYRot = 0.0f;
        mRightArm->mXRot = rightSpear * 0.8f + rightSpear * 0.2f * mSpearThrowProgress;
    }
    break;

    default:
        break;
    }

    if (mAttackTime > 0.0f)
    {
        const HumanoidArm* attackArm = getAttackArm(entity);
        ModelPart* attackArmPart = getArm(attackArm);

        float attackRoot = sqrtf(mAttackTime);
        float swing = Mth::sin(attackRoot * 3.1415927f + attackRoot * 3.1415927f) * 0.2f;

        if (attackArm == HumanoidArm::LEFT)
        {
            swing = -swing;
        }

        mBody->mYRot = swing;

        mRightArm->mZ = Mth::sin(mBody->mYRot) * 5.0f;
        mRightArm->mX = -Mth::cos(mBody->mYRot) * 5.0f;
        mLeftArm->mZ = -Mth::sin(mBody->mYRot) * 5.0f;
        mLeftArm->mX = Mth::cos(mBody->mYRot) * 5.0f;

        mRightArm->mYRot += mBody->mYRot;
        mLeftArm->mYRot += mBody->mYRot;
        mLeftArm->mXRot += mBody->mYRot;

        float recover = 1.0f - mAttackTime;
        recover = recover * recover;

        float lift = Mth::sin((1.0f - recover * recover) * 3.1415927f);
        float tilt = Mth::sin(mAttackTime * 3.1415927f) * -(mHead->mXRot - 0.7f) * 0.75f;

        attackArmPart->mXRot = attackArmPart->mXRot - (tilt + lift * 1.2f);
        attackArmPart->mYRot += mBody->mYRot + mBody->mYRot;
        attackArmPart->mZRot += Mth::sin(mAttackTime * 3.1415927f) * -0.4f;

        if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_RightArmOverride)) != 0 && mRightArmPose == eArmPose_Empty
            && mAttackTime == 0.0f)
        {
            mRightArm->mZRot -= Mth::sin(mAttackTime * 3.1415927f) * -0.4f;
        }
        else
        {
            mRightArm->mZRot += Mth::sin(mAttackTime * 3.1415927f) * -0.4f;
        }
    }

    if (mEating)
    {
        float chew = 1.0f - mUseItemProgress;
        chew = chew * chew * chew;
        chew = chew * chew * chew;
        chew = chew * chew * chew;
        chew = 1.0f - chew;

        float raise = mUseItemProgress > 0.2 ? 1.0f : 0.0f;
        float bob = -Mth::abs(Mth::cos(mUseItemRemainingTicks * 0.25f * 3.1415927f) * 0.1f);

        mRightArm->mYRot = mRightArm->mYRot - chew * 0.5f;
        mRightArm->mXRot = bob * raise * 2.0f - chew * 1.2f;
    }

    if (mCrouching && !fallFlying)
    {
        if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_Seated)) != 0)
        {
            mBody->mXRot = -0.5f;
            mRightArm->mXRot += 0.4f;
            mLeftArm->mXRot += 0.4f;
            mRightLeg->mZ = -4.0f;
            mLeftLeg->mZ = -4.0f;
            mBody->mZ = 2.0f;
            mBody->mY = 0.0f;
            mBody->mOriginalY = 0.0f;
            mRightArm->mY = 2.0f;
            mRightArm->mOriginalY = 2.0f;
            mLeftArm->mY = 2.0f;
            mLeftArm->mOriginalY = 2.0f;
            mRightLeg->mY = 9.0f;
            mRightLeg->mOriginalY = 9.0f;
            mLeftLeg->mY = 9.0f;
            mLeftLeg->mOriginalY = 9.0f;
            mHead->mY = 1.0f;
            mHead->mOriginalY = 1.0f;
        }
        else
        {
            mBody->mXRot = 0.5f;
            mRightArm->mXRot += 0.4f;
            mLeftArm->mXRot += 0.4f;
            mRightLeg->mZ = 4.0f;
            mLeftLeg->mZ = 4.0f;
            mBody->mZ = 0.0f;
            mBody->mY = 0.0f;
            mBody->mOriginalY = 0.0f;
            mRightArm->mY = 2.0f;
            mRightArm->mOriginalY = 2.0f;
            mLeftArm->mY = 2.0f;
            mLeftArm->mOriginalY = 2.0f;
            mRightLeg->mY = 9.0f;
            mRightLeg->mOriginalY = 9.0f;
            mLeftLeg->mY = 9.0f;
            mLeftLeg->mOriginalY = 9.0f;
            mHead->mY = 1.0f;
            mHead->mOriginalY = 1.0f;
        }
    }
    else if (mSwimming && !fallFlying && mSwimAmount <= 0.0f)
    {
        mLeftArm->mZRot = 0.0f;
        mLeftArm->mXRot = -45.0f;
        mLeftArm->mYRot = 0.0f;
        mRightArm->mYRot = 0.0f;
        mRightArm->mXRot = 45.0f;
        mRightArm->mZRot = 0.0f;
        mLeftLeg->mYRot = 0.0f;
        mLeftLeg->mXRot = 45.0f;
        mLeftLeg->mZRot = 0.0f;
        mRightLeg->mXRot = -45.0f;
        mRightLeg->mYRot = 0.0f;
        mRightLeg->mZRot = 0.0f;
    }
    else
    {
        mBody->mXRot = 0.0f;
        mRightLeg->mZ = 0.1f;
        mLeftLeg->mZ = 0.1f;

        if (!mRiding && mSitting)
        {
            mRightLeg->mY = 22.0f;
            mRightLeg->mOriginalY = 22.0f;
            mLeftLeg->mY = 22.0f;
            mLeftLeg->mOriginalY = 22.0f;
            mBody->mZ = 0.0f;
            mBody->mY = 10.0f;
            mBody->mOriginalY = 10.0f;
            mRightArm->mY = 12.0f;
            mRightArm->mOriginalY = 12.0f;
            mLeftArm->mY = 12.0f;
            mLeftArm->mOriginalY = 12.0f;
            mHead->mY = 10.0f;
            mHead->mOriginalY = 10.0f;
        }
        else
        {
            mRightLeg->mY = 12.0f;
            mRightLeg->mOriginalY = 12.0f;
            mLeftLeg->mY = 12.0f;
            mLeftLeg->mOriginalY = 12.0f;
            mBody->mZ = 0.0f;
            mBody->mY = 0.0f;
            mBody->mOriginalY = 0.0f;
            mRightArm->mY = 2.0f;
            mRightArm->mOriginalY = 2.0f;
            mLeftArm->mY = 2.0f;
            mLeftArm->mOriginalY = 2.0f;
            mHead->mY = 0.0f;
            mHead->mOriginalY = 0.0f;
        }
    }

    mRightArm->mZRot += Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
    mLeftArm->mZRot -= Mth::cos(ageInTicks * 0.09f) * 0.05f + 0.05f;
    mRightArm->mXRot += Mth::sin(ageInTicks * 0.067f) * 0.05f;
    mLeftArm->mXRot -= Mth::sin(ageInTicks * 0.067f) * 0.05f;

    if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_LegsStill)) == 0
        && (skinAdjustments.mAnimFlags & (1u << eAnimBit_LegsInSync)) == 0)
    {
        mLeftLeg->mXRot += Mth::sin(ageInTicks * 0.067f) * 0.05f;
    }

    if (mRightArmPose == eArmPose_BowAndArrow)
    {
        mRightArm->mYRot = mHead->mYRot + -0.1f;
        mLeftArm->mYRot = mHead->mYRot + 0.5f;
        mRightArm->mXRot = mHead->mXRot + -1.5707964f;
        mLeftArm->mXRot = mHead->mXRot + -1.5707964f;
    }
    else if (mLeftArmPose == eArmPose_BowAndArrow)
    {
        mRightArm->mYRot = mHead->mYRot + -0.5f;
        mLeftArm->mYRot = mHead->mYRot + 0.1f;
        mRightArm->mXRot = mHead->mXRot + -1.5707964f;
        mLeftArm->mXRot = mHead->mXRot + -1.5707964f;
    }

    if (mSwimAmount > 0.0f && !mRidingDolphin)
    {
        if (mAttackTime <= 0.0f)
        {
            float strokePos = fmodf(limbSwing, 26.0f);
            float strokeAmount = mAttackTime > 0.0f ? 0.0f : mSwimAmount;

            if (strokePos < 14.0f)
            {
                mLeftArm->mXRot = Mth::lerpRotateDegrees(mLeftArm->mXRot * Mth::RAD_TO_DEG, 0.0f * Mth::RAD_TO_DEG,
                                                         mSwimAmount)
                                  * Mth::DEG_TO_RAD;
                mRightArm->mXRot = Mth::Lerp(mRightArm->mXRot, 0.0f, strokeAmount);
                mLeftArm->mYRot = Mth::lerpRotateDegrees(mLeftArm->mYRot * Mth::RAD_TO_DEG,
                                                         3.1415927f * Mth::RAD_TO_DEG, mSwimAmount)
                                  * Mth::DEG_TO_RAD;
                mRightArm->mYRot = Mth::Lerp(mRightArm->mYRot, 3.1415927f, strokeAmount);

                float leftArmRoll = mLeftArm->mZRot;
                float stroke = 1.8707964f * (quadraticArmUpdate(strokePos) / quadraticArmUpdate(14.0f));

                mLeftArm->mZRot = Mth::lerpRotateDegrees(leftArmRoll * Mth::RAD_TO_DEG,
                                                         (stroke + 3.1415927f) * Mth::RAD_TO_DEG, mSwimAmount)
                                  * Mth::DEG_TO_RAD;
                mRightArm->mZRot = Mth::Lerp(mRightArm->mZRot, 3.1415927f - stroke, strokeAmount);
            }
            else if (strokePos < 22.0f)
            {
                float lift = 1.5707964f * (strokePos - 14.0f) * 0.125f;

                mLeftArm->mXRot = Mth::lerpRotateDegrees(mLeftArm->mXRot * Mth::RAD_TO_DEG, lift * Mth::RAD_TO_DEG,
                                                         mSwimAmount)
                                  * Mth::DEG_TO_RAD;
                mRightArm->mXRot = Mth::Lerp(mRightArm->mXRot, lift, strokeAmount);
                mLeftArm->mYRot = Mth::lerpRotateDegrees(mLeftArm->mYRot * Mth::RAD_TO_DEG,
                                                         3.1415927f * Mth::RAD_TO_DEG, mSwimAmount)
                                  * Mth::DEG_TO_RAD;
                mRightArm->mYRot = Mth::Lerp(mRightArm->mYRot, 3.1415927f, strokeAmount);

                float stroke = 1.8707964f * (strokePos - 14.0f) * 0.125f;

                mLeftArm->mZRot = Mth::lerpRotateDegrees(mLeftArm->mZRot * Mth::RAD_TO_DEG,
                                                         (5.012389f - stroke) * Mth::RAD_TO_DEG, mSwimAmount)
                                  * Mth::DEG_TO_RAD;
                mRightArm->mZRot = Mth::Lerp(mRightArm->mZRot, stroke + 1.2707963f, strokeAmount);
            }
            else if (strokePos < 26.0f)
            {
                float drop = 1.5707964f - 1.5707964f * (strokePos - 22.0f) * 0.25f;

                mLeftArm->mXRot = Mth::lerpRotateDegrees(mLeftArm->mXRot * Mth::RAD_TO_DEG, drop * Mth::RAD_TO_DEG,
                                                         mSwimAmount)
                                  * Mth::DEG_TO_RAD;
                mRightArm->mXRot = Mth::Lerp(mRightArm->mXRot, drop, strokeAmount);
                mLeftArm->mYRot = Mth::lerpRotateDegrees(mLeftArm->mYRot * Mth::RAD_TO_DEG,
                                                         3.1415927f * Mth::RAD_TO_DEG, mSwimAmount)
                                  * Mth::DEG_TO_RAD;
                mRightArm->mYRot = Mth::Lerp(mRightArm->mYRot, 3.1415927f, strokeAmount);
                mLeftArm->mZRot = Mth::lerpRotateDegrees(mLeftArm->mZRot * Mth::RAD_TO_DEG,
                                                         3.1415927f * Mth::RAD_TO_DEG, mSwimAmount)
                                  * Mth::DEG_TO_RAD;
                mRightArm->mZRot = Mth::Lerp(mRightArm->mZRot, 3.1415927f, strokeAmount);
            }
        }

        if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_LegsStill)) == 0)
        {
            float legRot = Mth::Lerp(mLeftLeg->mXRot,
                                     0.0f * Mth::DEG_TO_RAD + Mth::cos(limbSwing * 0.34f + 3.1415927f) * 0.9f / 3.0f,
                                     mSwimAmount);
            mLeftLeg->mXRot = legRot;

            if ((skinAdjustments.mAnimFlags & (1u << eAnimBit_LegsInSync)) == 0)
            {
                legRot = Mth::Lerp(mRightLeg->mXRot, 0.0f * Mth::DEG_TO_RAD + Mth::cos(limbSwing * 0.34f) * 0.9f / 3.0f,
                                   mSwimAmount);
            }

            mRightLeg->mXRot = legRot;
        }
    }

    Model::copyModelPart(mHead, mHat);

    if (mIsArmourModel)
    {
        switch (mArmourSlot->getEIndex())
        {
        case EquipmentSlot::eIndex_Head:
            mHead->mYRotOffset = skinAdjustments.mHelmetY;
            mHat->mYRotOffset = skinAdjustments.mHelmetY;
            break;

        case EquipmentSlot::eIndex_Chest:
            mBody->mYRotOffset = skinAdjustments.mChestY;
            mRightArm->mYRotOffset = skinAdjustments.mShoulder0Y;
            mLeftArm->mYRotOffset = skinAdjustments.mShoulder1Y;
            break;

        case EquipmentSlot::eIndex_Legs:
            mBody->mYRotOffset = skinAdjustments.mWaistY;
            mRightLeg->mYRotOffset = skinAdjustments.mPants0Y;
            mLeftLeg->mYRotOffset = skinAdjustments.mPants1Y;
            break;

        case EquipmentSlot::eIndex_Feet:
            mRightLeg->mYRotOffset = skinAdjustments.mBoot1Y;
            mLeftLeg->mYRotOffset = skinAdjustments.mBoot0Y;
            break;

        default:
            break;
        }
    }

    mHead->setYOffset(skinAdjustments.mHeadY);
    mHat->setYOffset(skinAdjustments.mHeadY);
    mBody->setYOffset(skinAdjustments.mBodyY);
    mRightArm->setYOffset(skinAdjustments.mArm0Y);
    mLeftArm->setYOffset(skinAdjustments.mArm1Y);
    mRightLeg->setYOffset(skinAdjustments.mLeg0Y);
    mLeftLeg->setYOffset(skinAdjustments.mLeg1Y);
}
