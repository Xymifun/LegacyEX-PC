#include "game/ui/FJ_AbstractContainerDocument.hpp"

#include "game/ui/FJ_Cursor.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_MouseMoveEvent.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_AbstractContainerDocument::FJ_AbstractContainerDocument(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

void FJ_AbstractContainerDocument::handleMouseMove(FJ_Event* event)
{
    FJ_MouseMoveEvent* mouseEvent = static_cast<FJ_MouseMoveEvent*>(event);
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"))->asMovieClip();
    FJ_Cursor* cursor = mainPanel->getChildByName(std::string("cursor"))->asFJ_Cursor();
    cursor->setX(mouseEvent->m_fX);
    cursor->setY(mouseEvent->m_fY);
    cursor->UpdatePanelAlignment();
}

void FJ_AbstractContainerDocument::_handleMouseMove(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_AbstractContainerDocument*>(node)->handleMouseMove(event);
}

void FJ_AbstractContainerDocument::init(FJ_Event* event)
{
    getStage()->addEventListener(eFJEventType_MouseMove, this, _handleMouseMove, false);
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"))->asMovieClip();
    FJ_Cursor* cursor = mainPanel->getChildByName(std::string("cursor"))->asFJ_Cursor();
    cursor->m_panelOffsetX = static_cast<int>(mainPanel->getX());
}

void FJ_AbstractContainerDocument::HideInventory()
{
    FJ_FuiNode* mainPanel = getChildByName(std::string("MainPanel"))->asMovieClip();
    FJ_SlotList* inventory = mainPanel->getChildByName(std::string("inventoryList"))->asFJ_SlotList();
    FJ_SlotList* hotbar = mainPanel->getChildByName(std::string("hotbarList"))->asFJ_SlotList();

    if (mainPanel != 0 && inventory != 0 && hotbar != 0)
    {
        int oldHeight = mainPanel->getRenderNodeChildAt(0)->getHeight();

        if (FJ_FuiNode::asMovieClip(mainPanel->getChildByName(std::string("BackgroundPanel")))->getChildAt(0) != 0)
        {
            FJ_FuiNode::asMovieClip(mainPanel->getChildByName(std::string("BackgroundPanel")))
                ->getChildAt(0)
                ->decHeight(inventory->FJ_SlotList::getHeight()
                            + (hotbar->getY() - (inventory->getY() + inventory->FJ_SlotList::getHeight())));
            hotbar->setY(inventory->getY());
            mainPanel->removeChild(inventory, true);
            incY(Math::round((oldHeight - mainPanel->getRenderNodeChildAt(0)->getHeight()) * 0.5f));
        }
        else
        {
            mainPanel->getChildAt(0)->decHeight(
                inventory->FJ_SlotList::getHeight()
                + (hotbar->getY() - (inventory->getY() + inventory->FJ_SlotList::getHeight())));
            hotbar->setY(inventory->getY());
            mainPanel->removeChild(inventory, true);
            incY(Math::round((oldHeight - mainPanel->getRenderNodeChildAt(0)->getHeight()) * 0.5f));
        }
    }
}
