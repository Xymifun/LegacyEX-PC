#include "game/ui/FJ_Panel9GridFade.hpp"

FJ_Panel9GridFade::FJ_Panel9GridFade(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
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

FJ_FuiNode* FJ_Panel9GridFade::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Panel9GridFade(pRenderNode);
}

void FJ_Panel9GridFade::ChangeState(int iState)
{
    (void)iState;
}

unsigned int FJ_Panel9GridFade::getType()
{
    return 0x80b1;
}
