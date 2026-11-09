#include "game/client/render/PlayerModel.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"
#include "vendor/SKIN_BOX.hpp"
#include "vendor/_SkinAdjustments.hpp"

PlayerModel::PlayerModel(float scale, bool slimArms, bool is64x64)
    : HumanoidModel(ModelDefinition(), scale, 0.0f, 64, is64x64 ? 64 : 32, false, !is64x64, false)
{
    mIs64x64 = is64x64;
    mSlimArms = slimArms;

    mEar = new ModelPart(this, 24, 0);
    mEar->addBox(-3.0f, -6.0f, -1.0f, 6, 6, 1, scale);

    mCloak = new ModelPart(this, 0, 0);
    mCloak->setTexSize(64, 32);
    mCloak->addBox(-5.0f, 0.0f, -1.0f, 10, 16, 1, scale);

    mEar->compile(0.0625f);
    mCloak->compile(0.0625f);

    if (is64x64)
    {
        mHead = new ModelPart(this, 0, 0);
        mHead->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale);
        mHead->setPos(0.0f, 0.0f, 0.0f);

        mHat = new ModelPart(this, 32, 0);
        mHat->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, scale + HAT_INFLATE);
        mHat->setPos(0.0f, 0.0f, 0.0f);

        mBody = new ModelPart(this, 16, 16);
        mBody->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, scale);
        mBody->setPos(0.0f, 0.0f, 0.0f);

        mLeftArm = new ModelPart(this, 32, 48);

        if (slimArms)
        {
            mLeftArm->addBox(-1.0f, -2.0f, -2.0f, 3, 12, 4, scale);
            mLeftArm->setPos(5.0f, 2.0f, 0.0f);

            mRightArm = new ModelPart(this, 40, 16);
            mRightArm->addBox(-2.0f, -2.0f, -2.0f, 3, 12, 4, scale);
            mRightArm->setPos(-5.0f, 2.0f, 0.0f);

            mLeftSleeve = new ModelPart(this, 48, 48);
            mLeftSleeve->addBox(-1.0f, -2.0f, -2.0f, 3, 12, 4, scale + OVERLAY_INFLATE);
            mLeftSleeve->setPos(5.0f, 2.0f, 0.0f);

            mRightSleeve = new ModelPart(this, 40, 32);
            mRightSleeve->addBox(-2.0f, -2.0f, -2.0f, 3, 12, 4, scale + OVERLAY_INFLATE);
            mRightSleeve->setPos(-5.0f, 2.0f, 10.0f);
        }
        else
        {
            mLeftArm->addBox(-1.0f, -2.0f, -2.0f, 4, 12, 4, scale);
            mLeftArm->setPos(5.0f, 2.0f, 0.0f);

            mRightArm = new ModelPart(this, 40, 16);
            mRightArm->addBox(-3.0f, -2.0f, -2.0f, 4, 12, 4, scale);
            mRightArm->setPos(-5.0f, 2.0f, 0.0f);

            mLeftSleeve = new ModelPart(this, 48, 48);
            mLeftSleeve->addBox(-1.0f, -2.0f, -2.0f, 4, 12, 4, scale + OVERLAY_INFLATE);
            mLeftSleeve->setPos(5.0f, 2.0f, 0.0f);

            mRightSleeve = new ModelPart(this, 40, 32);
            mRightSleeve->addBox(-3.0f, -2.0f, -2.0f, 4, 12, 4, scale + OVERLAY_INFLATE);
            mRightSleeve->setPos(-5.0f, 2.0f, 10.0f);
        }

        mLeftLeg = new ModelPart(this, 16, 48);
        mLeftLeg->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
        mLeftLeg->setPos(1.9f, 12.0f, 0.0f);

        mRightLeg = new ModelPart(this, 0, 16);
        mRightLeg->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale);
        mRightLeg->setPos(-1.9f, 12.0f, 0.0f);

        mLeftPants = new ModelPart(this, 0, 48);
        mLeftPants->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale + OVERLAY_INFLATE);
        mLeftPants->setPos(1.9f, 12.0f, 0.0f);

        mRightPants = new ModelPart(this, 0, 32);
        mRightPants->addBox(-2.0f, 0.0f, -2.0f, 4, 12, 4, scale + OVERLAY_INFLATE);
        mRightPants->setPos(-1.9f, 12.0f, 0.0f);

        mJacket = new ModelPart(this, 16, 32);
        mJacket->addBox(-4.0f, 0.0f, -2.0f, 8, 12, 4, scale + OVERLAY_INFLATE);
        mJacket->setPos(0.0f, 0.0f, 0.0f);

        mLeftShoulder = new ModelPart(this, 0, 0);
        mRightShoulder = new ModelPart(this, 0, 0);
        mLeftLegging = new ModelPart(this, 0, 0);
        mRightLegging = new ModelPart(this, 0, 0);
        mChest = new ModelPart(this, 0, 0);
        mWaist = new ModelPart(this, 0, 0);
        mLeftShin = new ModelPart(this, 0, 0);
        mRightShin = new ModelPart(this, 0, 0);
        mLeftBoot = new ModelPart(this, 0, 0);
        mRightBoot = new ModelPart(this, 0, 0);
    }

    compileSkinnedDrawlist();
}

void PlayerModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                         float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupBoneMatrices(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, useDisplayList);

    if (mIs64x64)
    {
        GlStateManager::pushMatrix();

        if (mYoung)
        {
            GlStateManager::scalef(0.5f, 0.5f, 0.5f);
            GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);

            mLeftPants->render(scale, useDisplayList, false);
            mRightPants->render(scale, useDisplayList, false);
            mLeftSleeve->render(scale, useDisplayList, false);
            mRightSleeve->render(scale, useDisplayList, false);
            mJacket->render(scale, useDisplayList, false);

            mLeftLegging->render(scale, useDisplayList, true);
            mRightLegging->render(scale, useDisplayList, true);
            mLeftBoot->render(scale, useDisplayList, true);
            mRightBoot->render(scale, useDisplayList, true);
            mLeftShin->render(scale, useDisplayList, true);
            mRightShin->render(scale, useDisplayList, true);
            mChest->render(scale, useDisplayList, true);
            mLeftShoulder->render(scale, useDisplayList, true);
            mRightShoulder->render(scale, useDisplayList, true);
            mWaist->render(scale, useDisplayList, true);
        }
        else
        {
            bool fallFlying = false;
            if (entity)
            {
                fallFlying = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
                             && boost::static_pointer_cast<LivingEntity>(entity)->getFallFlyingTicks() > 4;
            }

            if (entity && entity->isSneaking() && !fallFlying)
            {
                GlStateManager::translatef(0.0f, 0.2f, 0.0f);
            }

            mLeftPants->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftPants));
            mRightPants->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightPants));
            mLeftSleeve->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftSleeve));
            mRightSleeve->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightSleeve));
            mJacket->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_Jacket));

            boost::shared_ptr<Player> player;

            if (entity)
            {
                player = boost::static_pointer_cast<Player>(entity);
            }

            bool hasChestArmour = player && !player->getItemSlot(EquipmentSlot::CHEST)->isEmpty();
            bool hasLegsArmour = player && !player->getItemSlot(EquipmentSlot::LEGS)->isEmpty();
            bool hasFeetArmour = player && !player->getItemSlot(EquipmentSlot::FEET)->isEmpty();

            mLeftLegging->mShow = !hasLegsArmour;
            mRightLegging->mShow = !hasLegsArmour;
            mLeftLegging->render(scale, useDisplayList, false);
            mRightLegging->render(scale, useDisplayList, false);

            mLeftBoot->mShow = !hasFeetArmour;
            mRightBoot->mShow = !hasFeetArmour;
            mLeftBoot->render(scale, useDisplayList, false);
            mRightBoot->render(scale, useDisplayList, false);

            mLeftShin->mShow = !(hasLegsArmour || hasFeetArmour);
            mRightShin->mShow = !(hasLegsArmour || hasFeetArmour);
            mLeftShin->render(scale, useDisplayList, false);
            mRightShin->render(scale, useDisplayList, false);

            mChest->mShow = !hasChestArmour;
            mLeftShoulder->mShow = !hasChestArmour;
            mRightShoulder->mShow = !hasChestArmour;
            mChest->render(scale, useDisplayList, false);
            mLeftShoulder->render(scale, useDisplayList, false);
            mRightShoulder->render(scale, useDisplayList, false);

            mWaist->mShow = !(hasChestArmour || hasLegsArmour);
            mWaist->render(scale, useDisplayList, false);
        }

        GlStateManager::popMatrix();
    }

    callSkinnedDrawlist();
}

