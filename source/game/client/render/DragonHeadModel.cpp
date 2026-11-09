#include "game/client/render/DragonHeadModel.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"

DragonHeadModel::DragonHeadModel(float scale, const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;

        mHead = new ModelPart(this, L"head", modelDefinition, NULL, 0.0f);
        mJaw = new ModelPart(this, L"jaw", modelDefinition, NULL, 0.0f);

        mHead->addChild(mJaw);

        mUsingDefaultModel = false;
    }
    else
    {
        mTexWidth = 256;
        mTexHeight = 256;

        setMapTex(L"head.upperhead", 112, 30);
        setMapTex(L"head.upperlip", 176, 44);
        setMapTex(L"jaw.jaw", 176, 65);
        setMapTex(L"head.scale", 0, 0);
        setMapTex(L"neck.scale", 48, 0);
        setMapTex(L"head.nostril", 112, 0);

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

        mUsingDefaultModel = true;
    }

    mHead->compile(0.0625f);
    mJaw->compile(0.0625f);
}

void DragonHeadModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    mJaw->mXRot = (sin(limbSwing * 3.1415927f * 0.2f) + 1.0f) * 0.2f;

    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;

    GlStateManager::translatef(0.0f, -0.374375f, 0.0f);
    GlStateManager::scalef(0.75f, 0.75f, 0.75f);

    mHead->render(scale, useDisplayList, false);
}

bool DragonHeadModel::hasJawPart()
{
    if (mHasOverriddenParts && mJaw->mCubes.size() == 0)
    {
        return false;
    }

    return true;
}
