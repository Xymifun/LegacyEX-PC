#include "game/ui/FJ_DynamicHtmlText.hpp"

#include <cstdio>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/util/Math.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_DynamicHtmlText::FJ_DynamicHtmlText(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
    m_pHtmlTextPanel = NULL;
}

FJ_FuiNode* FJ_DynamicHtmlText::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_DynamicHtmlText(pRenderNode);
}

FJ_DynamicHtmlText::~FJ_DynamicHtmlText()
{
    if (m_pUpArrow != NULL && m_pUpArrow->getParent() != NULL)
        m_pUpArrow->getParent()->removeChild(m_pUpArrow, true);
    if (m_pDownArrow != NULL && m_pDownArrow->getParent() != NULL)
        m_pDownArrow->getParent()->removeChild(m_pDownArrow, true);
    removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
}

int FJ_DynamicHtmlText::getActiveLabelHeight() const
{
    int height = m_iTopMargin * 2 + m_iLabelSpacing;
    if (!m_labels.empty())
    {
        std::vector<FJ_Label*>::const_iterator last = m_labels.end();
        --last;
        height += (int)(*last)->getY();
        height += (int)(*last)->getHeight();
    }
    return height;
}

void FJ_DynamicHtmlText::UpdateTouchFlickScrolling()
{
    if (m_bTouchScrolling || !m_bTouchFlick)
        return;

    if (m_iFlickOffset < 0)
    {
        m_iFlickOffset += 5;
        if (m_iFlickOffset > 0)
            m_iFlickOffset = 0;
        if (m_iScrollDirection != -1)
        {
            m_pUpArrow->ChangeState(2);
            m_pDownArrow->ChangeState(3);
        }
        if (m_pUpArrow->isVisible())
        {
            m_iScrollDirection = -1;
            m_iTargetDirection = -1;
            m_iTouchScroll = -1;
            return;
        }
        m_iScrollDirection = 0;
        m_iCurrentTouchPosition = m_iTouchStart;
    }
    else if (m_iFlickOffset > 0)
    {
        m_iFlickOffset -= 5;
        if (m_iFlickOffset < 0)
            m_iFlickOffset = 0;
        if (m_iScrollDirection != 1)
        {
            m_pDownArrow->ChangeState(5);
            m_pUpArrow->ChangeState(0);
        }
        if (m_pDownArrow->isVisible())
        {
            m_iScrollDirection = 1;
            m_iTargetDirection = 1;
            m_iTouchScroll = 1;
            return;
        }
        m_iScrollDirection = 0;
        m_iCurrentTouchPosition = m_iTouchStart;
    }
    else
    {
        m_bTouchFlick = false;
        m_iCurrentTouchPosition = 0;
        m_iTouchStart = 0;
        m_iScrollDirection = 0;
        m_pUpArrow->ChangeState(0);
        m_pDownArrow->ChangeState(3);
    }
}

unsigned int FJ_DynamicHtmlText::SetNextLabel(int index, bool append)
{
    int slot = 0;
    FJ_Label* label = m_pLabels[slot];
    while (label->isVisible())
    {
        if (++slot >= 20)
            return 0;
        label = m_pLabels[slot];
    }

    label->setVisible(true);
    label->SetLabel(m_text[index]);
    if (append)
    {
        if (!m_labels.empty())
            label->setY(m_labels.back()->getY() + m_labels.back()->getHeight());
        else
            label->setY(0.0f);
        m_labels.push_back(label);
        m_labelTextIndices.push_back(index);
        return 1;
    }

    if (!m_labels.empty())
        label->setY(Math::round(m_labels.front()->getY() - label->getHeight()));
    m_labels.insert(m_labels.begin(), label);
    m_labelTextIndices.insert(m_labelTextIndices.begin(), index);
    return 1;
}

