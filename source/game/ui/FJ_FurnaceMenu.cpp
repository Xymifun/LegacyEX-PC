#include "game/ui/FJ_FurnaceMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_FurnaceMenu::FJ_FurnaceMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_AbstractContainerDocument* FJ_FurnaceMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_FurnaceMenu(renderNode);
}

unsigned int FJ_FurnaceMenu::getType()
{
    return 34;
}

void FJ_FurnaceMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        return;

    setX(-right);
    setY(top);
}
