#include "game/client/render/ElytraModel.hpp"
#include "game/core/EntityType.hpp"

#include <math.h>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"

ElytraModel::ElytraModel(const ModelDefinition& modelDefinition)
{
    mOverrideAnim = false;

    mLeftWing = new ModelPartSkin(this, 22, 0, NULL);
    mLeftWing->addBox(-10.0f, 0.0f, 0.0f, 10, 20, 2, 1.0f);

    mRightWing = new ModelPartSkin(this, 22, 0, NULL);
    mRightWing->mMirror = true;
    mRightWing->addBox(0.0f, 0.0f, 0.0f, 10, 20, 2, 1.0f);

    float xRot = 15.0f * Mth::DEG_TO_RAD;
    float zRot = -15.0f * Mth::DEG_TO_RAD;
    float y = 0.0f;
    float yRot = 0.0f;

    mLeftWing->mX = 5.0f;
    mLeftWing->mY = y;
    mLeftWing->mXRot = xRot;
    mLeftWing->mYRot = yRot;
    mLeftWing->mZRot = zRot;

    mRightWing->mX = -mLeftWing->mX;
    mRightWing->mYRot = -mLeftWing->mYRot;
    mRightWing->mY = mLeftWing->mY;
    mRightWing->mXRot = mLeftWing->mXRot;
    mRightWing->mZRot = -mLeftWing->mZRot;

    compileSkinnedDrawlist();
}

void ElytraModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                         float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    GlStateManager::disableRescaleNormal();
    GlStateManager::disableCull();

    if (Entity::instanceof(entity, EntityType::LIVING_ENTITY)
        && boost::static_pointer_cast<LivingEntity>(entity)->isBaby())
    {
        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        GlStateManager::translatef(0.0f, 1.5f, -0.1f);

        mLeftWing->render(scale, useDisplayList, false);
        mRightWing->render(scale, useDisplayList, false);

        GlStateManager::popMatrix();

        callSkinnedDrawlist();
    }
    else
    {
        mLeftWing->render(scale, useDisplayList, false);
        mRightWing->render(scale, useDisplayList, false);

        callSkinnedDrawlist();
    }
}

void ElytraModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                            float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    Model::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, skinAdjustments);

    float xRot = 15.0f * Mth::DEG_TO_RAD;
    float zRot = -15.0f * Mth::DEG_TO_RAD;
    float y = 0.0f;
    float yRot = 0.0f;

    if (!mOverrideAnim && entity.get() != NULL && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
        && boost::dynamic_pointer_cast<LivingEntity>(entity)->isFallFlying())
    {
        float flyAmount = 1.0f;

        if (entity->mYd < 0.0)
        {
            Vec3* movement = Vec3::newTemp(entity->mXd, entity->mYd, entity->mZd)->normalize();

            flyAmount = 1.0f - (float)pow(-movement->y, 1.5);
        }

        xRot = flyAmount * (20.0f * Mth::DEG_TO_RAD) + (1.0f - flyAmount) * xRot;
        zRot = flyAmount * (-90.0f * Mth::DEG_TO_RAD) + (1.0f - flyAmount) * zRot;
    }
    else if (entity.get() != NULL && entity->isSneaking())
    {
        xRot = 40.0f * Mth::DEG_TO_RAD;
        zRot = -45.0f * Mth::DEG_TO_RAD;
        y = 3.0f;
        yRot = 5.0f * Mth::DEG_TO_RAD;
    }

    mLeftWing->mX = 5.0f;
    mLeftWing->mY = y;

    if (!mOverrideAnim && boost::dynamic_pointer_cast<AbstractClientPlayer>(entity))
    {
        boost::shared_ptr<AbstractClientPlayer> player = boost::dynamic_pointer_cast<AbstractClientPlayer>(entity);

        player->mElytraRotX += (xRot - player->mElytraRotX) * 0.1f;
        player->mElytraRotY += (yRot - player->mElytraRotY) * 0.1f;
        player->mElytraRotZ += (zRot - player->mElytraRotZ) * 0.1f;

        mLeftWing->mXRot = player->mElytraRotX;
        mLeftWing->mYRot = player->mElytraRotY;
        mLeftWing->mZRot = player->mElytraRotZ;
    }
    else
    {
        mLeftWing->mXRot = xRot;
        mLeftWing->mYRot = yRot;
        mLeftWing->mZRot = zRot;
    }

    mRightWing->mX = -mLeftWing->mX;
    mRightWing->mYRot = -mLeftWing->mYRot;
    mRightWing->mY = mLeftWing->mY;
    mRightWing->mXRot = mLeftWing->mXRot;
    mRightWing->mZRot = -mLeftWing->mZRot;
}

void ElytraModel::overrideAnim(float flyAmount, float netHeadYaw, float headPitch, float scale)
{
    Model::setupAnim(0.0f, 0.0f, 0.0f, netHeadYaw, headPitch, scale, boost::shared_ptr<Entity>(), _SkinAdjustments());

    float x = 5.0f;
    float y = 0.0f;
    float xRot = flyAmount * (20.0f * Mth::DEG_TO_RAD) + (1.0f - flyAmount) * (15.0f * Mth::DEG_TO_RAD);
    float yRot = 0.0f;
    float zRot = flyAmount * (-90.0f * Mth::DEG_TO_RAD) + (1.0f - flyAmount) * (-15.0f * Mth::DEG_TO_RAD);

    ModelPart* leftWing = mLeftWing;
    ModelPart* rightWing = mRightWing;

    leftWing->mX = x;
    leftWing->mY = y;
    leftWing->mXRot = xRot;
    leftWing->mYRot = yRot;
    leftWing->mZRot = zRot;

    rightWing->mX = -x;
    rightWing->mY = y;
    rightWing->mXRot = xRot;
    rightWing->mYRot = -yRot;
    rightWing->mZRot = -zRot;
}
