#include "game/ui/FJ_HtmlText.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

static const int s_scrollUpKeys[2] = {3, 17};
static const int s_scrollDownKeys[2] = {4, 18};

FJ_HtmlText::FJ_HtmlText(fuiRenderNode* renderNode) : FJ_Base(renderNode)
{
    m_pText = NULL;
    m_bScrollingEnabled = 1;
    m_bStopAtTarget = false;
    m_bTouchScrolling = false;
    m_bScrollingDown = false;
    m_iFocusDirection = 0;
    m_bNoBackground = false;
    m_bScrollingUp = false;
    m_iTargetScroll = 0;
    m_bInitialized = 1;
    m_pScrollingCompleteDelegate = NULL;
    m_iTouchStart = 0;
    m_bUseScrollRect = true;
    m_iTouchPosition = 0;
    m_iTouchScroll = 0;
    m_iTouchTarget = 0;
    m_pUpArrow = FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, NULL);
    m_pDownArrow = static_cast<FJ_ScrollArrow*>(
        FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, NULL));
    getStage()->addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    getStage()->addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
    m_fScrollSpeed = 1.0f;
    if (getStage()->getStageHeight() == 1080.0f || getStage()->getStageHeight() == 540.0f)
        m_fScrollSpeed = 1.5f;
}

FJ_HtmlText::~FJ_HtmlText()
{
    if (m_pUpArrow != NULL && m_pUpArrow->getParent() != NULL)
        m_pUpArrow->getParent()->removeChild(m_pUpArrow, true);
    if (m_pDownArrow != NULL && m_pDownArrow->getParent() != NULL)
        m_pDownArrow->getParent()->removeChild(m_pDownArrow, true);
}

bool FJ_HtmlText::CanScroll(bool up)
{
    if (m_pText != NULL && m_bScrollingEnabled)
    {
        float top = m_fScrollTop;
        float visibleHeight = m_fScrollBottom - top;
        float textHeight = m_pText->getTextHeight();
        if (textHeight >= visibleHeight)
        {
            if (up)
            {
                if (top > 0.0f)
                    return true;
            }
            else if (top < textHeight - visibleHeight)
            {
                return true;
            }
        }
    }
    return false;
}

void FJ_HtmlText::HandleScrollListArrows(bool positionArrows)
{
    if (positionArrows)
    {
        getParent()->addChild(m_pUpArrow);
        getParent()->addChild(m_pDownArrow);
        m_pUpArrow->setVisible(true);
        m_pDownArrow->setVisible(true);
        if (isVisible())
        {
            static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(0);
            m_pDownArrow->ChangeState(3);
        }
    }

    if (isVisible() && !m_bNoBackground)
    {
        m_pUpArrow->setX(m_pText->getX() + m_iRealWidth - m_pUpArrow->getWidth() * 1.5f);
        m_pUpArrow->setY(m_pText->getY() + (m_fScrollBottom - m_fScrollTop) + m_pUpArrow->getHeight() * 0.5f);
        m_pDownArrow->setX(m_pText->getX() + m_iRealWidth - m_pUpArrow->getWidth() * 0.5f);
        m_pDownArrow->setY(m_pUpArrow->getY());
        m_pUpArrow->setVisible(CanScroll(true));
        m_pDownArrow->setVisible(CanScroll(false));
    }
    else
    {
        m_pUpArrow->setVisible(false);
        m_pDownArrow->setVisible(false);
    }
}

