#include "game/client/render/DragonModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/MaterialShaders.hpp"
#include "game/world/block/MaterialTable.hpp"

DragonModel::DragonModel(const ModelDefinition& modelDefinition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    mHasLegs = true;
    mHasNamedNeckParts = false;

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;

        mHead = new ModelPart(this, L"head", modelDefinition, NULL, 0.0f);
        mJaw = new ModelPart(this, L"jaw", modelDefinition, mHead, 0.0f);

        if (modelDefinition.hasModelPart(L"neck1"))
        {
            mHasNamedNeckParts = true;

            mNeck[0] = new ModelPart(this, L"neck1", modelDefinition, NULL, 0.0f);
            mNeck[1] = new ModelPart(this, L"neck2", modelDefinition, NULL, 0.0f);
            mNeck[2] = new ModelPart(this, L"neck3", modelDefinition, NULL, 0.0f);
            mNeck[3] = new ModelPart(this, L"neck4", modelDefinition, NULL, 0.0f);
            mNeck[4] = new ModelPart(this, L"neck5", modelDefinition, NULL, 0.0f);

            mTail[0] = new ModelPart(this, L"tail1", modelDefinition, NULL, 0.0f);
            mTail[1] = new ModelPart(this, L"tail2", modelDefinition, NULL, 0.0f);
            mTail[2] = new ModelPart(this, L"tail3", modelDefinition, NULL, 0.0f);
            mTail[3] = new ModelPart(this, L"tail4", modelDefinition, NULL, 0.0f);
            mTail[4] = new ModelPart(this, L"tail5", modelDefinition, NULL, 0.0f);
            mTail[5] = new ModelPart(this, L"tail6", modelDefinition, NULL, 0.0f);
            mTail[6] = new ModelPart(this, L"tail7", modelDefinition, NULL, 0.0f);
            mTail[7] = new ModelPart(this, L"tail8", modelDefinition, NULL, 0.0f);
            mTail[8] = new ModelPart(this, L"tail9", modelDefinition, NULL, 0.0f);
            mTail[9] = new ModelPart(this, L"tail10", modelDefinition, NULL, 0.0f);
            mTail[10] = new ModelPart(this, L"tail11", modelDefinition, NULL, 0.0f);
            mTail[11] = new ModelPart(this, L"tail12", modelDefinition, NULL, 0.0f);
        }
        else
        {
            for (int i = 0; i < 5; i++)
            {
                mNeck[i] = new ModelPart(this, L"neck", modelDefinition, NULL, 0.0f);
            }

            for (int i = 0; i < 12; i++)
            {
                mTail[i] = new ModelPart(this, L"neck", modelDefinition, NULL, 0.0f);
            }
        }

        mBody = new ModelPart(this, L"body", modelDefinition, NULL, 0.0f);

        mWing = new ModelPart(this, L"wing", modelDefinition, NULL, 0.0f);
        mWingTip = new ModelPart(this, L"wingtip", modelDefinition, mWing, 0.0f);

        mWing1 = new ModelPart(this, L"wing1", modelDefinition, NULL, 0.0f);
        mWingTip1 = new ModelPart(this, L"wingtip1", modelDefinition, mWing1, 0.0f);

        if (modelDefinition.hasModelPart(L"rearleg"))
        {
            mRearLeg = new ModelPart(this, L"rearleg", modelDefinition, NULL, 0.0f);
            mRearLegTip = new ModelPart(this, L"rearlegtip", modelDefinition, mRearLeg, 0.0f);

            mRearLeg1 = new ModelPart(this, L"rearleg1", modelDefinition, NULL, 0.0f);
            mRearLegTip1 = new ModelPart(this, L"rearlegtip1", modelDefinition, mRearLeg1, 0.0f);

            mFrontLeg = new ModelPart(this, L"frontleg", modelDefinition, NULL, 0.0f);
            mFrontLegTip = new ModelPart(this, L"frontlegtip", modelDefinition, mFrontLeg, 0.0f);

            mFrontLeg1 = new ModelPart(this, L"frontleg1", modelDefinition, NULL, 0.0f);
            mFrontLegTip1 = new ModelPart(this, L"frontlegtip1", modelDefinition, mFrontLeg1, 0.0f);

            mRearFoot = new ModelPart(this, L"rearfoot", modelDefinition, mRearLegTip, 0.0f);
            mFrontFoot = new ModelPart(this, L"frontfoot", modelDefinition, mFrontLegTip, 0.0f);

            mRearFoot1 = new ModelPart(this, L"rearfoot1", modelDefinition, mRearLegTip1, 0.0f);
            mFrontFoot1 = new ModelPart(this, L"frontfoot1", modelDefinition, mFrontLegTip1, 0.0f);
        }
        else
        {
            mHasLegs = false;
        }

        mWingYRot = mWing->mYRot;

        if (mHasNamedNeckParts || !mHasOverriddenParts)
        {
            mWing1YRot = mWing1->mYRot;
        }
        else
        {
            mWing1YRot = mWing->mYRot;
        }

        MaterialShaders shaders = Minecraft::GetInstance()->getMaterialTable()->getMaterialShadersFor(
            eMinecraftEntityMaterial_EnderDragon);
        builder->setVertexAndPixelShader(C4JRender::eVertexType_Default, shaders.mPixelShaderType);

        mHead->compile(0.0625f);

        builder->useDefaultShaders();

        compileSkinnedDrawlist();
    }
    else
    {
        mTexWidth = 256;
        mTexHeight = 256;

        mWingYRot = 0.25f;
        mWing1YRot = 0.25f;

        setMapTex(L"head.upperlip", 176, 44);
        setMapTex(L"head.upperhead", 112, 30);
        setMapTex(L"head.scale", 0, 0);
        setMapTex(L"head.nostril", 112, 0);
        setMapTex(L"jaw.jaw", 176, 65);
        setMapTex(L"neck.box", 192, 104);
        setMapTex(L"neck.scale", 48, 0);
        setMapTex(L"body.body", 0, 0);
        setMapTex(L"body.scale", 220, 53);
        setMapTex(L"wing.skin", -56, 88);
        setMapTex(L"wing.bone", 112, 88);
        setMapTex(L"wingtip.skin", -56, 144);
        setMapTex(L"wingtip.bone", 112, 136);
        setMapTex(L"rearleg.main", 0, 0);
        setMapTex(L"rearfoot.main", 112, 0);
        setMapTex(L"rearlegtip.main", 196, 0);
        setMapTex(L"frontleg.main", 112, 104);
        setMapTex(L"frontfoot.main", 144, 104);
        setMapTex(L"frontlegtip.main", 226, 138);

        mHead = new ModelPart(this, L"head");
        mHead->addBox(L"upperlip", -6.0f, -1.0f, -24.0f, 12, 5, 16);
        mHead->addBox(L"upperhead", -8.0f, -8.0f, -10.0f, 16, 16, 16);
        mHead->mMirror = true;
        mHead->addBox(L"scale", -5.0f, -12.0f, -4.0f, 2, 4, 6);
        mHead->addBox(L"nostril", -5.0f, -3.0f, -22.0f, 2, 2, 4);
        mHead->mMirror = false;
        mHead->addBox(L"scale", 3.0f, -12.0f, -4.0f, 2, 4, 6);
        mHead->addBox(L"nostril", 3.0f, -3.0f, -22.0f, 2, 2, 4);

        mJaw = new ModelPart(this, L"jaw");
        mJaw->setPos(0.0f, 4.0f, -8.0f);
        mJaw->addBox(L"jaw", -6.0f, 0.0f, -16.0f, 12, 4, 16);

        mHead->addChild(mJaw);

        for (int i = 0; i < 5; i++)
        {
            mNeck[i] = new ModelPart(this, L"neck");
            mNeck[i]->addBox(L"box", -5.0f, -5.0f, -5.0f, 10, 10, 10);
            mNeck[i]->addBox(L"scale", -1.0f, -9.0f, -3.0f, 2, 4, 6);
        }

        for (int i = 0; i < 12; i++)
        {
            mTail[i] = new ModelPart(this, L"neck");
            mTail[i]->addBox(L"box", -5.0f, -5.0f, -5.0f, 10, 10, 10);
            mTail[i]->addBox(L"scale", -1.0f, -9.0f, -3.0f, 2, 4, 6);
        }

        mBody = new ModelPart(this, L"body");
        mBody->setPos(0.0f, 4.0f, 8.0f);
        mBody->addBox(L"body", -12.0f, 0.0f, -16.0f, 24, 24, 64);
        mBody->addBox(L"scale", -1.0f, -6.0f, -10.0f, 2, 6, 12);
        mBody->addBox(L"scale", -1.0f, -6.0f, 10.0f, 2, 6, 12);
        mBody->addBox(L"scale", -1.0f, -6.0f, 30.0f, 2, 6, 12);

        mWing = new ModelPart(this, L"wing");
        mWing->setPos(-12.0f, 5.0f, 2.0f);
        mWing->addBox(L"bone", -56.0f, -4.0f, -4.0f, 56, 8, 8);
        mWing->addBox(L"skin", -56.0f, 0.0f, 2.0f, 56, 0, 56);

        mWingTip = new ModelPart(this, L"wingtip");
        mWingTip->setPos(-56.0f, 0.0f, 0.0f);
        mWingTip->addBox(L"bone", -56.0f, -2.0f, -2.0f, 56, 4, 4);
        mWingTip->addBox(L"skin", -56.0f, 0.0f, 2.0f, 56, 0, 56);

        mWing->addChild(mWingTip);

        mFrontLeg = new ModelPart(this, L"frontleg");
        mFrontLeg->setPos(-12.0f, 20.0f, 2.0f);
        mFrontLeg->addBox(L"main", -4.0f, -4.0f, -4.0f, 8, 24, 8);

        mFrontLegTip = new ModelPart(this, L"frontlegtip");
        mFrontLegTip->setPos(0.0f, 20.0f, -1.0f);
        mFrontLegTip->addBox(L"main", -3.0f, -1.0f, -3.0f, 6, 24, 6);

        mFrontLeg->addChild(mFrontLegTip);

        mFrontFoot = new ModelPart(this, L"frontfoot");
        mFrontFoot->setPos(0.0f, 23.0f, 0.0f);
        mFrontFoot->addBox(L"main", -4.0f, 0.0f, -12.0f, 8, 4, 16);

        mFrontLegTip->addChild(mFrontFoot);

        mRearLeg = new ModelPart(this, L"rearleg");
        mRearLeg->setPos(-16.0f, 16.0f, 42.0f);
        mRearLeg->addBox(L"main", -8.0f, -4.0f, -8.0f, 16, 32, 16);

        mRearLegTip = new ModelPart(this, L"rearlegtip");
        mRearLegTip->setPos(0.0f, 32.0f, -4.0f);
        mRearLegTip->addBox(L"main", -6.0f, -2.0f, 0.0f, 12, 32, 12);

        mRearLeg->addChild(mRearLegTip);

        mRearFoot = new ModelPart(this, L"rearfoot");
        mRearFoot->setPos(0.0f, 31.0f, 4.0f);
        mRearFoot->addBox(L"main", -9.0f, 0.0f, -20.0f, 18, 6, 24);

        mRearLegTip->addChild(mRearFoot);

        mWing1 = new ModelPart(this, L"wing");
        mWing1->setPos(-12.0f, 5.0f, 2.0f);
        mWing1->addBox(L"bone", -56.0f, -4.0f, -4.0f, 56, 8, 8);
        mWing1->addBox(L"skin", -56.0f, 0.0f, 2.0f, 56, 0, 56);

        mWingTip1 = new ModelPart(this, L"wingtip");
        mWingTip1->setPos(-56.0f, 0.0f, 0.0f);
        mWingTip1->addBox(L"bone", -56.0f, -2.0f, -2.0f, 56, 4, 4);
        mWingTip1->addBox(L"skin", -56.0f, 0.0f, 2.0f, 56, 0, 56);

        mWing1->addChild(mWingTip1);

        mFrontLeg1 = new ModelPart(this, L"frontleg");
        mFrontLeg1->setPos(-12.0f, 20.0f, 2.0f);
        mFrontLeg1->addBox(L"main", -4.0f, -4.0f, -4.0f, 8, 24, 8);

        mFrontLegTip1 = new ModelPart(this, L"frontlegtip");
        mFrontLegTip1->setPos(0.0f, 20.0f, -1.0f);
        mFrontLegTip1->addBox(L"main", -3.0f, -1.0f, -3.0f, 6, 24, 6);

        mFrontLeg1->addChild(mFrontLegTip1);

        mFrontFoot1 = new ModelPart(this, L"frontfoot");
        mFrontFoot1->setPos(0.0f, 23.0f, 0.0f);
        mFrontFoot1->addBox(L"main", -4.0f, 0.0f, -12.0f, 8, 4, 16);

        mFrontLegTip1->addChild(mFrontFoot1);

        mRearLeg1 = new ModelPart(this, L"rearleg");
        mRearLeg1->setPos(-16.0f, 16.0f, 42.0f);
        mRearLeg1->addBox(L"main", -8.0f, -4.0f, -8.0f, 16, 32, 16);

        mRearLegTip1 = new ModelPart(this, L"rearlegtip");
        mRearLegTip1->setPos(0.0f, 32.0f, -4.0f);
        mRearLegTip1->addBox(L"main", -6.0f, -2.0f, 0.0f, 12, 32, 12);

        mRearLeg1->addChild(mRearLegTip1);

        mRearFoot1 = new ModelPart(this, L"rearfoot");
        mRearFoot1->setPos(0.0f, 31.0f, 4.0f);
        mRearFoot1->addBox(L"main", -9.0f, 0.0f, -20.0f, 18, 6, 24);

        mRearLegTip1->addChild(mRearFoot1);

        compileSkinnedDrawlist();
    }
}

void DragonModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                  float partialTicks)
{
    mPartialTicks = partialTicks;
}

float DragonModel::rotWrap(double angle)
{
    while (angle >= 180.0)
    {
        angle -= 360.0;
    }

    while (angle < -180.0)
    {
        angle += 360.0;
    }

    return (float)angle;
}

void DragonModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                         float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    GlStateManager::pushMatrix();

    boost::shared_ptr<EnderDragon> dragon = boost::dynamic_pointer_cast<EnderDragon>(entity);

    float flapTime = dragon->mOldFlapTime + (dragon->mFlapTime - dragon->mOldFlapTime) * mPartialTicks;

    mJaw->mXRot = (Mth::sin(flapTime * 6.2831855f) + 1.0f) * 0.2f;

    float bob = Mth::sin(flapTime * 6.2831855f - 1.0f) + 1.0f;
    bob = (bob * bob + bob * 2.0f) * 0.05f;

    GlStateManager::translatef(0.0f, bob - 2.0f, -3.0f);
    GlStateManager::rotatef(bob * 2.0f, 1.0f, 0.0f, 0.0f);

    double headOffsetValues[3];
    arrayWithLength<double> headOffsets(headOffsetValues, 3);
    dragon->getLatencyPos(headOffsets, 6, mPartialTicks);

    double neckBaseOffsetValues[3];
    double tailBaseOffsetValues[3];
    arrayWithLength<double> neckBaseOffsets(neckBaseOffsetValues, 3);
    arrayWithLength<double> tailBaseOffsets(tailBaseOffsetValues, 3);
    dragon->getLatencyPos(neckBaseOffsets, 5, mPartialTicks);
    dragon->getLatencyPos(tailBaseOffsets, 10, mPartialTicks);

    float bodyYawDiff = rotWrap(neckBaseOffsets.data[0] - tailBaseOffsets.data[0]);
    float bodyYaw = rotWrap(neckBaseOffsets.data[0] + bodyYawDiff * 0.5f);

    float flapAngle = flapTime * 6.2831855f;

    double partOffsetValues[3];
    arrayWithLength<double> partOffsets(partOffsetValues, 3);

    float x = 0.0f;
    float y = 20.0f;
    float z = -12.0f;

    for (int i = 0; i < 5; i++)
    {
        dragon->getLatencyPos(partOffsets, 5 - i, mPartialTicks);

        float wobble = Mth::cos((float)i * 0.45f + flapAngle) * 0.15f;

        mNeck[i]->mYRot = rotWrap(dragon->getHeadPartYRotDiff(i, headOffsets, partOffsets)) * Mth::DEG_TO_RAD * 1.5f;
        float neckPitch = (float)dragon->getHeadPartYOffset(i, headOffsets, partOffsets) * Mth::DEG_TO_RAD * 1.5f
                          * 5.0f;
        float neckXRot = wobble + neckPitch;
        ModelPart* neck = mNeck[i];
        neck->mXRot = neckXRot;
        neck->mZRot = -rotWrap(partOffsets.data[0] - bodyYaw) * Mth::DEG_TO_RAD * 1.5f;
        neck->mY = y;
        neck->mZ = z;
        neck->mX = x;

        y += Mth::sin(neckXRot) * 10.0f;
        z -= Mth::cos(mNeck[i]->mYRot) * Mth::cos(mNeck[i]->mXRot) * 10.0f;
        x -= Mth::sin(mNeck[i]->mYRot) * Mth::cos(mNeck[i]->mXRot) * 10.0f;

        mNeck[i]->render(scale, useDisplayList, false);
    }

    mHead->mY = y;
    mHead->mZ = z;
    mHead->mX = x;

    dragon->getLatencyPos(partOffsets, 0, mPartialTicks);

    mHead->mYRot = rotWrap(dragon->getHeadPartYRotDiff(6, headOffsets, partOffsets)) * Mth::DEG_TO_RAD;
    float headPitchRot = (float)dragon->getHeadPartYOffset(6, headOffsets, partOffsets) * Mth::DEG_TO_RAD * 1.5f * 5.0f;
    ModelPart* head = mHead;
    head->mXRot = headPitchRot;
    head->mZRot = -rotWrap(partOffsets.data[0] - bodyYaw) * Mth::DEG_TO_RAD;

    head->render(scale, useDisplayList, false);

    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(-bodyYawDiff * 1.5f, 0.0f, 0.0f, 1.0f);
    GlStateManager::translatef(0.0f, -1.0f, 0.0f);

    mBody->mZRot = 0.0f;
    mBody->render(scale, useDisplayList, false);

    GlStateManager::enableCull();

    float wingXRot = 0.125f - Mth::cos(flapAngle) * 0.2f;
    ModelPart* wing = mWing;
    wing->mXRot = wingXRot;
    wing->mYRot = mWingYRot;
    wing->mZRot = (Mth::sin(flapAngle) + 0.125f) * 0.8f;
    mWingTip->mZRot = -(Mth::sin(flapAngle + 2.0f) + 0.5f) * 0.75f;

    mWing->render(scale, useDisplayList, false);

    if (mHasLegs)
    {
        mRearLeg->mXRot = 1.0f + bob * 0.1f;
        mRearLegTip->mXRot = 0.5f + bob * 0.1f;
        mRearFoot->mXRot = 0.75f + bob * 0.1f;
        mFrontLeg->mXRot = 1.3f + bob * 0.1f;
        mFrontLegTip->mXRot = -0.5f - bob * 0.1f;
        mFrontFoot->mXRot = 0.75f + bob * 0.1f;

        mFrontLeg->render(scale, useDisplayList, false);
        mRearLeg->render(scale, useDisplayList, false);
    }

    if (!mHasOverriddenParts)
    {
        GlStateManager::scalef(-1.0f, 1.0f, 1.0f);
        GlStateManager::cullFace(0);
    }

    float wing1XRot = 0.125f - Mth::cos(flapAngle) * 0.2f;
    ModelPart* wing1 = mWing1;
    wing1->mXRot = wing1XRot;
    wing1->mYRot = mWing1YRot;

    if (!mHasOverriddenParts)
    {
        wing1->mZRot = (Mth::sin(flapAngle) + 0.125f) * 0.8f;
        mWingTip1->mZRot = -(Mth::sin(flapAngle + 2.0f) + 0.5f) * 0.75f;
    }
    else if (mHasNamedNeckParts)
    {
        wing1->mZRot = (Mth::sin(-flapAngle) + 0.125f) * 0.8f;
        mWingTip1->mZRot = (Mth::sin(flapAngle + 2.0f) + 0.5f) * 0.75f;
    }
    else
    {
        wing1->mZRot = 3.1415927f - (Mth::sin(flapAngle) + 0.125f) * 0.8f;
        mWingTip1->mZRot = (Mth::sin(flapAngle + 2.0f) + 0.5f) * 0.75f;
    }

    mWing1->render(scale, useDisplayList, false);

    if (mHasLegs)
    {
        mRearLeg1->mXRot = 1.0f + bob * 0.1f;
        mRearLegTip1->mXRot = 0.5f + bob * 0.1f;
        mRearFoot1->mXRot = 0.75f + bob * 0.1f;
        mFrontLeg1->mXRot = 1.3f + bob * 0.1f;
        mFrontLegTip1->mXRot = -0.5f - bob * 0.1f;
        mFrontFoot1->mXRot = 0.75f + bob * 0.1f;

        mFrontLeg1->render(scale, useDisplayList, false);
        mRearLeg1->render(scale, useDisplayList, false);
    }

    GlStateManager::popMatrix();

    if (!mHasOverriddenParts)
    {
        GlStateManager::cullFace(1);
    }

    GlStateManager::disableCull();

    float tailWobble = -Mth::sin(flapAngle) * 0.0f;

    y = 10.0f;
    z = 60.0f;
    x = 0.0f;

    dragon->getLatencyPos(headOffsets, 11, mPartialTicks);

    float tailYRotOffset = 180.0f;
    int tailSide = 1;

    if (mHasOverriddenParts)
    {
        tailYRotOffset = 0.0f;
        tailSide = -1;
    }

    for (int i = 0; i < 12; i++)
    {
        dragon->getLatencyPos(partOffsets, 12 + i, mPartialTicks);

        tailWobble += Mth::sin((float)i * 0.45f + flapAngle) * 0.05f;

        float tailYRot = (rotWrap(partOffsets.data[0] - headOffsets.data[0]) * 1.5f + tailYRotOffset) * Mth::DEG_TO_RAD;
        ModelPart* tail = mTail[i];
        float tailXRot = tailWobble
                         + (float)(partOffsets.data[1] - headOffsets.data[1]) * Mth::DEG_TO_RAD * 1.5f * 5.0f;
        tail->mYRot = tailYRot;
        tail->mXRot = tailXRot;
        tail->mZRot = rotWrap(partOffsets.data[0] - bodyYaw) * Mth::DEG_TO_RAD * 1.5f;

        tail->mY = y;
        tail->mZ = z;
        tail->mX = x;

        y += Mth::sin(tailXRot) * 10.0f * (float)tailSide;
        z -= Mth::cos(mTail[i]->mYRot) * Mth::cos(mTail[i]->mXRot) * 10.0f * (float)tailSide;
        x -= Mth::sin(mTail[i]->mYRot) * Mth::cos(mTail[i]->mXRot) * 10.0f;

        mTail[i]->render(scale, useDisplayList, false);
    }

    GlStateManager::popMatrix();

    callSkinnedDrawlist();
}
