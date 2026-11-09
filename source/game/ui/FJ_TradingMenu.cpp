#include "game/ui/FJ_TradingMenu.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_HtmlTextPanel.hpp"
#include "game/ui/FJ_ScrollArrowH.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_TradingMenu::FJ_TradingMenu(fuiRenderNode* renderNode)
    : FJ_Document(renderNode),
      m_scrollRight(0),
      m_scrollLeft(0),
      m_slotWidth(0),
      m_visibleSlots(0),
      m_selectorSlot(0)
{
}

FJ_Document* FJ_TradingMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_TradingMenu(renderNode);
}

unsigned int FJ_TradingMenu::getType()
{
    return 78;
}

void FJ_TradingMenu::init(FJ_Event* event)
{
    FJ_FuiNode* mainPanel = getChildByName("MainPanel");
    FJ_FuiNode* tradingBar = mainPanel->getChildByName("TradingBar");
    FJ_FuiNode* selector = mainPanel->getChildByName("TradingSelector");
    FJ_FuiNode* offer = mainPanel->getChildByName("OfferDescription");

    m_slotWidth = tradingBar->asFJ_SlotList()->m_slotSize;
    m_visibleSlots = Math::round(tradingBar->getWidth() / static_cast<float>(m_slotWidth));
    m_selectorSlot = 0;

    selector->setX(tradingBar->getX()
                   - static_cast<float>(Math::round((selector->getWidth() - static_cast<float>(m_slotWidth)) * 0.5f))
                   + static_cast<float>(m_selectorSlot * m_slotWidth));
    selector->setY(tradingBar->getY()
                   - static_cast<float>(Math::round((selector->getHeight() - static_cast<float>(m_slotWidth)) * 0.5f)));

    m_scrollRight = mainPanel->getChildByName("ScrollRight");
    m_scrollLeft = mainPanel->getChildByName("ScrollLeft");
    static_cast<FJ_ScrollArrowH*>(m_scrollRight)->ChangeState(2);
    static_cast<FJ_ScrollArrowH*>(m_scrollLeft)->ChangeState(0);

    offer->setX(selector->getX() + selector->getWidth());
    offer->setY(selector->getY());

    fuiRect leftRect = m_scrollLeft->getRect(m_scrollLeft);
    if (m_scrollLeft->getX() + leftRect.xMin * m_scrollLeft->getScaleX() + m_scrollLeft->getWidth()
        > tradingBar->getX())
    {
        fuiRect rightRect = m_scrollRight->getRect(m_scrollRight);
        const int overlap = static_cast<int>((m_scrollRight->getX() + rightRect.xMin * m_scrollRight->getScaleX())
                                             - (tradingBar->getX() + tradingBar->getWidth()));
        fuiRect currentLeftRect = m_scrollLeft->getRect(m_scrollLeft);
        m_scrollLeft->setX(tradingBar->getX() - m_scrollLeft->getWidth()
                           - currentLeftRect.xMin * m_scrollLeft->getScaleX() - static_cast<float>(overlap));
    }
}

void FJ_TradingMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
    {
        setX(-right);
        setY(top);
    }
}

void FJ_TradingMenu::ScrollRight()
{
    static_cast<FJ_ScrollArrowH*>(m_scrollRight)->ChangeState(3);
}

void FJ_TradingMenu::ScrollLeft()
{
    static_cast<FJ_ScrollArrowH*>(m_scrollLeft)->ChangeState(1);
}

void FJ_TradingMenu::PositionOfferDescription()
{
    FJ_FuiNode* offer = getChildByName("MainPanel.OfferDescription");
    FJ_FuiNode* selector = getChildByName("MainPanel.TradingSelector");
    if (m_selectorSlot < m_visibleSlots / 2)
    {
        offer->setX(Math::round(selector->getX() + selector->getWidth() * 1.1f));
    }
    else
    {
        offer->setX(Math::round(selector->getX() - offer->getWidth() - selector->getWidth() * 0.1f));
    }
    offer->setY(Math::round(selector->getY() + selector->getHeight() * 0.5f - offer->getHeight() * 0.5f));
}

void FJ_TradingMenu::MoveSelector(bool right)
{
    FJ_FuiNode* mainPanel = getChildByName("MainPanel");
    FJ_FuiNode* tradingBar = mainPanel->getChildByName("TradingBar");
    FJ_FuiNode* selector = mainPanel->getChildByName("TradingSelector");

    if (right)
    {
        if (m_selectorSlot < m_visibleSlots - 1)
        {
            ++m_selectorSlot;
        }
        else if (m_scrollRight->isVisible())
        {
            ScrollRight();
        }
    }
    else
    {
        if (m_selectorSlot > 0)
        {
            --m_selectorSlot;
        }
        else if (m_scrollLeft->isVisible())
        {
            ScrollLeft();
        }
    }

    selector->setX(tradingBar->getX()
                   - static_cast<float>(Math::round((selector->getWidth() - static_cast<float>(m_slotWidth)) * 0.5f))
                   + static_cast<float>(m_selectorSlot * m_slotWidth));
    selector->setY(tradingBar->getY()
                   - static_cast<float>(Math::round((selector->getHeight() - static_cast<float>(m_slotWidth)) * 0.5f)));
    PositionOfferDescription();
}

void FJ_TradingMenu::SetSelectorSlot(int slot)
{
    FJ_FuiNode* mainPanel = getChildByName("MainPanel");
    FJ_FuiNode* tradingBar = mainPanel->getChildByName("TradingBar");
    FJ_FuiNode* selector = mainPanel->getChildByName("TradingSelector");

    m_selectorSlot = slot;
    selector->setX(tradingBar->getX()
                   - static_cast<float>(Math::round((selector->getWidth() - static_cast<float>(m_slotWidth)) * 0.5f))
                   + static_cast<float>(m_selectorSlot * m_slotWidth));
    selector->setY(tradingBar->getY()
                   - static_cast<float>(Math::round((selector->getHeight() - static_cast<float>(m_slotWidth)) * 0.5f)));
    PositionOfferDescription();
}

void FJ_TradingMenu::SetOfferDescription(const std::wstring& description)
{
    getChildByName("MainPanel.OfferDescription")->asFJ_HtmlTextPanel()->SetLabel(description);
    PositionOfferDescription();
}

void FJ_TradingMenu::ShowScrollRightArrow(bool show)
{
    m_scrollRight->setVisible(show);
}

void FJ_TradingMenu::ShowScrollLeftArrow(bool show)
{
    m_scrollLeft->setVisible(show);
}

int FJ_TradingMenu::getTotalTradingSlots()
{
    return m_visibleSlots;
}
