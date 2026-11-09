#include "game/client/render/EvocationFangsModel.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"

EvocationFangsModel::EvocationFangsModel(const ModelDefinition& modelDefinition)
{
    mBase = new ModelPartSkin(this, 0, 0, NULL);
    mBase->setPos(-5.0f, 22.0f, -5.0f);
    mBase->addBox(0.0f, 0.0f, 0.0f, 10, 12, 10);

    mUpperJaw = new ModelPartSkin(this, 40, 0, NULL);
    mUpperJaw->setPos(1.5f, 22.0f, -4.0f);
    mUpperJaw->addBox(0.0f, 0.0f, 0.0f, 4, 14, 8, 0.01f);

    mLowerJaw = new ModelPartSkin(this, 40, 0, NULL);
    mLowerJaw->setPos(-1.5f, 22.0f, 12.0f);
    mLowerJaw->addBox(0.0f, 0.0f, 0.0f, 4, 14, 8);

    compileSkinnedDrawlist();
}

EvocationFangsModel::~EvocationFangsModel()
{
    delete mBase;
    delete mUpperJaw;
    delete mLowerJaw;
}

void EvocationFangsModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                                 float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    float openProgress = limbSwing * 2.0f;

    if (openProgress > 1.0f)
    {
        openProgress = 1.0f;
    }

    openProgress = 1.0f - openProgress * openProgress * openProgress;

    float jawAngle = openProgress * 0.35f * 3.1415927f;
    float upperAngle = 3.1415927f - jawAngle;
    float lowerAngle = jawAngle + 3.1415927f;
    mUpperJaw->mZRot = upperAngle;
    mLowerJaw->mYRot = 3.1415927f;
    mLowerJaw->mZRot = lowerAngle;

    float jawY = 24.0f - (limbSwing + sinf(limbSwing * 2.7f)) * 0.6f * 12.0f;
    mUpperJaw->mY = jawY;
    mLowerJaw->mY = jawY;
    mBase->mY = mUpperJaw->mY;

    mBase->render(scale, useDisplayList, false);
    mUpperJaw->render(scale, useDisplayList, false);
    mLowerJaw->render(scale, useDisplayList, false);

    callSkinnedDrawlist();
}