void FJ_HtmlText::UpdateScrolling(FJ_Event* event)
{
    float speedFactor = 1.0f;
    if (m_iScrollDelay < 1)
    {
        if (m_iFlickDelay >= 0)
        {
            if (m_iFlickDelay > 0)
                --m_iFlickDelay;
            speedFactor += (30 - m_iFlickDelay) / 30.0f;
        }
    }
    else
    {
        --m_iScrollDelay;
    }
    int step = Math::round(((120.0f * m_fScrollSpeed) / 30.0f) * speedFactor);

    if (m_bScrollingUp && CanScroll(true))
    {
        float top = m_fScrollTop;
        if (!m_bTouchScrolling)
        {
            m_fScrollTop = top - step;
            m_fScrollBottom = m_fScrollTop + (m_fScrollBottom - top);
        }
        else
        {
            float target = m_iTouchTarget;
            if (top <= target)
            {
                m_bTouchScrolling = false;
                m_bScrollingUp = false;
            }
            else
            {
                float bottom = target + (m_fScrollBottom - top);
                if (target < 0.0f)
                {
                    m_fScrollTop = 0.0f;
                    m_fScrollBottom = bottom - target;
                }
                else
                {
                    m_fScrollTop = target;
                    m_fScrollBottom = bottom;
                }
            }
        }

        if (m_bStopAtTarget && m_fScrollTop <= m_iTargetScroll)
        {
            float target = m_iTargetScroll;
            m_fScrollBottom = target + (m_fScrollBottom - m_fScrollTop);
            m_fScrollTop = target;
            m_bScrollingUp = false;
        }
    }
    else if (m_bScrollingDown && CanScroll(false))
    {
        float top = m_fScrollTop;
        if (!m_bTouchScrolling)
        {
            m_fScrollTop = top + step;
            m_fScrollBottom = m_fScrollTop + (m_fScrollBottom - top);
        }
        else
        {
            float target = m_iTouchTarget;
            if (target <= top)
            {
                m_bTouchScrolling = false;
                m_bScrollingDown = false;
            }
            else
            {
                m_fScrollTop = target;
                m_fScrollBottom = target + (m_fScrollBottom - top);
                if (m_fScrollTop > m_pText->getHeight() - (m_fScrollBottom - m_fScrollTop))
                {
                    float visibleHeight = m_fScrollBottom - m_fScrollTop;
                    m_fScrollTop = m_pText->getHeight() - visibleHeight;
                    m_fScrollBottom = m_fScrollTop + visibleHeight;
                }
            }
        }

        if (m_bStopAtTarget && m_iTargetScroll <= m_fScrollTop)
        {
            float target = m_iTargetScroll;
            m_fScrollBottom = target + (m_fScrollBottom - m_fScrollTop);
            m_fScrollTop = target;
            m_bScrollingDown = false;
        }
    }
    else
    {
        HandleScrollListArrows(false);
        getStage()->removeEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling);
        if (isVisible())
        {
            static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(0);
            m_pDownArrow->ChangeState(3);
        }
    }

    fuiRect scrollRect;
    scrollRect.xMin = m_fScrollLeft;
    scrollRect.xMax = m_fWidth;
    scrollRect.yMin = m_fScrollTop;
    scrollRect.yMax = m_fScrollBottom;
    m_pText->setScrollRect(scrollRect);
}

void FJ_HtmlText::_UpdateScrolling(FJ_FuiNode* node, FJ_Event* event)
{
    node->asFJ_HtmlText()->UpdateScrolling(event);
}

void FJ_HtmlText::keyDownHandler(FJ_Event* event)
{
    if (!m_bScrollingEnabled)
        return;

    bool scrollUp = false;
    bool scrollDown = false;
    bool page = false;
    m_iFocusDirection = getStage()->getFocus() != this;
    int key = static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode;
    if (key == s_scrollUpKeys[m_iFocusDirection])
        scrollUp = true;
    else if (key == s_scrollDownKeys[m_iFocusDirection])
        scrollDown = true;
    if (key == 1)
    {
        page = true;
        scrollUp = true;
    }
    else if (key == 2)
    {
        page = true;
        scrollDown = true;
    }

    if (scrollUp)
    {
        m_bStopAtTarget = false;
        m_bScrollingDown = false;
        if (!m_bScrollingUp && CanScroll(true))
        {
            m_bScrollingUp = true;
            m_iFlickDelay = m_iScrollDelay = page ? 0 : 30;
            getStage()->addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
            if (isVisible())
            {
                static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(2);
                m_pDownArrow->ChangeState(3);
            }
        }
        HandleScrollListArrows(false);
    }
    else if (scrollDown)
    {
        m_bStopAtTarget = false;
        m_bScrollingUp = false;
        if (!m_bScrollingDown && CanScroll(false))
        {
            m_bScrollingDown = true;
            m_iFlickDelay = m_iScrollDelay = page ? 0 : 30;
            getStage()->addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
            if (isVisible())
            {
                m_pDownArrow->ChangeState(5);
                static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(0);
            }
        }
        HandleScrollListArrows(false);
        if (m_pScrollingCompleteDelegate != NULL && !CanScroll(false))
        {
            m_pScrollingCompleteDelegate();
            m_pScrollingCompleteDelegate = NULL;
        }
    }
}