void FJ_DynamicHtmlText::HandleScrollListArrows(bool positionArrows)
{
    if (positionArrows)
    {
        getParent()->addChild(m_pUpArrow);
        getParent()->addChild(m_pDownArrow);
        m_pUpArrow->setX(Math::round(((getX() - m_iTopMargin) + m_fRealWidth) - m_pUpArrow->getWidth() * 1.5));
        m_pUpArrow->setY(Math::round((getY() - m_iTopMargin * 0.5) + m_fRealHeight + m_pUpArrow->getHeight() * 0.5f));
        m_pDownArrow->setX(Math::round(((getX() - m_iTopMargin) + m_fRealWidth) - m_pUpArrow->getWidth() * 0.5f));
        m_pDownArrow->setY(m_pUpArrow->getY());
        m_pUpArrow->ChangeState(0);
        m_pDownArrow->ChangeState(3);
    }

    bool reachedLastText = false;
    if (m_labels[m_labels.size() - 1]->getY() + m_labels[m_labels.size() - 1]->getHeight() + m_iLabelSpacing
            + m_iTopMargin
        < m_fRealHeight)
    {
        if (m_labelTextIndices[m_labels.size() - 1] == static_cast<int>(m_text.size()) - 1)
            reachedLastText = true;
    }

    m_pUpArrow->setVisible(m_bListReady == false);
    m_pDownArrow->setVisible(reachedLastText ? false : !m_bAtEnd);

    if (!m_pUpArrow->isVisible() && !m_pDownArrow->isVisible())
        removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
}

