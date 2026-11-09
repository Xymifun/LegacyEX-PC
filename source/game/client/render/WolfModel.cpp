#include "game/client/render/WolfModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Wolf.hpp"
WolfModel::WolfModel(const ModelDefinition& modelDefinition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;
        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Wolf);

        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mLeg2 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, NULL);
        mLeg3 = new ModelPartSkin(this, L"leg2", modelDefinition, NULL, 0.0f, NULL);
        mLeg4 = new ModelPartSkin(this, L"leg3", modelDefinition, NULL, 0.0f, NULL);
        mTail = new ModelPartSkin(this, L"tail", modelDefinition, NULL, 0.0f, NULL);
        mUpperBody = new ModelPartSkin(this, L"upperBody", modelDefinition, NULL, 0.0f, NULL);

        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
    else
    {
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-2.0f, -3.0f, -2.0f, 6, 6, 4, 0.0f);
        mHead->setPos(-1.0f, 13.5f, -7.0f);

        mBody = new ModelPartSkin(this, 18, 14, NULL);
        mBody->addBox(-3.0f, -2.0f, -3.0f, 6, 9, 6, 0.0f);
        mBody->setPos(0.0f, 14.0f, 2.0f);

        mUpperBody = new ModelPartSkin(this, 21, 0, NULL);
        mUpperBody->addBox(-3.0f, -3.0f, -3.0f, 8, 6, 7, 0.0f);
        mUpperBody->setPos(-1.0f, 14.0f, 2.0f);

        mLeg1 = new ModelPartSkin(this, 0, 18, NULL);
        mLeg1->addBox(0.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mLeg1->setPos(-2.5f, 16.0f, 7.0f);

        mLeg2 = new ModelPartSkin(this, 0, 18, NULL);
        mLeg2->addBox(0.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mLeg2->setPos(0.5f, 16.0f, 7.0f);

        mLeg3 = new ModelPartSkin(this, 0, 18, NULL);
        mLeg3->addBox(0.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mLeg3->setPos(-2.5f, 16.0f, -4.0f);

        mLeg4 = new ModelPartSkin(this, 0, 18, NULL);
        mLeg4->addBox(0.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mLeg4->setPos(0.5f, 16.0f, -4.0f);

        mTail = new ModelPartSkin(this, 9, 18, NULL);
        mTail->addBox(0.0f, 0.0f, -1.0f, 2, 8, 2, 0.0f);
        mTail->setPos(-1.0f, 12.0f, 8.0f);

        mHead->texOffs(16, 14)->addBox(-2.0f, -5.0f, 0.0f, 2, 2, 1, 0.0f);
        mHead->texOffs(16, 14)->addBox(2.0f, -5.0f, 0.0f, 2, 2, 1, 0.0f);
        mHead->texOffs(0, 10)->addBox(-0.5f, 0.0f, -5.0f, 3, 3, 4, 0.0f);

        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
}
void WolfModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                       float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());
    if (mYoung)
    {
        GlStateManager::pushMatrix();
        GlStateManager::translatef(0.0f, 5.0f * scale, scale + scale);
        mHead->renderRollable(scale, useDisplayList);
        GlStateManager::popMatrix();
        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.5f, 0.5f, 0.5f);
        GlStateManager::translatef(0.0f, 24.0f * scale, 0.0f);
        mBody->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        mLeg4->render(scale, useDisplayList, false);
        mTail->renderRollable(scale, useDisplayList);
        mUpperBody->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();
    }
    else
    {
        mHead->renderRollable(scale, useDisplayList);
        mBody->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        mLeg4->render(scale, useDisplayList, false);
        mTail->renderRollable(scale, useDisplayList);
        mUpperBody->render(scale, useDisplayList, false);
    }
    callSkinnedDrawlist();
}
void WolfModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                float partialTicks)
{
    boost::shared_ptr<Wolf> wolf = boost::dynamic_pointer_cast<Wolf>(entity);

    if (wolf->isAngry())
        mTail->mYRot = 0.0f;
    else
        mTail->mYRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount;

    if (wolf->isSitting())
    {
        mUpperBody->setPos(-1.0f, 16.0f, -3.0f);
        mUpperBody->mXRot = 1.2566371f;
        mUpperBody->mYRot = 0.0f;
        mBody->setPos(0.0f, 18.0f, 0.0f);
        mBody->mXRot = 0.7853982f;
        mTail->setPos(-1.0f, 21.0f, 6.0f);
        mLeg1->setPos(-2.5f, 22.0f, 2.0f);
        mLeg1->mXRot = 4.712389f;
        mLeg2->setPos(0.5f, 22.0f, 2.0f);
        mLeg2->mXRot = 4.712389f;
        mLeg3->mXRot = 5.811947f;
        mLeg3->setPos(-2.49f, 17.0f, -4.0f);
        mLeg4->mXRot = 5.811947f;
        mLeg4->setPos(0.51f, 17.0f, -4.0f);
    }
    else
    {
        mBody->setPos(0.0f, 14.0f, 2.0f);
        mBody->mXRot = 90.0f * Mth::DEG_TO_RAD;
        mUpperBody->setPos(-1.0f, 14.0f, -3.0f);
        mUpperBody->mXRot = mBody->mXRot;
        mTail->setPos(-1.0f, 12.0f, 8.0f);
        mLeg1->setPos(-2.5f, 16.0f, 7.0f);
        mLeg2->setPos(0.5f, 16.0f, 7.0f);
        mLeg3->setPos(-2.5f, 16.0f, -4.0f);
        mLeg4->setPos(0.5f, 16.0f, -4.0f);

        mLeg1->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount;
        mLeg2->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount;
        mLeg3->mXRot = Mth::cos(limbSwing * 0.6662f + 3.1415927f) * 1.4f * limbSwingAmount;
        mLeg4->mXRot = Mth::cos(limbSwing * 0.6662f) * 1.4f * limbSwingAmount;
    }

    float headRoll = wolf->getHeadRollAngle(partialTicks) + wolf->getBodyRollAngle(partialTicks, 0.0f);
    mHead->mZRot = headRoll;
    mUpperBody->mZRot = (float)wolf->getBodyRollAngle(partialTicks, -0.08f);
    mBody->mZRot = (float)wolf->getBodyRollAngle(partialTicks, -0.16f);
    mTail->mZRot = (float)wolf->getBodyRollAngle(partialTicks, -0.2f);
}
void WolfModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                          float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    Model::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mTail->mXRot = ageInTicks;
}
