#include "game/client/render/BedModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"

BedModel::BedModel(const ModelDefinition& modelDefinition)
{
    mTexWidth = 64;
    mTexHeight = 128;

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;

        mBed = new ModelPartSkin(this, L"bed", modelDefinition, NULL, 0.0f, NULL);
        mLeg0 = new ModelPartSkin(this, L"leg0", modelDefinition, NULL, 0.0f, NULL);
        mLeg1 = new ModelPartSkin(this, L"leg1", modelDefinition, NULL, 0.0f, NULL);
        mLeg2 = new ModelPartSkin(this, L"leg2", modelDefinition, NULL, 0.0f, NULL);
        mLeg3 = new ModelPartSkin(this, L"leg3", modelDefinition, NULL, 0.0f, NULL);
    }
    else
    {
        mBed = new ModelPartSkin(this, 0, 0, NULL);
        mBed->addBox(0.0f, 0.0f, 0.0f, 16, 32, 6, 0.0f);
        mBed->addBoxMapTex(3.0f, 31.0f, 6.0f, 10, 1, 3, 38, 2, 0.0f);
        mBed->addBoxMapTex(3.0f, 0.0f, 6.0f, 10, 1, 3, 38, 38, 0.0f);
        mBed->addBoxMapTex(15.0f, 3.0f, 6.0f, 1, 26, 3, 52, 6, 0.0f);
        mBed->addBoxMapTex(0.0f, 3.0f, 6.0f, 1, 26, 3, 44, 6, 0.0f);

        mLeg0 = new ModelPartSkin(this, 0, 44, NULL);
        mLeg0->addBox(0.0f, 29.0f, 6.0f, 3, 3, 3);

        mLeg1 = new ModelPartSkin(this, 12, 44, NULL);
        mLeg1->addBox(13.0f, 29.0f, 6.0f, 3, 3, 3);

        mLeg2 = new ModelPartSkin(this, 0, 38, NULL);
        mLeg2->addBox(0.0f, 0.0f, 6.0f, 3, 3, 3);

        mLeg3 = new ModelPartSkin(this, 12, 38, NULL);
        mLeg3->addBox(13.0f, 0.0f, 6.0f, 3, 3, 3);
    }

    compileSkinnedDrawlist();
}

uint32_t BedModel::modelVersion()
{
    return 51;
}

void BedModel::render(bool useDisplayList)
{
    mBed->render(0.0625f, useDisplayList, false);
    mLeg0->render(0.0625f, useDisplayList, false);
    mLeg1->render(0.0625f, useDisplayList, false);
    mLeg2->render(0.0625f, useDisplayList, false);
    mLeg3->render(0.0625f, useDisplayList, false);

    callSkinnedDrawlist();
}