void PlayerModel::renderEars(float scale)
{
    Model::copyModelPart(mHead, mEar);

    mEar->mX = 0.0f;
    mEar->mY = 0.0f;
    mEar->render(scale, true, false);
}

void PlayerModel::renderCloak(float scale)
{
    mCloak->render(scale, true, false);
}

void PlayerModel::copyModelPartsExceptHead()
{
    Model::copyModelPart(mLeftLeg, mLeftPants);
    Model::copyModelPart(mRightLeg, mRightPants);
    Model::copyModelPart(mLeftArm, mLeftSleeve);
    Model::copyModelPart(mRightArm, mRightSleeve);
    Model::copyModelPart(mBody, mJacket);

    Model::copyModelPart(mLeftLeg, mLeftLegging);
    Model::copyModelPart(mRightLeg, mRightLegging);
    Model::copyModelPart(mLeftLeg, mLeftShin);
    Model::copyModelPart(mRightLeg, mRightShin);
    Model::copyModelPart(mLeftLeg, mLeftBoot);
    Model::copyModelPart(mRightLeg, mRightBoot);
    Model::copyModelPart(mBody, mChest);
    Model::copyModelPart(mBody, mWaist);
    Model::copyModelPart(mLeftArm, mLeftShoulder);
    Model::copyModelPart(mRightArm, mRightShoulder);
}

void PlayerModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                            float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    HumanoidModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                             skinAdjustments);

    if (mIs64x64)
    {
        copyModelPartsExceptHead();

        bool fallFlying = false;
        if (entity)
        {
            fallFlying = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
                         && boost::static_pointer_cast<LivingEntity>(entity)->getFallFlyingTicks() > 4;
        }

        if (entity && entity->isSneaking() && !fallFlying)
        {
            mCloak->mY = 2.0f;
        }
        else
        {
            mCloak->mY = 0.0f;
        }
    }
}

void PlayerModel::render(HumanoidModel* model, float scale, bool useDisplayList)
{
    setupBoneMatrices(model, scale, useDisplayList);

    if (mIs64x64)
    {
        copyModelPartsExceptHead();

        mLeftPants->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftPants));
        mRightPants->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightPants));
        mLeftSleeve->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftSleeve));
        mRightSleeve->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightSleeve));
        mJacket->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_Jacket));

        mLeftShoulder->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftSleeve));
        mRightShoulder->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightSleeve));
        mLeftLegging->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftPants));
        mRightLegging->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightPants));
        mChest->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_Jacket));
        mWaist->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_Jacket));
        mLeftShin->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftPants));
        mRightShin->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightPants));
        mLeftBoot->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_LeftPants));
        mRightBoot->render(scale, useDisplayList, IsBodyPartDisabled(eAnimBit_RightPants));
    }

    callSkinnedDrawlist();
}

void PlayerModel::setAllVisible(bool visible)
{
    HumanoidModel::setAllVisible(visible);

    if (mIs64x64)
    {
        mLeftSleeve->mShow = visible;
        mRightSleeve->mShow = visible;
        mLeftPants->mShow = visible;
        mRightPants->mShow = visible;
        mJacket->mShow = visible;
        mCloak->mShow = visible;
        mEar->mShow = visible;
        mLeftShoulder->mShow = visible;
        mRightShoulder->mShow = visible;
        mLeftLegging->mShow = visible;
        mRightLegging->mShow = visible;
        mChest->mShow = visible;
        mWaist->mShow = visible;
        mLeftShin->mShow = visible;
        mRightShin->mShow = visible;
        mLeftBoot->mShow = visible;
        mRightBoot->mShow = visible;
    }
}

