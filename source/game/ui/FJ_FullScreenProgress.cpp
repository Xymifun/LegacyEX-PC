#include "game/ui/FJ_FullScreenProgress.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_FullScreenProgress::FJ_FullScreenProgress(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_FullScreenProgress::Create(fuiRenderNode* renderNode)
{
    return new FJ_FullScreenProgress(renderNode);
}

unsigned int FJ_FullScreenProgress::getType()
{
    return 13;
}

void FJ_FullScreenProgress::ShowConfirmationButton(bool show)
{
    FJ_FuiNode* confirmation = getChildByName("Confirm");

    if (confirmation != NULL)
    {
        confirmation->setVisible(show);
    }
}

void FJ_FullScreenProgress::SetLoadingScreenType(int type)
{
    gotoAndStop(type + 1);
}