void FJ_HtmlText::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_HtmlText*>(node)->keyDownHandler(event);
}

void FJ_HtmlText::GetScrollTarget(bool page)
{
    int lastLine = Math::round(m_pText->getHeight() - (m_fScrollBottom - m_fScrollTop));
    int position = Math::round(m_fScrollTop);
    int target;
    if (m_bScrollingUp)
    {
        target = lastLine;
        if (page)
            position -= Math::round((m_fScrollBottom - m_fScrollTop) - m_iCharacterHeight * 3);
        do
        {
            target -= m_iCharacterHeight;
        } while (target > position);
        if (target - m_iCharacterHeight < 0)
        {
            m_iTargetScroll = 0;
            return;
        }
    }
    else
    {
        if (!m_bScrollingDown)
            return;
        target = 0;
        if (page)
            position += Math::round((m_fScrollBottom - m_fScrollTop) - m_iCharacterHeight * 3);
        do
        {
            target += m_iCharacterHeight;
        } while (target < position);
        if (target + m_iCharacterHeight > lastLine)
            target = lastLine;
    }
    m_iTargetScroll = target;
}

void FJ_HtmlText::keyUpHandler(FJ_Event* event)
{
    int key = static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode;
    bool scrollUp = false;
    bool scrollDown = false;
    bool page = false;
    if (key == s_scrollUpKeys[m_iFocusDirection])
        scrollUp = true;
    if (key == s_scrollDownKeys[m_iFocusDirection])
        scrollDown = true;
    if (key == 1 || key == 2)
    {
        page = true;
        m_bStopAtTarget = true;
        GetScrollTarget(page);
    }
    else if (scrollUp || scrollDown)
    {
        m_bStopAtTarget = true;
        GetScrollTarget(page);
    }
}

void FJ_HtmlText::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_HtmlText*>(node)->keyUpHandler(event);
}

int FJ_HtmlText::getTextHeight()
{
    return m_pText->getHeight();
}

unsigned int FJ_HtmlText::getType()
{
    return 0x80a1;
}

void FJ_HtmlText::setVisible(bool visible)
{
    FJ_FuiNode::setVisible(visible);
    if (m_pText != NULL)
        m_pText->setVisibility(visible);
    if (m_pUpArrow != NULL)
    {
        m_pUpArrow->m_pRenderNode->setVisibility(visible);
        static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(0);
    }
    if (m_pDownArrow != NULL)
    {
        m_pDownArrow->m_pRenderNode->setVisibility(visible);
        m_pDownArrow->ChangeState(3);
    }
    if (visible)
    {
        HandleScrollListArrows(false);
        getStage()->addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
        getStage()->addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
    }
    else
    {
        getStage()->removeEventListener(eFJEventType_KeyDown, this, _keyDownHandler);
        getStage()->removeEventListener(eFJEventType_KeyUp, this, _keyUpHandler);
    }
}

void FJ_HtmlText::UpdateScrollListArrows()
{
    if (m_pUpArrow != NULL && m_pDownArrow != NULL)
    {
        m_pUpArrow->setX(m_pText->getX() + m_iRealWidth - m_pUpArrow->getWidth() * 1.5f);
        m_pUpArrow->setY(m_pText->getY() + (m_fScrollBottom - m_fScrollTop) + m_pUpArrow->getHeight() * 0.5f);
        m_pDownArrow->setX(m_pText->getX() + m_iRealWidth - m_pUpArrow->getWidth() * 0.5f);
        m_pDownArrow->setY(m_pUpArrow->getY());
    }
}

