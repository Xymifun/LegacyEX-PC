#include "game/ui/FJ_MobEffect_Tile.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"

FJ_MobEffect_Tile::FJ_MobEffect_Tile(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
}

void FJ_MobEffect_Tile::Init()
{
    FJ_FuiNode* pPanel = getChildByName("PointerTextPanel")->asMovieClip();

    int iWidth = (int)pPanel->getWidth();
    int iHeight = (int)pPanel->getHeight();

    pPanel->setScaleX(1.0f);
    pPanel->setScaleY(1.0f);
    pPanel->setX(0.0f);
    pPanel->setY(0.0f);

    pPanel->getChildAt(0)->setWidth((float)iWidth);
    pPanel->getChildAt(0)->setHeight((float)iHeight);

    setVisible(true);
}

void FJ_MobEffect_Tile::SetupEffectType(int iEffectType)
{
    getChildByName("Icon")->gotoAndStop(iEffectType);
}

void FJ_MobEffect_Tile::SetEffectAlpha(float fAlpha)
{
    getChildByName("Icon")->setAlpha(fAlpha);
}
