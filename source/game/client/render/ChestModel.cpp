#include "game/client/render/ChestModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"

ChestModel::ChestModel(const ModelDefinition&)
{
    mLid = (new ModelPart(this, 0, 0))->setTexSize(64, 64);
    mLid->addBox(0.0f, -5.0f, -14.0f, 14, 5, 14, 0.0f);
    mLid->mX = 1.0f;
    mLid->mY = 7.0f;
    mLid->mZ = 15.0f;

    mLock = (new ModelPart(this, 0, 0))->setTexSize(64, 64);
    mLock->addBox(-1.0f, -2.0f, -15.0f, 2, 4, 1, 0.0f);
    mLock->mX = 8.0f;
    mLock->mY = 7.0f;
    mLock->mZ = 15.0f;

    mBottom = (new ModelPart(this, 0, 19))->setTexSize(64, 64);
    mBottom->addBox(0.0f, 0.0f, 0.0f, 14, 10, 14, 0.0f);
    mBottom->mX = 1.0f;
    mBottom->mY = 6.0f;
    mBottom->mZ = 1.0f;

    mLid->compile(0.0625f);
    mLock->compile(0.0625f);
    mBottom->compile(0.0625f);
}

void ChestModel::render(bool useDisplayList)
{
    mLock->mXRot = mLid->mXRot;

    mLock->render(0.0625f, useDisplayList, false);
    mBottom->render(0.0625f, useDisplayList, false);

    GlStateManager::polygonOffset(-0.3f, -0.3f);

    mLid->render(0.0625f, useDisplayList, false);

    GlStateManager::polygonOffset(0.0f, 0.0f);
}