void FJ_HtmlText::setX(float x)
{
    FJ_FuiNode::setX(x);
    if (m_pText != NULL)
    {
        m_pText->setX(x);
        UpdateScrollListArrows();
    }
}

void FJ_HtmlText::setY(float y)
{
    float delta = y - FJ_FuiNode::getY();
    FJ_FuiNode::setY(y);
    if (m_pText != NULL)
    {
        m_pText->setY(m_pText->getY() + delta);
        UpdateScrollListArrows();
    }
}

void FJ_HtmlText::setHeight(float height)
{
    FJ_FuiNode::setHeight(height);
    if (m_pText != NULL)
    {
        int arrowHeight = 0;
        if (!m_bNoBackground)
            arrowHeight = m_pUpArrow->getHeight();
        m_fScrollBottom = m_fScrollTop + (height - arrowHeight);
        m_pText->setHeight(height);

        fuiRect scrollRect;
        scrollRect.xMin = m_fScrollLeft;
        scrollRect.xMax = m_fWidth;
        scrollRect.yMin = m_fScrollTop;
        scrollRect.yMax = m_fScrollBottom;
        m_pText->setScrollRect(scrollRect);
        UpdateScrollListArrows();
    }
}

void FJ_HtmlText::Init(const std::wstring& label)
{
    float width = getWidth();
    float height = getHeight();
    FJ_FuiNode* background = getChildByName("HtmlBackground");
    if (background != NULL)
    {
        m_iRealWidth = background->getWidth() * getScaleX();
        m_iRealHeight = background->getHeight() * getScaleY();
        removeChild(background, true);
    }

    int arrowHeight = 0;
    if (!m_bNoBackground)
        arrowHeight = m_pUpArrow->getHeight();
    removeChild(m_pText, false);
    getParent()->addChild(m_pText);
    m_pText->setX(getX());
    m_pText->setY(getY());
    m_pText->setWidth(width);
    float textHeight = height - arrowHeight;
    m_pText->setHeight(textHeight);
    m_pText->setAutoSize(TextFieldAutoSizeLeft);
    m_pText->m_wordWrap = true;
    m_pText->m_multiline = true;

    m_bScrollingEnabled = m_bUseScrollRect;
    m_fScrollLeft = 0.0f;
    m_fScrollTop = 0.0f;
    m_fWidth = width;
    m_fScrollBottom = textHeight;
    if (m_bUseScrollRect)
    {
        fuiRect scrollRect;
        scrollRect.xMin = 0.0f;
        scrollRect.xMax = width;
        scrollRect.yMin = 0.0f;
        scrollRect.yMax = textHeight;
        m_pText->setScrollRect(scrollRect);
    }
    SetLabel(label);
    HandleScrollListArrows(true);
}

void FJ_HtmlText::GetScrollDistance()
{
    int samples = 0;
    fuiRenderNodeEditText* text = m_pText;
    int total = 0;
    unsigned int i = 0;
    m_iCharacterHeight = 0;
    while (i < text->getText().length())
    {
        fuiRect boundaries = text->getCharBoundaries(i);
        ++samples;
        total = m_iCharacterHeight + static_cast<int>(boundaries.yMax - boundaries.yMin);
        if (samples >= 5)
            break;
        m_iCharacterHeight = total;
        ++i;
        text = m_pText;
    }
    m_iCharacterHeight = total / samples;
}

void FJ_HtmlText::SetLabel(const std::wstring& label)
{
    if (label == L"")
        m_pText->setHTMLText(L" ");
    else
        m_pText->setHTMLText(label);

    if (m_bScrollingEnabled)
    {
        m_fScrollBottom = m_fScrollBottom - m_fScrollTop;
        m_fScrollTop = 0.0f;
        fuiRect scrollRect;
        scrollRect.xMin = m_fScrollLeft;
        scrollRect.xMax = m_fWidth;
        scrollRect.yMin = m_fScrollTop;
        scrollRect.yMax = m_fScrollBottom;
        m_pText->setScrollRect(scrollRect);
    }
    HandleScrollListArrows(false);
    GetScrollDistance();
    if (m_bScrollingEnabled)
    {
        m_bScrollingUp = false;
        m_bStopAtTarget = false;
        m_bScrollingDown = false;
        m_iTargetScroll = 0;
    }
}

