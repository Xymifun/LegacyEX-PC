#include "game/client/render/BoatModel.hpp"

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Boat.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#endif

ModelPartSkin* BoatModel::makePaddle(bool left)
{
    ModelPartSkin* paddle = new ModelPartSkin(this, 62, left ? 0 : 20, NULL);
    paddle->setTexSize(128, 64);
    int i = 20;
    int j = 7;
    int k = 6;
    float f = -5.0f;
    paddle->addBox(-1.0f, 0.0f, -5.0f, 2, 2, 18);
    paddle->addBox(left ? -1.001f : 0.001f, -3.0f, 8.0f, 1, 6, 7);
    return paddle;
}

BoatModel::BoatModel(const ModelDefinition& definition)
{
    if (definition.DoesOverrideModel())
    {
        mBottom = new ModelPartSkin(this, L"bottom", definition, NULL, 0.0f, NULL);
        mBack = new ModelPartSkin(this, L"back", definition, NULL, 0.0f, NULL);
        mFront = new ModelPartSkin(this, L"front", definition, NULL, 0.0f, NULL);
        mRight = new ModelPartSkin(this, L"right", definition, NULL, 0.0f, NULL);
        mLeft = new ModelPartSkin(this, L"left", definition, NULL, 0.0f, NULL);
        mPaddleLeft = new ModelPartSkin(this, L"paddle_left", definition, NULL, 0.0f, NULL);
        mPaddleRight = new ModelPartSkin(this, L"paddle_right", definition, NULL, 0.0f, NULL);
    }
    else
    {
        mBottom = new ModelPartSkin(this, 0, 0, 128, 64);
        mBack = new ModelPartSkin(this, 0, 19, 128, 64);
        mFront = new ModelPartSkin(this, 0, 27, 128, 64);
        mRight = new ModelPartSkin(this, 0, 35, 128, 64);
        mLeft = new ModelPartSkin(this, 0, 43, 128, 64);
        int i = 32;
        int j = 6;
        int k = 20;
        int l = 4;
        int i1 = 28;
        mBottom->addBox(-14.0f, -9.0f, -3.0f, 28, 16, 3, 0.0f);
        mBottom->setPos(0.0f, 3.0f, 1.0f);
        mBack->addBox(-13.0f, -7.0f, -1.0f, 18, 6, 2, 0.0f);
        mBack->setPos(-15.0f, 4.0f, 4.0f);
        mFront->addBox(-8.0f, -7.0f, -1.0f, 16, 6, 2, 0.0f);
        mFront->setPos(15.0f, 4.0f, 0.0f);
        mRight->addBox(-14.0f, -7.0f, -1.0f, 28, 6, 2, 0.0f);
        mRight->setPos(0.0f, 4.0f, -9.0f);
        mLeft->addBox(-14.0f, -7.0f, -1.0f, 28, 6, 2, 0.0f);
        mLeft->setPos(0.0f, 4.0f, 9.0f);

        mBottom->mXRot = 1.5707964f;
        mBack->mYRot = 4.712389f;
        mFront->mYRot = 1.5707964f;
        mRight->mYRot = 3.1415927f;

        mPaddleLeft = makePaddle(true);
        mPaddleLeft->setPos(3.0f, -5.0f, 9.0f);
        mPaddleRight = makePaddle(false);
        mPaddleRight->setPos(3.0f, -5.0f, -9.0f);

        mPaddleRight->mYRot = 3.1415927f;
        mPaddleLeft->mZRot = 0.19634955f;
        mPaddleRight->mZRot = 0.19634955f;
    }

    mSecondPassBottom = new ModelPart(this, 0, 0);
    mSecondPassBottom->setTexSize(128, 64);
    mSecondPassBottom->addBox(-14.0f, -9.0f, -3.0f, 28, 16, 3, 0.0f);
    mSecondPassBottom->setPos(0.0f, -3.0f, 1.0f);
    mSecondPassBottom->mXRot = 1.5707964f;

    compileSkinnedDrawlist();
    mSecondPassBottom->compile(0.0625f);
}

void BoatModel::animatePaddle(ModelPartSkin* paddle, boost::shared_ptr<Boat> boat, int side, float scale,
                              float partialTicks, bool useDisplayList)
{
    float rowingTime = boat->getRowingTime(side, partialTicks);

    paddle->mXRot = (float)Mth::clampedLerp(-1.0471976f, -0.2617994f, (Mth::sin(-rowingTime) + 1.0f) * 0.5f);

    paddle->mYRot = (float)Mth::clampedLerp(-0.7853982f, 0.7853982f, (Mth::sin(1.0f - rowingTime) + 1.0f) * 0.5f);
    if (side == 1)
    {
        paddle->mYRot = 3.1415927f - paddle->mYRot;
    }

    paddle->render(scale, useDisplayList, false);
}

void BoatModel::render(boost::shared_ptr<Entity>& entity, float partialTicks, float limbSwingAmount, float ageInTicks,
                       float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    GlStateManager::rotatef(90.0f, 0.0f, 1.0f, 0.0f);

    boost::shared_ptr<Boat> boat = boost::dynamic_pointer_cast<Boat>(entity);

    setupAnim(partialTicks, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mBottom->render(scale, useDisplayList, false);
    mBack->render(scale, useDisplayList, false);
    mFront->render(scale, useDisplayList, false);
    mRight->render(scale, useDisplayList, false);
    mLeft->render(scale, useDisplayList, false);

#ifdef MINECRAFT_PC
    if (!PcGoldenAgeOn(kPcGoldenAgeModels))
#endif
    {
        animatePaddle(mPaddleLeft, boat, 0, scale, partialTicks, useDisplayList);
        animatePaddle(mPaddleRight, boat, 1, scale, partialTicks, useDisplayList);
    }

    callSkinnedDrawlist();
}

void BoatModel::renderSecondPass(boost::shared_ptr<Entity> entity, float partialTicks, float limbSwingAmount,
                                 float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    GlStateManager::rotatef(90.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::colorMask(false, false, false, false);
    mSecondPassBottom->render(scale, useDisplayList, false);
    GlStateManager::colorMask(true, true, true, false);
}
