#include "game/ui/FJ_PanelRecessFade.hpp"

FJ_PanelRecessFade::FJ_PanelRecessFade(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
    float width = getWidth();
    float height = getHeight();
    FJ_FuiNode* child = getChildAt(0);
    FJ_FuiNode* panel = child->asMovieClip();
    FJ_FuiNode* content = panel->getChildAt(0)->asMovieClip();
    setScaleX(1.0f);
    setScaleY(1.0f);
    content->setWidth(width);
    content->setHeight(height);
}

FJ_FuiNode* FJ_PanelRecessFade::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_PanelRecessFade(pRenderNode);
}

void FJ_PanelRecessFade::ChangeState(int iState)
{
    gotoAndStop(iState);
}

unsigned int FJ_PanelRecessFade::getType()
{
    return 0x80b2;
}
