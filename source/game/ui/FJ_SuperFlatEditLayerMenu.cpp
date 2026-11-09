#include "game/ui/FJ_SuperFlatEditLayerMenu.hpp"

#include <string>

#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

FJ_SuperFlatEditLayerMenu::FJ_SuperFlatEditLayerMenu(fuiRenderNode* renderNode)
    : FJ_AbstractContainerDocument(renderNode)
{
    FJ_FuiNode* scrollBar = getChildByName(std::string("MainPanel.Scrollbar"));
    FJ_FuiNode* button = scrollBar->getChildByName(std::string("ScrollBarButton"));
    FJ_FuiNode* track = scrollBar->getChildByName(std::string("ScrollBarTrack"));
    if (scrollBar != NULL)
    {
        mScrollUp = static_cast<FJ_ScrollArrow*>(
            FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, NULL));
        mScrollDown = static_cast<FJ_ScrollArrow*>(
            FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, NULL));
        scrollBar->addChild(mScrollUp);
        scrollBar->addChild(mScrollDown);
        mScrollUp->setX(FJ_FuiNode::asMovieClip(scrollBar)->getChildByName(std::string("ScrollUpArrow"))->getX());
        mScrollUp->setY(FJ_FuiNode::asMovieClip(scrollBar)->getChildByName(std::string("ScrollUpArrow"))->getY());
        mScrollDown->setX(FJ_FuiNode::asMovieClip(scrollBar)->getChildByName(std::string("ScrollDownArrow"))->getX());
        mScrollDown->setY(FJ_FuiNode::asMovieClip(scrollBar)->getChildByName(std::string("ScrollDownArrow"))->getY());
        FJ_FuiNode::asMovieClip(scrollBar)->removeChild(
            FJ_FuiNode::asMovieClip(scrollBar)->getChildByName(std::string("ScrollUpArrow")), true);
        FJ_FuiNode::asMovieClip(scrollBar)->removeChild(
            FJ_FuiNode::asMovieClip(scrollBar)->getChildByName(std::string("ScrollDownArrow")), true);
        mScrollUp->ChangeState(0);
        mScrollDown->ChangeState(3);
        mSlotID = 0;
        mScrollTop = (int)button->getY();
        mScrollBottom = (int)((track->getY() - button->getY()) + (track->getY() + track->getHeight())
                              - button->getHeight());
    }
    SetSlotID();
}

void FJ_SuperFlatEditLayerMenu::SetSlotID()
{
    FJ_FuiNode* panel = getChildByName(std::string("MainPanel"));
    FJ_FuiNode* slotControl = FJ_FuiNode::asMovieClip(panel->getChildByName(std::string("SlotControl")));
    if (slotControl != NULL)
    {
        fuiRect rect;
        rect.xMin = slotControl->getX();
        rect.yMin = slotControl->getY();
        rect.xMax = rect.xMin + slotControl->getWidth() / getScaleX();
        rect.yMax = rect.yMin + slotControl->getHeight();
        fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect,
                                                                            "iggy_layerslot_0");
        renderNode->setAlpha(0.25f);
        panel->removeChild(slotControl, true);
        FJ_FuiNode* slot = new FJ_FuiNode(renderNode);
        renderNode->setFuiNode(slot);
        panel->addChild(slot);
    }
}

FJ_AbstractContainerDocument* FJ_SuperFlatEditLayerMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_SuperFlatEditLayerMenu(renderNode);
}

unsigned int FJ_SuperFlatEditLayerMenu::getType()
{
    return 41;
}

void FJ_SuperFlatEditLayerMenu::SetScrollBar(int count, int position)
{
    FJ_FuiNode* scrollBar = getChildByName(std::string("MainPanel.Scrollbar"));
    FJ_FuiNode* button = scrollBar->getChildByName(std::string("ScrollBarButton"));
    if (scrollBar == NULL)
    {
        return;
    }
    int last = count - 1;
    int clamped = position;
    if (clamped >= count)
        clamped = last;
    else if (clamped < 0)
        clamped = 0;
    if (count == 1)
    {
        scrollBar->setAlpha(0.5f);
        mScrollUp->setVisible(false);
        mScrollDown->setVisible(false);
    }
    else
    {
        scrollBar->setAlpha(1.0f);
        button->setVisible(true);
        if (count > 1)
        {
            if (mSlotID > clamped)
            {
                if (clamped < last)
                {
                    mScrollUp->setVisible(true);
                    mScrollUp->ChangeState(1);
                }
                mScrollDown->setVisible(true);
            }
            else if (mSlotID < clamped)
            {
                if (clamped > 0)
                {
                    mScrollDown->setVisible(true);
                    mScrollDown->ChangeState(4);
                }
                mScrollUp->setVisible(true);
            }
        }
    }
    if (clamped == last)
    {
        mScrollDown->setVisible(false);
    }
    if (clamped == 0)
    {
        mScrollUp->setVisible(false);
    }
    if (count > 1)
    {
        button->setY((float)mScrollTop + (float)(mScrollBottom - mScrollTop) * ((float)clamped / (float)last));
    }
    else
    {
        button->setY((float)mScrollTop);
    }
    mSlotID = clamped;
}
