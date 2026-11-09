#include "game/client/render/BookModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"

BookModel::BookModel(const ModelDefinition& modelDefinition)
{
    mLeftLid = new ModelPartSkin(this);
    mLeftLid->texOffs(0, 0)->addBox(-6.0f, -5.0f, 0.0f, 6, 10, 0);

    mRightLid = new ModelPartSkin(this);
    mRightLid->texOffs(16, 0)->addBox(0.0f, -5.0f, 0.0f, 6, 10, 0);

    mSeam = new ModelPartSkin(this);
    mSeam->texOffs(12, 0)->addBox(-1.0f, -5.0f, 0.0f, 2, 10, 0);

    mLeftPages = new ModelPartSkin(this);
    mLeftPages->texOffs(0, 10)->addBoxWithMask(0.0f, -4.0f, -0.99f, 5, 8, 1, 47);

    mRightPages = new ModelPartSkin(this);
    mRightPages->texOffs(12, 10)->addBoxWithMask(0.0f, -4.0f, -0.01f, 5, 8, 1, 31);

    mFlipPage1 = new ModelPartSkin(this);
    mFlipPage1->texOffs(24, 10)->addBox(0.0f, -4.0f, 0.0f, 5, 8, 0);

    mFlipPage2 = new ModelPartSkin(this);
    mFlipPage2->texOffs(24, 10)->addBox(0.0f, -4.0f, 0.0f, 5, 8, 0);

    mLeftLid->setPos(0.0f, 0.0f, -1.0f);
    mRightLid->setPos(0.0f, 0.0f, 1.0f);

    mSeam->mYRot = 1.5707964f;

    compileSkinnedDrawlist();
}

void BookModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                       float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mLeftLid->render(scale, useDisplayList, false);
    mRightLid->render(scale, useDisplayList, false);
    mSeam->render(scale, useDisplayList, false);
    mLeftPages->render(scale, useDisplayList, false);
    mRightPages->render(scale, useDisplayList, false);
    mFlipPage1->render(scale, useDisplayList, false);
    mFlipPage2->render(scale, useDisplayList, false);

    callSkinnedDrawlist();
}

void BookModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                          float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    float openAngle = (Mth::sin(limbSwing * 0.02f) * 0.1f + 1.25f) * netHeadYaw;

    mLeftLid->mYRot = 3.1415927f + openAngle;
    mRightLid->mYRot = -openAngle;
    mLeftPages->mYRot = openAngle;
    mRightPages->mYRot = -openAngle;
    mFlipPage1->mYRot = openAngle - openAngle * 2.0f * limbSwingAmount;
    mFlipPage2->mYRot = openAngle - openAngle * 2.0f * ageInTicks;

    mLeftPages->mX = Mth::sin(openAngle);
    mRightPages->mX = Mth::sin(openAngle);
    mFlipPage1->mX = Mth::sin(openAngle);
    mFlipPage2->mX = Mth::sin(openAngle);
}