void PlayerModel::translateToHandItem(float scale, const HumanoidArm* arm)
{
    ModelPart* part = getArm(arm);
    float y = part->mY;

    part->mY += mSkinAdjustments.mTool0Y;

    if (mSlimArms)
    {
        float offset = 0.5f * (arm == HumanoidArm::RIGHT ? 1.0f : -1.0f);

        part->mX += offset;
        part->translateTo(scale);
        part->mX -= offset;
    }
    else
    {
        part->translateTo(scale);
    }

    part->mY = y;
}

void PlayerModel::clearAllOffsets()
{
    mHead->clearAllOffsets();
    mHat->clearAllOffsets();
    mBody->clearAllOffsets();
    mRightArm->clearAllOffsets();
    mLeftArm->clearAllOffsets();
    mRightLeg->clearAllOffsets();
    mLeftLeg->clearAllOffsets();

    if (mIs64x64)
    {
        mLeftSleeve->clearAllOffsets();
        mRightSleeve->clearAllOffsets();
        mLeftPants->clearAllOffsets();
        mRightPants->clearAllOffsets();
        mJacket->clearAllOffsets();
    }
}

void PlayerModel::SetSkinAdjustments(_SkinAdjustments skinAdjustments)
{
    mSkinAdjustments = skinAdjustments;
}

ModelPart* PlayerModel::AddOrRetrievePart(SKIN_BOX* skinBox, unsigned int skinId)
{
    ModelPart* parent = NULL;
    float delta = 0.0f;

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

    case 7:
        parent = mHat;
        delta = HAT_INFLATE;
        break;

    case 8:
        parent = mJacket;
        delta = OVERLAY_INFLATE;
        break;

    case 9:
        parent = mRightSleeve;
        delta = OVERLAY_INFLATE;
        break;

    case 10:
        parent = mLeftSleeve;
        delta = OVERLAY_INFLATE;
        break;

    case 11:
        parent = mRightPants;
        delta = OVERLAY_INFLATE;
        break;

    case 12:
        parent = mLeftPants;
        delta = OVERLAY_INFLATE;
        break;

    case 13:
        parent = mBody;
        delta = OVERLAY_INFLATE;
        break;

    case 14:
        parent = mRightLegging;
        break;

    case 15:
        parent = mLeftLegging;
        break;

    case 16:
        parent = mRightShin;
        break;

    case 17:
        parent = mLeftShin;
        break;

    case 18:
        parent = mRightBoot;
        break;

    case 19:
        parent = mLeftBoot;
        break;

    case 20:
        parent = mLeftShoulder;
        break;

    case 21:
        parent = mRightShoulder;
        break;

    case 22:
        parent = mChest;
        break;

    case 23:
        parent = mWaist;
        break;
    }

    ModelPart* part = parent->retrieveChildWithSkinIdAndArmourFlags(skinId, skinBox->armourFlags);

    if (part == NULL)
    {
        part = new ModelPart(this);
        part->mSkinId = skinId;
        part->mArmourFlags = skinBox->armourFlags;
        part->mSkinBoxType = skinBox->id;
        part->mShow = false;

        parent->addChild(part);
    }

    part->texOffs((int)skinBox->xTexOffs, (int)skinBox->yTexOffs);
    part->mMirror = skinBox->mirror != 0;

    if (mSkinAdjustments.mXMLVersion >= 3)
    {
        part->addBox(skinBox->x, skinBox->y, skinBox->z, (int)skinBox->width, (int)skinBox->height, (int)skinBox->depth,
                     skinBox->delta);
    }
    else if (mSkinAdjustments.mXMLVersion != 0)
    {
        part->addBox(skinBox->x, skinBox->y, skinBox->z, (int)skinBox->width, (int)skinBox->height, (int)skinBox->depth,
                     delta);
    }
    else
    {
        part->addHumanoidBox(skinBox->x, skinBox->y, skinBox->z, (int)skinBox->width, (int)skinBox->height,
                             (int)skinBox->depth, 0.0f);
    }

    return part;
}
