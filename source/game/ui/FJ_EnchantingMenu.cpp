#include "game/ui/FJ_EnchantingMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_EnchantingMenu::FJ_EnchantingMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_AbstractContainerDocument* FJ_EnchantingMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_EnchantingMenu(renderNode);
}

unsigned int FJ_EnchantingMenu::getType()
{
    return 33;
}

void FJ_EnchantingMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        return;

    setX(-right);
    setY(top);
}
