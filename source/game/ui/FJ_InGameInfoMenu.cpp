#include "game/ui/FJ_InGameInfoMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"

FJ_InGameInfoMenu::FJ_InGameInfoMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_InGameInfoMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_InGameInfoMenu(renderNode);
}

unsigned int FJ_InGameInfoMenu::getType()
{
    return 46;
}

void FJ_InGameInfoMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
    {
        setY(top);
    }
}
