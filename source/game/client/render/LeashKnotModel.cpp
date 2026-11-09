#include "game/client/render/LeashKnotModel.hpp"
#include "game/core/Mth.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"

LeashKnotModel::LeashKnotModel(const ModelDefinition& modelDefinition)
{
    _init(0, 0, 32, 32);
}

LeashKnotModel::LeashKnotModel(int xTexOffs, int yTexOffs, int texWidth, int texHeight)
{
    _init(xTexOffs, yTexOffs, texWidth, texHeight);
}

void LeashKnotModel::_init(int xTexOffs, int yTexOffs, int texWidth, int texHeight)
{
    mTexWidth = texWidth;
    mTexHeight = texHeight;

    mKnot = new ModelPart(this, xTexOffs, yTexOffs);
    mKnot->addBox(-3.0f, -6.0f, -3.0f, 6, 8, 6, 0.0f);
    mKnot->setPos(0.0f, 0.0f, 0.0f);
}

void LeashKnotModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                            float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mKnot->render(scale, useDisplayList, false);
}

void LeashKnotModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                               float headPitch, float scale, boost::shared_ptr<Entity> entity,
                               _SkinAdjustments skinAdjustments)
{
    Model::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mKnot->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mKnot->mXRot = headPitch * Mth::DEG_TO_RAD;
}
