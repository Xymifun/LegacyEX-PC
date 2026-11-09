#include "game/ui/FJ_HopperMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_HopperMenu::FJ_HopperMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_AbstractContainerDocument* FJ_HopperMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_HopperMenu(renderNode);
}

unsigned int FJ_HopperMenu::getType()
{
    return 0x23;
}

void FJ_HopperMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        return;

    setX(-right);
    setY(top);
}
