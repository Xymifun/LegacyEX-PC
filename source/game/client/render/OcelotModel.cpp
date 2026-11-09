#include "game/client/render/OcelotModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Ocelot.hpp"

static float OCELOT_HEAD_Y = 15.0f;
static float OCELOT_HEAD_Z = -9.0f;
static float OCELOT_BODY_Y = 12.0f;
static float OCELOT_BODY_Z = -10.0f;
static float OCELOT_TAIL1_Y = 15.0f;
static float OCELOT_TAIL1_Z = 8.0f;
static float OCELOT_TAIL2_Y = 20.0f;
static float OCELOT_TAIL2_Z = 14.0f;
static float OCELOT_BACK_LEG_Y = 18.0f;
static float OCELOT_BACK_LEG_Z = 5.0f;
static float OCELOT_FRONT_LEG_Y = 13.8f;
static float OCELOT_FRONT_LEG_Z = -5.0f;

OcelotModel::OcelotModel()
{
    TexturePack* texturePack = Minecraft::GetInstance()->mTexturePackRepository->defaultTexturePack;
    LoadOcelotModelDefinition(*texturePack->GetCustomModel(L"ocelot"));
}

OcelotModel::OcelotModel(const ModelDefinition& modelDefinition)
{
    LoadOcelotModelDefinition(modelDefinition);
}

void OcelotModel::LoadOcelotModelDefinition(const ModelDefinition& modelDefinition)
{
    mPose = 1;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mBackLegL = new ModelPartSkin(this, L"backLegL", modelDefinition, NULL, 0.0f, NULL);
        mBackLegR = new ModelPartSkin(this, L"backLegR", modelDefinition, NULL, 0.0f, NULL);
        mFrontLegL = new ModelPartSkin(this, L"frontLegL", modelDefinition, NULL, 0.0f, NULL);
        mFrontLegR = new ModelPartSkin(this, L"frontLegR", modelDefinition, NULL, 0.0f, NULL);
        mTail1 = new ModelPartSkin(this, L"tail1", modelDefinition, NULL, 0.0f, NULL);
        mTail2 = new ModelPartSkin(this, L"tail2", modelDefinition, NULL, 0.0f, NULL);
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
    }

    compileSkinnedDrawlist();
    builder->useDefaultShaders();
}

void OcelotModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                         float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    if (mYoung)
    {
        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.75f, 0.75f, 0.75f);
        GlStateManager::translatef(0.0f, 10.0f * scale, 4.0f * scale);
        mHead->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
        mBody->render(scale, useDisplayList, false);
        mBackLegL->render(scale, useDisplayList, false);
        mBackLegR->render(scale, useDisplayList, false);
        mFrontLegL->render(scale, useDisplayList, false);
        mFrontLegR->render(scale, useDisplayList, false);
        mTail1->render(scale, useDisplayList, false);
        mTail2->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
        callSkinnedDrawlist();
    }
    else
    {
        mHead->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mTail1->render(scale, useDisplayList, false);
        mTail2->render(scale, useDisplayList, false);
        mBackLegL->render(scale, useDisplayList, false);
        mBackLegR->render(scale, useDisplayList, false);
        mFrontLegL->render(scale, useDisplayList, false);
        mFrontLegR->render(scale, useDisplayList, false);
        callSkinnedDrawlist();
    }
}

void OcelotModel::render(OcelotModel* model, float scale, bool useDisplayList)
{
    mHead->mYRot = model->mHead->mYRot;
    mHead->mXRot = model->mHead->mXRot;
    mHead->mY = model->mHead->mY;
    mHead->mX = model->mHead->mX;
    mBody->mYRot = model->mBody->mYRot;
    mBody->mXRot = model->mBody->mXRot;
    mTail1->mYRot = model->mBody->mYRot;
    mTail1->mY = model->mBody->mY;
    mTail1->mX = model->mBody->mX;
    mTail1->render(scale, useDisplayList, false);
    mTail2->mYRot = model->mBody->mYRot;
    mTail2->mY = model->mBody->mY;
    mTail2->mX = model->mBody->mX;
    mTail2->render(scale, useDisplayList, false);
    mBackLegL->mXRot = model->mBackLegL->mXRot;
    mBackLegR->mXRot = model->mBackLegR->mXRot;
    mBackLegL->render(scale, useDisplayList, false);
    mBackLegR->render(scale, useDisplayList, false);
    mFrontLegL->mXRot = model->mFrontLegL->mXRot;
    mFrontLegR->mXRot = model->mFrontLegR->mXRot;
    mFrontLegL->render(scale, useDisplayList, false);
    mFrontLegR->render(scale, useDisplayList, false);
    mHead->render(scale, useDisplayList, false);
    mBody->render(scale, useDisplayList, false);
}

void OcelotModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                            float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    int pose = mPose;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;

    if (pose != 3)
    {
        mBackLegL->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.0f * limbSwingAmount;
        if (pose == 2)
        {
            mBackLegR->mXRot = Mth::cos(limbSwing * 0.6662f + 0.3f) * 1.0f * limbSwingAmount;
            mFrontLegL->mXRot = Mth::cos(limbSwing * 0.6662f + 3.4415927f) * 1.0f * limbSwingAmount;
            mFrontLegR->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.0f * limbSwingAmount;
            mTail2->mXRot = 0.31415927f * Mth::cos(limbSwing) * limbSwingAmount + 1.7278761f;
        }
        else
        {
            mBackLegR->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.0f * limbSwingAmount;
            mFrontLegL->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.0f * limbSwingAmount;
            mFrontLegR->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.0f * limbSwingAmount;
            if (mPose == 1)
                mTail2->mXRot = 0.7853982f * Mth::cos(limbSwing) * limbSwingAmount + 1.7278761f;
            else
                mTail2->mXRot = 0.47123894f * Mth::cos(limbSwing) * limbSwingAmount + 1.7278761f;
        }
    }
}

void OcelotModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                  float partialTicks)
{
    boost::shared_ptr<Ocelot> ocelot = boost::dynamic_pointer_cast<Ocelot>(entity);

    mBody->mY = OCELOT_BODY_Y;
    mBody->mZ = OCELOT_BODY_Z;
    mHead->mY = OCELOT_HEAD_Y;
    mHead->mZ = OCELOT_HEAD_Z;
    mTail1->mY = OCELOT_TAIL1_Y;
    mTail1->mZ = OCELOT_TAIL1_Z;
    mTail2->mY = OCELOT_TAIL2_Y;
    mTail2->mZ = OCELOT_TAIL2_Z;
    mFrontLegL->mY = mFrontLegR->mY = OCELOT_FRONT_LEG_Y;
    mFrontLegL->mZ = mFrontLegR->mZ = OCELOT_FRONT_LEG_Z;
    mBackLegL->mY = mBackLegR->mY = OCELOT_BACK_LEG_Y;
    mBackLegL->mZ = mBackLegR->mZ = OCELOT_BACK_LEG_Z;
    mTail1->mXRot = 0.9f;
    mBody->mXRot = 0.0f;

    if (ocelot->isSneaking())
    {
        mBody->mY += 1.0f;
        mHead->mY += 2.0f;
        mTail1->mY += 1.0f;
        mTail2->mY += -4.0f;
        mTail2->mZ += 2.0f;
        mTail1->mXRot = 1.5707964f;
        mTail2->mXRot = 1.5707964f;
        mPose = 0;
    }
    else if (ocelot->isSprinting())
    {
        mTail2->mY = mTail1->mY;
        mTail2->mZ += 2.0f;
        mTail1->mXRot = 1.5707964f;
        mTail2->mXRot = 1.5707964f;
        mPose = 2;
    }
    else if (ocelot->isSitting())
    {
        mBody->mXRot = -45.0f * Mth::DEG_TO_RAD;
        mBody->mY += -4.0f;
        mBody->mZ += 5.0f;
        mHead->mY += -3.3f;
        mHead->mZ += 1.0f;
        mTail1->mY += 8.0f;
        mTail1->mZ += -2.0f;
        mTail2->mY += 2.0f;
        mTail2->mZ += -0.8f;
        mTail1->mXRot = 1.7278761f;
        mTail2->mXRot = 2.670354f;
        mFrontLegL->mXRot = -0.15707964f;
        mFrontLegL->mY = OCELOT_FRONT_LEG_Y + 2.0f;
        mFrontLegL->mZ = -7.0f;
        mFrontLegR->mXRot = -0.15707964f;
        mFrontLegR->mY = OCELOT_FRONT_LEG_Y + 2.0f;
        mFrontLegR->mZ = -7.0f;
        mBackLegL->mXRot = -1.5707964f;
        mBackLegL->mY = OCELOT_BACK_LEG_Y + 3.0f;
        mBackLegL->mZ = OCELOT_BACK_LEG_Z - 4.0f;
        mBackLegR->mXRot = -1.5707964f;
        mBackLegR->mY = OCELOT_BACK_LEG_Y + 3.0f;
        mBackLegR->mZ = OCELOT_BACK_LEG_Z - 4.0f;
        mPose = 3;
    }
    else
    {
        mPose = 1;
    }
}
