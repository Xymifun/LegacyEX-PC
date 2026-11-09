#include "game/client/render/ConduitBaseModel.hpp"

#include "game/client/render/ModelPartSkin.hpp"

ConduitBaseModel::ConduitBaseModel(int xTexOffs, int yTexOffs, int texWidth, int texHeight)
{
    mBase = new ModelPartSkin(this, xTexOffs, yTexOffs, texWidth, texHeight);
    mBase->addBox(-3.0f, -3.0f, -3.0f, 6, 6, 6);

    compileSkinnedDrawlist();
}

void ConduitBaseModel::renderBase(bool useDisplayList)
{
    mBase->render(0.0625f, useDisplayList, false);

    callSkinnedDrawlist();
}
