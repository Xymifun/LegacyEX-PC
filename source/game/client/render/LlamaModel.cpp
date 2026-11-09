#include "game/client/render/LlamaModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/entity/mob/Entity.hpp"

LlamaModel::LlamaModel(float scale, const ModelDefinition& modelDefinition) : QuadrupedModel(15, scale, modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mChest1 = new ModelPartSkin(this, L"chest1", modelDefinition, NULL, 0.0f, NULL);
        mChest2 = new ModelPartSkin(this, L"chest2", modelDefinition, NULL, 0.0f, NULL);
    }
    else
    {
        mTexWidth = 128;
        mTexHeight = 64;

        delete mHead;
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-2.0f, -14.0f, -10.0f, 4, 4, 9, scale);
        mHead->setPos(0.0f, 7.0f, -6.0f);
        mHead->texOffs(0, 14)->addBox(-4.0f, -16.0f, -6.0f, 8, 18, 6, scale);
        mHead->texOffs(17, 0)->addBox(-4.0f, -19.0f, -4.0f, 3, 3, 2, scale);
        mHead->texOffs(17, 0)->addBox(1.0f, -19.0f, -4.0f, 3, 3, 2, scale);

        delete mBody;
        mBody = new ModelPartSkin(this, 29, 0, NULL);
        mBody->addBox(-6.0f, -10.0f, -7.0f, 12, 18, 10, scale);
        mBody->setPos(0.0f, 5.0f, 2.0f);

        mChest1 = new ModelPartSkin(this, 45, 28, NULL);
        mChest1->addBox(-3.0f, 0.0f, 0.0f, 8, 8, 3, scale);
        mChest1->setPos(-8.5f, 3.0f, 3.0f);
        mChest1->mYRot = 1.5707964f;

        mChest2 = new ModelPartSkin(this, 45, 41, NULL);
        mChest2->addBox(-3.0f, 0.0f, 0.0f, 8, 8, 3, scale);
        mChest2->setPos(5.5f, 3.0f, 3.0f);
        mChest2->mYRot = 1.5707964f;

        delete mLeg0;
        mLeg0 = new ModelPartSkin(this, 29, 29, NULL);
        mLeg0->addBox(-2.0f, 0.0f, -2.0f, 4, 14, 4, scale);
        mLeg0->setPos(-2.5f, 10.0f, 6.0f);

        delete mLeg1;
        mLeg1 = new ModelPartSkin(this, 29, 29, NULL);
        mLeg1->addBox(-2.0f, 0.0f, -2.0f, 4, 14, 4, scale);
        mLeg1->setPos(2.5f, 10.0f, 6.0f);

        delete mLeg2;
        mLeg2 = new ModelPartSkin(this, 29, 29, NULL);
        mLeg2->addBox(-2.0f, 0.0f, -2.0f, 4, 14, 4, scale);
        mLeg2->setPos(-2.5f, 10.0f, -4.0f);

        delete mLeg3;
        mLeg3 = new ModelPartSkin(this, 29, 29, NULL);
        mLeg3->addBox(-2.0f, 0.0f, -2.0f, 4, 14, 4, scale);
        mLeg3->setPos(2.5f, 10.0f, -4.0f);

        mLeg0->mX -= 1.0f;
        mLeg1->mX += 1.0f;
        mLeg2->mX -= 1.0f;
        mLeg3->mX += 1.0f;
        mLeg2->mZ -= 1.0f;
        mLeg3->mZ -= 1.0f;
        mZHeadOffs += 2.0f;
    }

    compileSkinnedDrawlist();
}

LlamaModel::~LlamaModel()
{
    delete mChest1;
    delete mChest2;
}

void LlamaModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    boost::shared_ptr<AbstractChestedHorse> horse = boost::static_pointer_cast<AbstractChestedHorse>(entity);
    bool chestVisible = !horse->isBaby() && horse->hasChest();

    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    if (mYoung)
    {
        GlStateManager::pushMatrix();
        GlStateManager::translatef(0.0f, mYHeadOffs * scale, mZHeadOffs * scale);
        GlStateManager::popMatrix();

        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.71428573f, 0.64935064f, 0.79365081f);
        GlStateManager::translatef(0.0f, 21.0f * scale, 0.22f);
        mHead->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();

        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.625f, 0.45454544f, 0.45454544f);
        GlStateManager::translatef(0.0f, 33.0f * scale, 0.0f);
        mBody->render(scale, useDisplayList, false);
        GlStateManager::popMatrix();

        GlStateManager::pushMatrix();
        GlStateManager::scalef(0.45454544f, 0.41322312f, 0.45454544f);
        GlStateManager::translatef(0.0f, 33.0f * scale, 0.0f);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        mChest1->render(scale, useDisplayList, !chestVisible);
        mChest2->render(scale, useDisplayList, !chestVisible);
        GlStateManager::popMatrix();
    }
    else
    {
        mHead->render(scale, useDisplayList, false);
        mBody->render(scale, useDisplayList, false);
        mLeg0->render(scale, useDisplayList, false);
        mLeg1->render(scale, useDisplayList, false);
        mLeg2->render(scale, useDisplayList, false);
        mLeg3->render(scale, useDisplayList, false);
        mChest1->render(scale, useDisplayList, !chestVisible);
        mChest2->render(scale, useDisplayList, !chestVisible);
    }

    callSkinnedDrawlist();
}
