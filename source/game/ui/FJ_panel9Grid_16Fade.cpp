#include "game/ui/FJ_panel9Grid_16Fade.hpp"

FJ_panel9Grid_16Fade::FJ_panel9Grid_16Fade(fuiRenderNode* renderNode) : FJ_FuiNode(renderNode)
{
    float width = getWidth();
    float height = getHeight();
    FJ_FuiNode* panel = getChildAt(0)->asMovieClip();
    FJ_FuiNode* content = panel->getChildAt(0)->asMovieClip();
    setScaleX(1.0f);
    setScaleY(1.0f);
    content->setWidth(width);
    content->setHeight(height);
}

FJ_FuiNode* FJ_panel9Grid_16Fade::Create(fuiRenderNode* renderNode)
{
    return new FJ_panel9Grid_16Fade(renderNode);
}

void FJ_panel9Grid_16Fade::ChangeState(int state)
{
    (void)state;
}

unsigned int FJ_panel9Grid_16Fade::getType()
{
    return 0x80b0;
}