void FJ_DynamicHtmlText::UpdateScrolling(FJ_Event*)
{
    bool reachedBoundary = false;
    bool moving = false;
    int delta = 0;
    UpdateTouchFlickScrolling();

    if (m_iTargetDirection > 0 || m_iTouchScroll > 0)
    {
        if (m_fRealHeight <= static_cast<float>(getActiveLabelHeight()))
        {
            m_iTouchScroll = 0;
            m_bListReady = false;
        }
        else if (m_labelTextIndices[m_labelTextIndices.size() - 1] + 1 < m_text.size())
        {
            SetNextLabel(m_labelTextIndices[m_labelTextIndices.size() - 1] + 1, true);
            m_bListReady = false;
        }
        else
        {
            reachedBoundary = true;
            m_bAtEnd = true;
            m_bListReady = false;
        }
    }
    else if (m_iTargetDirection < 0 || m_iTouchScroll < 0)
    {
        if (m_labels.front()->getY() <= 0.0f)
        {
            m_iTouchScroll = 0;
            m_bAtEnd = false;
        }
        else if (m_labelTextIndices.front() < 1)
        {
            m_bListReady = true;
            m_bAtEnd = false;
            reachedBoundary = true;
        }
        else
        {
            SetNextLabel(m_labelTextIndices.front() - 1, false);
            m_bAtEnd = false;
        }
    }

    if (!reachedBoundary)
    {
        int step = m_bKeyboardScroll ? static_cast<int>(m_fRealHeight - static_cast<float>(m_iCharacterHeight)
                                                        - static_cast<float>(m_iTopMargin * 2))
                                     : m_iCharacterHeight;
        if (m_iScrollDirection < 0)
        {
            if (m_iTargetScrollPosition < m_iContentHeight)
                m_iTargetScrollPosition += step;
            moving = true;
        }
        else if (m_iScrollDirection > 0)
        {
            if (m_iContentHeight < m_iTargetScrollPosition)
                m_iTargetScrollPosition -= step;
            moving = true;
        }
    }
    if ((m_iTargetDirection > 0 && m_iTargetScrollPosition < m_iContentHeight)
        || (m_iTargetDirection < 0 && m_iContentHeight < m_iTargetScrollPosition))
        moving = true;

    if (reachedBoundary || !moving)
    {
        m_iContentHeight = 0;
        m_iTargetScrollPosition = 0;
        m_iTargetDirection = 0;
        m_iScrollDelay = 30;
        m_iFlickDelay = 30;
    }
    else
    {
        float acceleration = 1.0f;
        if (m_iScrollDelay < 1)
        {
            if (m_iFlickDelay >= 0)
            {
                if (m_iFlickDelay > 0)
                    --m_iFlickDelay;
                acceleration += static_cast<float>(30 - m_iFlickDelay) / 30.0f;
            }
        }
        else
        {
            --m_iScrollDelay;
        }

        if (m_bTouchScrolling || m_bTouchFlick)
        {
            delta = m_bTouchScrolling ? m_iCurrentTouchPosition - m_iTouchStart : m_iFlickOffset;
            if (delta < -75)
                delta = -75;
            if (delta > 75)
                delta = 75;
            int activeHeight = getActiveLabelHeight();
            if (static_cast<float>(activeHeight - delta) < m_fRealHeight
                && m_labelTextIndices[m_labelTextIndices.size() - 1] == static_cast<int>(m_text.size()) - 1)
            {
                delta = activeHeight - static_cast<int>(m_fRealHeight);
                if (m_labels.size() > 1)
                {
                    m_iTargetDirection = 0;
                    m_bAtEnd = true;
                }
            }
            if (m_bTouchScrolling)
                m_iFlickOffset = delta;
        }
        else
        {
            delta = Math::round((160.0f * m_fScrollSpeed / 30.0f) * static_cast<float>(m_iTargetDirection)
                                * acceleration);
        }
        if (m_labelTextIndices.front() == 0 && m_labels.front()->getY() - static_cast<float>(delta) > 0.0f)
        {
            m_iTargetDirection = 0;
            m_bListReady = true;
            delta = static_cast<int>(m_labels.front()->getY());
        }
        for (std::vector<FJ_Label*>::iterator label = m_labels.begin(); label != m_labels.end(); ++label)
            (*label)->setY((*label)->getY() - static_cast<float>(delta));
        m_iContentHeight -= delta;
        if (m_bTouchScrolling)
        {
            if (m_iCurrentTouchPosition < m_iTouchStart)
            {
                m_iCurrentTouchPosition -= delta;
                if (m_iCurrentTouchPosition > m_iTouchStart)
                    m_iCurrentTouchPosition = m_iTouchStart;
            }
            else if (m_iCurrentTouchPosition > m_iTouchStart)
            {
                m_iCurrentTouchPosition -= delta;
                if (m_iCurrentTouchPosition < m_iTouchStart)
                    m_iCurrentTouchPosition = m_iTouchStart;
            }
        }
    }

    HandleScrollListArrows(false);
    if (m_labels.size() <= 1)
        return;

    int correction = 0;
    if (m_labelTextIndices.front() == 0 && m_labels.front()->getY() > 0.0f)
        correction = static_cast<int>(m_labels.front()->getY());
    int activeHeight = getActiveLabelHeight();
    if (activeHeight - delta < static_cast<int>(m_fRealHeight)
        && m_labelTextIndices[m_labelTextIndices.size() - 1] == static_cast<int>(m_text.size()) - 1)
        correction = activeHeight - delta - static_cast<int>(m_fRealHeight);

    if (correction != 0)
    {
        for (std::vector<FJ_Label*>::iterator label = m_labels.begin(); label != m_labels.end(); ++label)
            (*label)->setY((*label)->getY() - static_cast<float>(correction));
        m_bKeyboardScroll = false;
        m_bTouchScrolling = false;
        m_iScrollDirection = 0;
        m_iTargetDirection = 0;
        m_iCurrentTouchPosition = 0;
        m_iTouchStart = 0;
        m_iContentHeight = 0;
        m_bTouchFlick = false;
        m_iFlickOffset = 0;
        m_iTouchScroll = 0;
        m_iTargetScrollPosition = 0;
        return;
    }

    for (int attempt = 0; attempt < 20; ++attempt)
    {
        bool added = false;
        if (m_iScrollDirection < 0)
        {
            if (m_labels.front()->getY() > 0.0f && m_labelTextIndices.front() > 0)
            {
                SetNextLabel(m_labelTextIndices.front() - 1, false);
                added = true;
            }
            if (m_labels[m_labels.size() - 1]->getY() > m_fRealHeight)
            {
                m_labels[m_labels.size() - 1]->SetLabel(L"");
                m_labels[m_labels.size() - 1]->setVisible(false);
                m_labels[m_labels.size() - 1]->setY(m_labels[m_labels.size() - 1]->getY()
                                                    - getStage()->getStageHeight());
                m_labels.pop_back();
                m_labelTextIndices.pop_back();
            }
        }
        else if (m_iScrollDirection > 0)
        {
            if (m_labels[m_labels.size() - 1]->getY() + m_labels[m_labels.size() - 1]->getHeight() < m_fRealHeight
                && m_labelTextIndices[m_labelTextIndices.size() - 1] + 1 < m_text.size())
            {
                SetNextLabel(m_labelTextIndices[m_labelTextIndices.size() - 1] + 1, true);
                added = true;
            }
            if (m_labels.front()->getY() + m_labels.front()->getHeight() + 10.0f < 0.0f)
            {
                m_labels.front()->SetLabel(L"");
                m_labels.front()->setVisible(false);
                m_labels.erase(m_labels.begin());
                m_labelTextIndices.erase(m_labelTextIndices.begin());
            }
        }
        if (!added)
            break;
    }
}

