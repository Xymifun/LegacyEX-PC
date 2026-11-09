#include "game/client/render/BannerModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"

BannerModel::BannerModel(const ModelDefinition& modelDefinition)
{
    mTexWidth = 64;
    mTexHeight = 64;

    mFlag = new ModelPartSkin(this, 0, 0, NULL);
    mFlag->addBox(-10.0f, 0.0f, -2.0f, 20, 40, 1, 0.0f);

    mPole = new ModelPartSkin(this, 44, 0, NULL);
    mPole->addBox(-1.0f, -30.0f, -1.0f, 2, 42, 2, 0.0f);

    mBar = new ModelPartSkin(this, 0, 42, NULL);
    mBar->addBox(-10.0f, -32.0f, -1.0f, 20, 2, 2, 0.0f);

    compileSkinnedDrawlist();
}

BannerModel::~BannerModel()
{
}

void BannerModel::render()
{
    mFlag->mY = -32.0f;

    mFlag->render(0.0625f, true, false);
    mPole->render(0.0625f, true, false);
    mBar->render(0.0625f, true, false);

    callSkinnedDrawlist();
}
