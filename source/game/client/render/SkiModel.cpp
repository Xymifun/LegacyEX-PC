#include "game/client/render/SkiModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"

#include <math.h>

void SkiModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                      float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    for (unsigned int i = 0; i < mSkiParts.length; i++)
    {
        mSkiParts.data[i]->render(scale, useDisplayList, false);
    }

    callSkinnedDrawlist();
}

void SkiModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                         float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    mSkiParts.data[0]->mY = 24.2f;
    mSkiParts.data[0]->mXRot = 1.5707964f;
    mSkiParts.data[1]->mY = 24.2f;
    mSkiParts.data[1]->mXRot = 1.5707964f;

    if (!mMirrored)
    {
        mSkiParts.data[0]->mZ = cosf(limbSwing * 0.6662f) * 0.7f * 12.0f * limbSwingAmount - 26.0f;
        mSkiParts.data[1]->mZ = cosf(limbSwing * 0.6662f) * 0.7f * 12.0f * limbSwingAmount - 26.0f;
        mSkiParts.data[0]->mX = -3.5f;
        mSkiParts.data[1]->mX = -3.5f;
    }
    else
    {
        mSkiParts.data[0]->mZ = -(cosf(limbSwing * 0.6662f) * 0.7f * 12.0f * limbSwingAmount + 26.0f);
        mSkiParts.data[1]->mZ = -(cosf(limbSwing * 0.6662f) * 0.7f * 12.0f * limbSwingAmount + 26.0f);
        mSkiParts.data[0]->mX = 0.5f;
        mSkiParts.data[1]->mX = 0.5f;
    }
}