void FJ_DynamicHtmlText::_UpdateScrolling(FJ_FuiNode* node, FJ_Event* event)
{
    node->asFJ_DynamicHtmlText()->UpdateScrolling(event);
}

void FJ_DynamicHtmlText::Init()
{
    m_fRealWidth = getWidth();
    m_fRealHeight = getHeight() - m_pUpArrow->getHeight();

    FJ_FuiNode* background = getChildByName(std::string("HtmlBackground"));
    if (background != NULL)
    {
        removeChild(background, true);
        m_iTopMargin = 0;
    }

    FJ_FuiNode* panel = getChildByName(std::string("HtmlTextPanel"));
    if (panel != NULL)
    {
        float width = getWidth();
        float height = getHeight();
        m_pHtmlTextPanel = panel->getChildAt(0);
        panel->removeChild(m_pHtmlTextPanel, false);
        removeChild(panel, true);
        getParent()->addChild(m_pHtmlTextPanel);
        m_pHtmlTextPanel->setX(getX());
        m_pHtmlTextPanel->setY(getY());
        m_pHtmlTextPanel->setWidth(width);
        m_pHtmlTextPanel->setHeight(height);
    }

    m_pContentContainer = FJ_Factory::createObject("", m_pRenderNode, true, NULL);
    getParent()->addChild(m_pContentContainer);
    m_pContentContainer->setX(getX() + (float)m_iTopMargin);
    m_pContentContainer->setY(getY() + (float)m_iTopMargin);
    m_scrollRect.xMin = 0.0f;
    m_scrollRect.yMax = m_scrollRect.yMax - m_scrollRect.yMin;
    m_scrollRect.yMin = 0.0f;
    m_scrollRect.xMax = m_fRealWidth;
    m_scrollRect.yMax = m_scrollRect.yMin + (m_fRealHeight - Math::round(m_iTopMargin * 1.4));
    m_pContentContainer->setScrollRect(m_scrollRect);
    for (int i = 0; i < 20; ++i)
    {
        char name[64];
        std::sprintf(name, "FJ_HtmlLabel_%d", i);
        FJ_Label* label = getChildByName(name)->asFJ_Label();
        m_pContentContainer->addChild(label);
        if (label != NULL)
        {
            m_pLabels[i] = label;
            m_pLabels[i]->ResetLabel();
            m_pLabels[i]->setWidth(m_fRealWidth - m_iTopMargin * 2);
            m_pLabels[i]->setX(0.0f);
            m_pLabels[i]->setY(0.0f);
            m_pLabels[i]->setVisible(false);
            m_pLabels[i]->SetLabel(L"");
        }
    }
}

