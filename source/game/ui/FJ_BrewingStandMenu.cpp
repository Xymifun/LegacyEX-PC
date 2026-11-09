#include "game/ui/FJ_BrewingStandMenu.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_BrewingStandMenu::FJ_BrewingStandMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_BrewingStandMenu* FJ_BrewingStandMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_BrewingStandMenu(renderNode);
}

unsigned int FJ_BrewingStandMenu::getType()
{
    return 0x1d;
}

void FJ_BrewingStandMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        return;

    setX(static_cast<float>(-right));
    setY(static_cast<float>(top));
}
