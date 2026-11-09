#include "game/client/render/MinecartModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"

MinecartModel::MinecartModel(const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mBottom = new ModelPartSkin(this, L"bottom", modelDefinition, NULL, 0.0f, NULL);
        mBack = new ModelPartSkin(this, L"back", modelDefinition, NULL, 0.0f, NULL);
        mFront = new ModelPartSkin(this, L"front", modelDefinition, NULL, 0.0f, NULL);
        mRight = new ModelPartSkin(this, L"right", modelDefinition, NULL, 0.0f, NULL);
        mLeft = new ModelPartSkin(this, L"left", modelDefinition, NULL, 0.0f, NULL);
    }
    else
    {
        mBottom = new ModelPartSkin(this, 0, 10, NULL);
        mBack = new ModelPartSkin(this, 0, 0, NULL);
        mFront = new ModelPartSkin(this, 0, 0, NULL);
        mRight = new ModelPartSkin(this, 0, 0, NULL);
        mLeft = new ModelPartSkin(this, 0, 0, NULL);

        mBottom->addBox(-10.0f, -8.0f, -1.0f, 20, 16, 2, 0.0f);
        mBottom->setPos(0.0f, 4.0f, 0.0f);

        mBack->addBox(-8.0f, -9.0f, -1.0f, 16, 8, 2, 0.0f);
        mBack->setPos(-9.0f, 4.0f, 0.0f);

        mFront->addBox(-8.0f, -9.0f, -1.0f, 16, 8, 2, 0.0f);
        mFront->setPos(9.0f, 4.0f, 0.0f);

        mRight->addBox(-8.0f, -9.0f, -1.0f, 16, 8, 2, 0.0f);
        mRight->setPos(0.0f, 4.0f, -7.0f);

        mLeft->addBox(-8.0f, -9.0f, -1.0f, 16, 8, 2, 0.0f);
        mLeft->setPos(0.0f, 4.0f, 7.0f);

        mBottom->mXRot = 1.5707964f;
        mBack->mYRot = 4.712389f;
        mFront->mYRot = 1.5707964f;
        mRight->mYRot = 3.1415927f;
    }

    compileSkinnedDrawlist();
}

void MinecartModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                           float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    mBottom->render(scale, useDisplayList, false);
    mBack->render(scale, useDisplayList, false);
    mFront->render(scale, useDisplayList, false);
    mRight->render(scale, useDisplayList, false);
    mLeft->render(scale, useDisplayList, false);

    callSkinnedDrawlist();
}