void FJ_DynamicHtmlText::ASConstructor()
{
    m_bTouchScrolling = false;
    m_iCurrentTouchPosition = 0;
    m_iLabelSpacing = 10;
    m_iTopMargin = 20;
    m_iTargetDirection = 0;
    m_iTouchScroll = 0;
    m_bKeyboardScroll = false;
    m_iScrollDirection = 0;
    m_iTouchStart = 0;
    m_iCharacterHeight = 0;
    m_bTouchFlick = false;
    m_iFlickOffset = 0;
    m_pUpArrow = static_cast<FJ_ScrollArrow*>(
        FJ_Factory::createObject("fourj.FJ_ScrollArrow", m_pRenderNode, true, NULL));
    m_pDownArrow = static_cast<FJ_ScrollArrow*>(
        FJ_Factory::createObject("fourj.FJ_ScrollArrow", m_pRenderNode, true, NULL));
    Init();
    getStage()->addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    getStage()->addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
    m_fScrollSpeed = 1.0f;
    if (getStage()->getStageHeight() == 1080.0f || getStage()->getStageHeight() == 540.0f)
        m_fScrollSpeed = 1.5f;
}

void FJ_DynamicHtmlText::keyDownHandler(FJ_Event* event)
{
    FJ_KeyboardEvent* keyEvent = static_cast<FJ_KeyboardEvent*>(event);
    if (m_pUpArrow->isVisible() || m_pDownArrow->isVisible())
    {
        if (keyEvent->m_iKeyCode == 3 || keyEvent->m_iKeyCode == 17 || keyEvent->m_iKeyCode == 1)
        {
            if (m_iScrollDirection != -1)
            {
                m_pUpArrow->ChangeState(2);
                m_pDownArrow->ChangeState(3);
            }
            if (keyEvent->m_iKeyCode == 1)
            {
                m_iFlickDelay = 0;
                m_iScrollDelay = 0;
                m_bKeyboardScroll = true;
            }
            if (m_pUpArrow->isVisible())
            {
                m_iScrollDirection = -1;
                m_iTargetDirection = -1;
            }
        }
        if (keyEvent->m_iKeyCode == 4 || keyEvent->m_iKeyCode == 18 || keyEvent->m_iKeyCode == 2)
        {
            if (m_iScrollDirection != 1)
            {
                m_pDownArrow->ChangeState(5);
                m_pUpArrow->ChangeState(0);
            }
            if (keyEvent->m_iKeyCode == 2)
            {
                m_iFlickDelay = 0;
                m_iScrollDelay = 0;
                m_bKeyboardScroll = true;
            }
            if (m_pDownArrow->isVisible())
            {
                m_iScrollDirection = 1;
                m_iTargetDirection = 1;
            }
        }
    }
}

void FJ_DynamicHtmlText::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_DynamicHtmlText*>(node)->keyDownHandler(event);
}

void FJ_DynamicHtmlText::keyUpHandler(FJ_Event* event)
{
    m_bKeyboardScroll = false;
    m_iScrollDirection = 0;
    m_pUpArrow->ChangeState(0);
    m_pDownArrow->ChangeState(3);
}

void FJ_DynamicHtmlText::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_DynamicHtmlText*>(node)->keyUpHandler(event);
}

unsigned int FJ_DynamicHtmlText::getType()
{
    return 0x80aa;
}

void FJ_DynamicHtmlText::setVisible(bool visible)
{
    FJ_FuiNode::setVisible(visible);
    if (!visible)
    {
        removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
        for (int i = 0; i < 20; ++i)
            m_pLabels[i]->setVisible(false);
    }
    if (m_pHtmlTextPanel != NULL)
        m_pHtmlTextPanel->setVisible(visible);
    if (m_pUpArrow != NULL)
        m_pUpArrow->setVisible(visible);
    if (m_pDownArrow != NULL)
        m_pDownArrow->setVisible(visible);
    if (visible)
        HandleScrollListArrows(false);
}

