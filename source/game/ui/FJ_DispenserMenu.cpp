#include "game/ui/FJ_DispenserMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_DispenserMenu::FJ_DispenserMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
}

FJ_AbstractContainerDocument* FJ_DispenserMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_DispenserMenu(renderNode);
}

unsigned int FJ_DispenserMenu::getType()
{
    return 0x20;
}

void FJ_DispenserMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
    {
        setX(-right);
        setY(top);
    }
}
