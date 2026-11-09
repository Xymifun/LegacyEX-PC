#include "game/ui/FJ_InGamePlayerOptions.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

void FJ_InGamePlayerOptions::handleCheckboxToggled(FJ_Event* event)
{
    FJ_CheckBox* checkBox = event->m_pTarget->asFJ_CheckBox();
    if (checkBox != NULL)
    {
        FJ_Base* base = static_cast<FJ_Base*>(checkBox);
        m_pCallbackScene->handleCheckboxToggled(base->getId(), checkBox->getChecked());
    }
}

void FJ_InGamePlayerOptions::_handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_InGamePlayerOptions*>(node)->handleCheckboxToggled(event);
}

FJ_InGamePlayerOptions::FJ_InGamePlayerOptions(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    addEventListener(eFJEventType_CheckboxToggled, this, _handleCheckboxToggled, false);
}

FJ_Document* FJ_InGamePlayerOptions::Create(fuiRenderNode* renderNode)
{
    return new FJ_InGamePlayerOptions(renderNode);
}

unsigned int FJ_InGamePlayerOptions::getType()
{
    return 44;
}

void FJ_InGamePlayerOptions::InitMenu()
{
    FJ_FuiNode* background = getChildByName(std::string("BackgroundPanel"));
    FJ_FuiNode* list = getChildByName(std::string("PlayerOptionsList"));
    FJ_FuiNode* gamertag = getChildByName(std::string("Gamertag"));
    FJ_FuiNode* playerIcon = getChildByName(std::string("PlayerIcon"));

    const int backgroundHeight = static_cast<int>(background->getChildAt(0)->getHeight());
    if (static_cast<float>(list->asFJ_List_MultiList()->GetListHeight()) < list->getHeight())
    {
        FJ_FuiNode* backgroundItem = background->getChildAt(0);
        backgroundItem->setHeight(
            static_cast<float>(list->asFJ_List_MultiList()->GetListHeight()) + (list->getY() - background->getY())
            + ((background->getY() + background->getHeight()) - (list->getY() + list->getHeight())));

        const int amount = Math::round((static_cast<float>(backgroundHeight) - background->getChildAt(0)->getHeight())
                                       * 0.5f);
        if (amount > 0)
        {
            const float offset = static_cast<float>(amount);
            gamertag->incY(offset);
            playerIcon->incY(offset);
            background->getChildAt(0)->incY(offset);
            list->incY(offset);
        }
    }
    SetFocus(-1);
}

void FJ_InGamePlayerOptions::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() != 360.0f && getStage()->getStageHeight() != 540.0f)
        return;
    setY(top);
}

void FJ_InGamePlayerOptions::SetPlayerIcon(int icon)
{
    FJ_FuiNode* playerIcon = getChildByName(std::string("PlayerIcon"));
    const int frame = icon + 1;
    FJ_FuiNode* movieClip = FJ_FuiNode::asMovieClip(playerIcon->getChildAt(0));
    movieClip->gotoAndStop(frame);
}
