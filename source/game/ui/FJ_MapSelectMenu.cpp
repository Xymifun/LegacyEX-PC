#include "game/ui/FJ_MapSelectMenu.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_MapSelectMenu::FJ_MapSelectMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_MapSelectMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_MapSelectMenu(renderNode);
}

unsigned int FJ_MapSelectMenu::getType()
{
    return 48;
}

void FJ_MapSelectMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        return;

    setY(top);
}
