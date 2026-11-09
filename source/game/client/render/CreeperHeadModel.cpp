#include "game/client/render/CreeperHeadModel.hpp"
#include "game/core/Mth.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"

CreeperHeadModel::CreeperHeadModel(float delta, const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mHead = new ModelPart(this, L"head", modelDefinition, NULL, 0.0f);
    }
    else
    {
        mHead = new ModelPart(this, 0, 0);
        mHead->addBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, delta);
        mHead->compile(0.0625f);
    }
}

void CreeperHeadModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                              float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
    mHead->render(scale, useDisplayList, false);

    callSkinnedDrawlist();
}
