#include "game/client/render/ConduitCageModel.hpp"

#include "game/client/render/ModelPartSkin.hpp"

ConduitCageModel::ConduitCageModel(int texOffsX, int texOffsY, int texWidth, int texHeight)
{
    mCage = new ModelPartSkin(this, texOffsX, texOffsY, texWidth, texHeight);
    mCage->addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);

    compileSkinnedDrawlist();
}

void ConduitCageModel::renderCage(bool useDisplayList)
{
    mCage->render(0.0625f, useDisplayList, false);

    callSkinnedDrawlist();
}
