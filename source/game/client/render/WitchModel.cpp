#include "game/client/render/WitchModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include <cmath>

WitchModel::WitchModel(float scale, const ModelDefinition& modelDefinition)
    : VillagerModel(scale, 0.0f, 64, 128, modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mWitchNose = new ModelPartSkin(this, L"bottom", modelDefinition, NULL, 0.0f, NULL);
        mHat = new ModelPartSkin(this, L"hat", modelDefinition, NULL, 0.0f, NULL);
        mHat2 = new ModelPartSkin(this, L"hat2", modelDefinition, NULL, 0.0f, NULL);
        mHat3 = new ModelPartSkin(this, L"hat3", modelDefinition, NULL, 0.0f, NULL);
        mHat4 = new ModelPartSkin(this, L"hat4", modelDefinition, NULL, 0.0f, NULL);
        mNose->addChild(mWitchNose);
        mHead->addChild(mHat);
        mHat->addChild(mHat2);
        mHat2->addChild(mHat3);
        mHat3->addChild(mHat4);
        compileSkinnedDrawlist();
    }
    else
    {
        mWitchNose = new ModelPartSkin(this);
        mWitchNose->setTexSize(64, 128);
        mWitchNose->setPos(0.0f, -2.0f, 0.0f);
        mWitchNose->texOffs(0, 0)->addBox(0.0f, 3.0f, -6.75f, 1, 1, 1, -0.25f);
        mHat = new ModelPartSkin(this);
        mHat->setTexSize(64, 128);
        mHat->setPos(-5.0f, -10.03125f, -5.0f);
        mHat->texOffs(0, 64)->addBox(0.0f, 0.0f, 0.0f, 10, 2, 10);
        mHat2 = new ModelPartSkin(this);
        mHat2->setTexSize(64, 128);
        mHat2->setPos(1.75f, -4.0f, 2.0f);
        mHat2->texOffs(0, 76)->addBox(0.0f, 0.0f, 0.0f, 7, 4, 7);
        mHat2->mXRot = -3.0f * Mth::DEG_TO_RAD;
        mHat2->mZRot = 1.5f * Mth::DEG_TO_RAD;
        mHat3 = new ModelPartSkin(this);
        mHat3->setTexSize(64, 128);
        mHat3->setPos(1.75f, -4.0f, 2.0f);
        mHat3->texOffs(0, 87)->addBox(0.0f, 0.0f, 0.0f, 4, 4, 4);
        mHat3->mXRot = -6.0f * Mth::DEG_TO_RAD;
        mHat3->mZRot = 3.0f * Mth::DEG_TO_RAD;
        mHat4 = new ModelPartSkin(this);
        mHat4->setTexSize(64, 128);
        mHat4->setPos(1.75f, -2.0f, 2.0f);
        mHat4->texOffs(0, 95)->addBox(0.0f, 0.0f, 0.0f, 1, 2, 1, 0.25f);
        mHat4->mXRot = -12.0f * Mth::DEG_TO_RAD;
        mHat4->mZRot = 6.0f * Mth::DEG_TO_RAD;
        mNose->addChild(mWitchNose);
        mHead->addChild(mHat);
        mHat->addChild(mHat2);
        mHat2->addChild(mHat3);
        mHat3->addChild(mHat4);
        compileSkinnedDrawlist();
    }
}

void WitchModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments)
{
    VillagerModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                             _SkinAdjustments());
    mNose->mXOffset = 0.0f;
    mNose->mYOffset = 0.0f;
    mNose->mZOffset = 0.0f;
    float rate = 0.01f * (float)(entity->getId() % 10);
    mNose->mXRot = std::sin((float)entity->mTickCount * rate) * 4.5f * Mth::DEG_TO_RAD;
    mNose->mYRot = 0.0f;
    mNose->mZRot = std::cos((float)entity->mTickCount * rate) * 2.5f * Mth::DEG_TO_RAD;
    if (mHoldingItem)
    {
        mNose->mXRot = -0.9f;
        mNose->mZOffset = -0.09375f;
        mNose->mYOffset = 0.1875f;
    }
}
