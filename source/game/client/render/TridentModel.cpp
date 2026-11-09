#include "game/client/render/TridentModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"

TridentModel::TridentModel(const ModelDefinition& modelDefinition)
{
    mTexWidth = 32;
    mTexHeight = 32;

    if (modelDefinition.DoesOverrideModel())
    {
        mPole = new ModelPartSkin(this, L"pole", modelDefinition, NULL, 0.0f, NULL);
    }
    else
    {
        mPole = new ModelPartSkin(this, 0, 0, NULL);
        mPole->addBox(-0.5f, -4.0f, -0.49f, 1, 31, 1, 0.0f);

        ModelPartSkin* head = new ModelPartSkin(this, 4, 0, NULL);
        head->addBox(-1.5f, 0.0f, -0.5f, 3, 2, 1);
        mPole->addChild(head);

        ModelPartSkin* leftProng = new ModelPartSkin(this, 4, 3, NULL);
        leftProng->addBox(-2.5f, -3.0f, -0.5f, 1, 4, 1);
        mPole->addChild(leftProng);

        ModelPartSkin* rightProng = new ModelPartSkin(this, 4, 3, NULL);
        rightProng->mMirror = true;
        rightProng->addBox(1.5f, -3.0f, -0.5f, 1, 4, 1);
        mPole->addChild(rightProng);
    }

    compileSkinnedDrawlist();
}

void TridentModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                          float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    mPole->render(0.0625f, useDisplayList, false);

    callSkinnedDrawlist();
}
