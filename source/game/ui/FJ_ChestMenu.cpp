#include "game/ui/FJ_ChestMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_ChestMenu::FJ_ChestMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_AbstractContainerDocument* FJ_ChestMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_ChestMenu(renderNode);
}

unsigned int FJ_ChestMenu::getType()
{
    return 31;
}

void FJ_ChestMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        return;

    setX(static_cast<float>(-right));
    setY(static_cast<float>(top));
}
