#include "game/ui/FJ_InGameHostOptions.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_InGameHostOptions::FJ_InGameHostOptions(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_InGameHostOptions::Create(fuiRenderNode* renderNode)
{
    return new FJ_InGameHostOptions(renderNode);
}

unsigned int FJ_InGameHostOptions::getType()
{
    return 0x2d;
}

void FJ_InGameHostOptions::InitMenu()
{
    FJ_FuiNode* background = getChildByName(std::string("BackgroundPanel"));
    FJ_FuiNode* list = getChildByName(std::string("HostOptionsList"));
    const int backgroundHeight = static_cast<int>(background->getChildAt(0)->getHeight());
    background->getChildAt(0)->setHeight(
        static_cast<float>(static_cast<FJ_List*>(list->asFJ_List_MultiList())->GetListHeight())
        + (list->getY() - background->getY()) * 2.0f);

    const int amount = Math::round((static_cast<float>(backgroundHeight) - background->getChildAt(0)->getHeight())
                                   * 0.5f);
    if (amount > 0)
    {
        background->getChildAt(0)->incY(static_cast<float>(amount));
        list->incY(static_cast<float>(amount));
    }
    SetFocus(-1);
}

void FJ_InGameHostOptions::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        return;

    setY(top);
}
