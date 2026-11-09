#include "game/client/render/SkeletonHeadModel.hpp"
#include "game/core/Mth.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"

SkeletonHeadModel::SkeletonHeadModel(const ModelDefinition& modelDefinition) : Model()
{
    _init(0, 35, 64, 64, modelDefinition);
}

SkeletonHeadModel::SkeletonHeadModel(int xTexOffs, int yTexOffs, int texWidth, int texHeight,
                                     const ModelDefinition& modelDefinition)
    : Model()
{
    _init(xTexOffs, yTexOffs, texWidth, texHeight, modelDefinition);
}

void SkeletonHeadModel::_init(int xTexOffs, int yTexOffs, int texWidth, int texHeight,
                              const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mHead = new ModelPart(this, L"head", modelDefinition, NULL, 0.0f);
    }
    else
    {
        mTexWidth = texWidth;
        mTexHeight = texHeight;

        mHead = new ModelPart(this, xTexOffs, yTexOffs);
        mHead->addHumanoidBox(-4.0f, -8.0f, -4.0f, 8, 8, 8, 0.1f);
        mHead->setPos(0.0f, 0.0f, 0.0f);
        mHead->compile(0.0625f);
    }
}

void SkeletonHeadModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                               float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mHead->render(scale, useDisplayList, false);
}

void SkeletonHeadModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                  float headPitch, float scale, boost::shared_ptr<Entity> entity,
                                  _SkinAdjustments skinAdjustments)
{
    Model::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, skinAdjustments);

    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
}