void FJ_DynamicHtmlText::Reload(float width, float height)
{
    m_fRealWidth = width;
    m_fRealHeight = height - m_pUpArrow->getHeight();
    if (m_pHtmlTextPanel != NULL)
    {
        m_pHtmlTextPanel->setX(getX());
        m_pHtmlTextPanel->setY(getY());
        m_pHtmlTextPanel->setWidth(width);
        m_pHtmlTextPanel->setHeight(height);
    }
    m_pContentContainer->setX(getX() + (float)m_iTopMargin);
    m_pContentContainer->setY(getY() + (float)m_iTopMargin);
    m_scrollRect.xMax = m_scrollRect.xMin + m_fRealWidth;
    m_scrollRect.yMax = m_scrollRect.yMin + (m_fRealHeight - Math::round(m_iTopMargin * 1.4));
    m_pContentContainer->setScrollRect(m_scrollRect);
    for (int i = 0; i < 20; ++i)
    {
        m_pLabels[i]->ResetLabel();
        m_pLabels[i]->setWidth(m_fRealWidth - m_iTopMargin * 2);
        m_pLabels[i]->setX(0.0f);
        m_pLabels[i]->setY(0.0f);
        m_pLabels[i]->setVisible(false);
        m_pLabels[i]->SetLabel(L"");
    }
    m_iCharacterHeight = 0;
    m_iScrollDirection = 0;
    m_iTargetDirection = 0;
    m_iCurrentTouchPosition = 0;
    m_iTouchStart = 0;
    m_iFlickOffset = 0;
    m_bKeyboardScroll = false;
    m_bTouchScrolling = false;
    m_bTouchFlick = false;
    m_iContentHeight = 0;
    m_iTargetScrollPosition = 0;
    m_iTouchScroll = 0;
    m_text.clear();
    m_labels.clear();
    m_labelTextIndices.clear();
    removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
}

void FJ_DynamicHtmlText::SetupList()
{
    unsigned int added = 0;
    bool exhausted = false;
    do
    {
        if (added >= m_text.size())
        {
            exhausted = true;
        }
        else
        {
            SetNextLabel(m_labels.size(), true);
            ++added;
        }
    } while (m_labels[m_labels.size() - 1]->getY() + m_labels[m_labels.size() - 1]->getHeight() + m_iTopMargin * 2
                 < m_fRealHeight
             && !exhausted);
    m_bListReady = true;
    m_bAtEnd = exhausted;
}

void FJ_DynamicHtmlText::AddText(const std::wstring& text, bool setup)
{
    m_text.push_back(text);
    if (setup)
    {
        addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
        SetupList();
        HandleScrollListArrows(true);
        if (m_labels.size() != 0)
            m_iCharacterHeight = m_labels[0]->GetAverageCharHeight();
    }
}

float FJ_DynamicHtmlText::GetRealWidth()
{
    return m_fRealWidth;
}

float FJ_DynamicHtmlText::GetRealHeight()
{
    return m_fRealHeight;
}

void FJ_DynamicHtmlText::TouchScroll(int offset, bool active)
{
    if (active)
    {
        m_iTouchStart = offset;
        if (!m_bTouchScrolling)
        {
            m_iCurrentTouchPosition = offset;
            m_bTouchFlick = false;
            m_bTouchScrolling = true;
            return;
        }
        offset -= m_iCurrentTouchPosition;
        if (offset > 0)
        {
            if (m_iScrollDirection != -1)
            {
                m_pUpArrow->ChangeState(2);
                m_pDownArrow->ChangeState(3);
            }
            if (m_pUpArrow->isVisible())
            {
                m_iScrollDirection = -1;
                m_iTargetDirection = -1;
                m_iTouchScroll = -1;
                return;
            }
        }
        else if (offset < 0)
        {
            if (m_iScrollDirection != 1)
            {
                m_pDownArrow->ChangeState(5);
                m_pUpArrow->ChangeState(0);
            }
            if (m_pDownArrow->isVisible())
            {
                m_iScrollDirection = 1;
                m_iTargetDirection = 1;
                m_iTouchScroll = 1;
                return;
            }
        }
        else
        {
            m_iScrollDirection = 0;
            m_pUpArrow->ChangeState(0);
            m_pDownArrow->ChangeState(3);
            return;
        }
        m_iScrollDirection = 0;
        m_iCurrentTouchPosition = m_iTouchStart;
    }
    else
    {
        if (m_iFlickOffset != 0)
            m_bTouchFlick = true;
        m_bTouchScrolling = false;
        m_iCurrentTouchPosition = 0;
        m_iTouchStart = 0;
        m_iScrollDirection = 0;
        m_pUpArrow->ChangeState(0);
        m_pDownArrow->ChangeState(3);
    }
}
