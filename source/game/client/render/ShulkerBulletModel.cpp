#include "game/client/render/ShulkerBulletModel.hpp"
#include "game/core/Mth.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"

ShulkerBulletModel::ShulkerBulletModel(const ModelDefinition& modelDefinition)
{
    mTexWidth = 64;
    mTexHeight = 32;

    mBase = new ModelPart(this);
    mBase->texOffs(0, 0)->addBox(-4.0f, -4.0f, -1.0f, 8, 8, 2, 0.0f);
    mBase->texOffs(0, 10)->addBox(-1.0f, -4.0f, -4.0f, 2, 8, 8, 0.0f);
    mBase->texOffs(20, 0)->addBox(-4.0f, -1.0f, -4.0f, 8, 2, 8, 0.0f);
    mBase->setPos(0.0f, 0.0f, 0.0f);

    mBase->compile(0.0625f);
}

void ShulkerBulletModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                                float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mBase->render(scale, useDisplayList, false);
}

void ShulkerBulletModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                   float headPitch, float scale, boost::shared_ptr<Entity> entity,
                                   _SkinAdjustments skinAdjustments)
{
    Model::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mBase->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mBase->mXRot = headPitch * Mth::DEG_TO_RAD;
}