int FJ_HtmlText::GetNumberOfLines()
{
    return Math::round((m_fScrollBottom - m_fScrollTop) / m_iCharacterHeight);
}

int FJ_HtmlText::GetOptimalHeight(int lines)
{
    int arrowHeight = 0;
    if (!m_bNoBackground)
        arrowHeight = m_pUpArrow->getHeight();

    if (lines == -1)
    {
        lines = GetNumberOfLines();
        if (lines < 3)
        {
            int lineHeight = m_iCharacterHeight + 2;
            return lineHeight * 3 + arrowHeight;
        }
        if (lines > 8)
        {
            int lineHeight = m_iCharacterHeight + 2;
            int height = lineHeight * lines + arrowHeight;
            do
            {
                if (height <= m_fScrollBottom - m_fScrollTop)
                    return height;
                --lines;
                height = lineHeight * lines + arrowHeight;
            } while (lines > 8);
            return height;
        }
    }
    int lineHeight = m_iCharacterHeight + 2;
    return lineHeight * lines + arrowHeight;
}

float FJ_HtmlText::GetRealWidth()
{
    return m_iRealWidth;
}

float FJ_HtmlText::GetRealHeight()
{
    return m_iRealHeight;
}

void FJ_HtmlText::TouchScroll(int position, bool active)
{
    if (active)
    {
        m_iTouchPosition = position;
        if (!m_bTouchScrolling)
        {
            m_iTouchStart = position;
            m_bTouchScrolling = true;
            m_iTouchTarget = m_fScrollTop;
            m_iTouchScroll = m_fScrollTop;
            return;
        }
        int movement = position - m_iTouchStart;
        if (movement > 0)
        {
            m_bStopAtTarget = false;
            m_bScrollingDown = false;
            if (m_bScrollingUp || !CanScroll(true))
            {
                m_iTouchTarget = m_iTouchScroll - movement;
            }
            else
            {
                m_iScrollDelay = 30;
                m_iFlickDelay = 30;
                m_bScrollingUp = true;
                getStage()->addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
                if (isVisible())
                {
                    static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(2);
                    m_pDownArrow->ChangeState(3);
                }
                m_iTouchScroll = m_fScrollTop;
                m_iTouchTarget = static_cast<int>(m_fScrollTop) - movement;
            }
            HandleScrollListArrows(false);
        }
        else if (movement < 0)
        {
            m_bStopAtTarget = false;
            m_bScrollingUp = false;
            if (m_bScrollingDown || !CanScroll(false))
            {
                m_iTouchTarget = m_iTouchScroll - movement;
            }
            else
            {
                m_iScrollDelay = 30;
                m_iFlickDelay = 30;
                m_bScrollingDown = true;
                getStage()->addEventListener(eFJEventType_EnterFrame, this, _UpdateScrolling, false);
                if (isVisible())
                {
                    m_pDownArrow->ChangeState(5);
                    static_cast<FJ_ScrollArrow*>(m_pUpArrow)->ChangeState(0);
                }
                m_iTouchScroll = m_fScrollTop;
                m_iTouchTarget = static_cast<int>(m_fScrollTop) - movement;
            }
            HandleScrollListArrows(false);
            if (m_pScrollingCompleteDelegate != NULL && !CanScroll(false))
            {
                m_pScrollingCompleteDelegate();
                m_pScrollingCompleteDelegate = NULL;
            }
        }
    }
    else
    {
        m_bScrollingDown = false;
        m_iTouchStart = 0;
        m_iTouchPosition = 0;
        m_bScrollingUp = false;
        m_bTouchScrolling = false;
        m_bStopAtTarget = true;
        m_iTouchScroll = 0;
        m_iTouchTarget = m_fScrollTop;
        m_iTargetScroll = m_fScrollTop;
    }
}
