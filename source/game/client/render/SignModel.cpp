#include "game/client/render/SignModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"

SignModel::SignModel(const ModelDefinition& modelDefinition)
{
    mSignBoard = new ModelPartSkin(this, 0, 0, NULL);
    mSignBoard->addBox(-12.0f, -14.0f, -1.0f, 24, 12, 2, 0.0f);

    mSignStick = new ModelPartSkin(this, 0, 14, NULL);
    mSignStick->addBox(-1.0f, -2.0f, -1.0f, 2, 14, 2, 0.0f);

    compileSkinnedDrawlist();
}

void SignModel::render(bool useDisplayList)
{
    mSignBoard->render(0.0625f, useDisplayList, false);
    mSignStick->render(0.0625f, useDisplayList, false);

    callSkinnedDrawlist();
}
