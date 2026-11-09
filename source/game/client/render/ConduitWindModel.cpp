#include "game/client/render/ConduitWindModel.hpp"

#include "game/client/render/ModelPartSkin.hpp"

ConduitWindModel::ConduitWindModel(int xTexOffs, int yTexOffs, int texWidth, int texHeight)
{
    mWind = new ModelPartSkin(this, xTexOffs, yTexOffs, texWidth, texHeight);
    mWind->addBox(-8.0f, -8.0f, -8.0f, 16, 16, 16);

    compileSkinnedDrawlist();
}

void ConduitWindModel::renderWind(bool useDisplayList)
{
    mWind->render(0.0625f, useDisplayList, false);

    callSkinnedDrawlist();
}
