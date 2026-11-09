#include "game/ui/FJ_CreativeMenu.hpp"

#include <string>

#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_CreativeMenu::FJ_CreativeMenu(fuiRenderNode* renderNode) : FJ_AbstractContainerDocument(renderNode)
{
    FJ_FuiNode* scrollBar = getChildByName(std::string("MainPanel.Scrollbar"));
    if (scrollBar != NULL)
    {
        getChildByName(std::string("MainPanel.PageNumber"))->asFJ_Base()->SetLabel(L"");

        m_scrollUpArrow = scrollBar->getChildByName(std::string("ScrollUpArrow"))->asFJ_ScrollArrow();
        m_scrollDownArrow = scrollBar->getChildByName(std::string("ScrollDownArrow"))->asFJ_ScrollArrow();
        m_scrollUpArrow->ChangeState(0);
        m_scrollDownArrow->ChangeState(3);
        m_currentPage = 0;
        m_scrollBarButtonPosition = static_cast<int>(scrollBar->getChildByName(std::string("ScrollBarButton"))->getY());
        m_scrollBarEndPosition = static_cast<int>(
            (scrollBar->getChildByName(std::string("ScrollBarTrack"))->getY()
             - scrollBar->getChildByName(std::string("ScrollBarButton"))->getY())
            + (scrollBar->getChildByName(std::string("ScrollBarTrack"))->getY()
               + scrollBar->getChildByName(std::string("ScrollBarTrack"))->getHeight())
            - scrollBar->getChildByName(std::string("ScrollBarButton"))->getHeight());
    }
    addEventListener(eFJEventType_SlideComplete, this, _handleSlideComplete, false);
}

FJ_CreativeMenu* FJ_CreativeMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_CreativeMenu(renderNode);
}

void FJ_CreativeMenu::handleSlideComplete(FJ_Event*)
{
    m_pCallbackScene->handleTouchBoxRebuild();
}

void FJ_CreativeMenu::_handleSlideComplete(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_CreativeMenu*>(node)->handleSlideComplete(event);
}

unsigned int FJ_CreativeMenu::getType()
{
    return 10;
}

void FJ_CreativeMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
        setY(static_cast<float>(top));
}

void FJ_CreativeMenu::SetActiveTab(int tab)
{
    int frame = tab + 1;
    m_activeTab = tab;
    if (frame < 1)
        frame = 1;
    else if (frame > 8)
        frame = 8;

    getChildByName(std::string("MainPanel.CreativeTabs"))->getChildAt(0)->gotoAndStop(frame);
    m_currentPage = 0;
    m_scrollUpArrow->setVisible(true);
    m_scrollDownArrow->setVisible(true);
}

void FJ_CreativeMenu::SetScrollBar(int pageCount, int currentPage)
{
    FJ_FuiNode* scrollBar = getChildByName(std::string("MainPanel.Scrollbar"));
    if (scrollBar == NULL)
        return;

    int lastPage = pageCount - 1;
    if (currentPage >= pageCount)
        currentPage = lastPage;
    else if (currentPage < 0)
        currentPage = 0;

    if (pageCount == 1)
    {
        scrollBar->setAlpha(0.5f);
        m_scrollUpArrow->setVisible(false);
        m_scrollDownArrow->setVisible(false);
    }
    else
    {
        scrollBar->setAlpha(1.0f);
        scrollBar->getChildByName(std::string("ScrollBarButton"))->setVisible(true);
        if (pageCount > 1)
        {
            if (m_currentPage > currentPage)
            {
                if (currentPage < lastPage)
                {
                    m_scrollUpArrow->setVisible(true);
                    m_scrollUpArrow->ChangeState(1);
                }
                m_scrollDownArrow->setVisible(true);
            }
            else if (m_currentPage < currentPage)
            {
                if (currentPage > 0)
                {
                    m_scrollDownArrow->setVisible(true);
                    m_scrollDownArrow->ChangeState(4);
                }
                m_scrollUpArrow->setVisible(true);
            }
        }
    }

    if (currentPage == lastPage)
        m_scrollDownArrow->setVisible(false);
    if (currentPage == 0)
        m_scrollUpArrow->setVisible(false);

    if (pageCount > 1)
    {
        scrollBar->getChildByName(std::string("ScrollBarButton"))
            ->setY(static_cast<float>(m_scrollBarButtonPosition)
                   + static_cast<float>(m_scrollBarEndPosition - m_scrollBarButtonPosition)
                         * (static_cast<float>(currentPage) / static_cast<float>(lastPage)));
        m_currentPage = currentPage;
    }
    else
    {
        scrollBar->getChildByName(std::string("ScrollBarButton"))->setY(static_cast<float>(m_scrollBarButtonPosition));
        m_currentPage = currentPage;
    }
}
