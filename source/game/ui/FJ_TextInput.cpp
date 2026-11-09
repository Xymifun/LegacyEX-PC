#include "game/ui/FJ_TextInput.hpp"

#include <algorithm>
#include <string>
#include <utility>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_ScrollArrowH.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/ui/UIScene.hpp"

FJ_TextInput::FJ_TextInput(fuiRenderNode* renderNode)
    : FJ_Base(renderNode),
      m_scrollWidth(100),
      m_firstFocus(true),
      m_caretIndex(0),
      m_scrollOffset(0.0f),
      m_charLimit(-1),
      m_pLeftArrow(0),
      m_pRightArrow(0),
      m_pTextField(0),
      m_bKeepCaret(false),
      m_bUseScale(false)
{
}

FJ_Base* FJ_TextInput::Create(fuiRenderNode* renderNode)
{
    return new FJ_TextInput(renderNode);
}

unsigned int FJ_TextInput::getType()
{
    return 0x803d;
}

void FJ_TextInput::ChangeState(int state)
{
    m_state = state;
    if (m_pOutline != 0 && !m_bKeepCaret)
        m_pOutline->setVisible(state == 0);
    if (m_pCaret != 0)
        m_pCaret->setVisible(state == 0);
}

void FJ_TextInput::PositionScrollArrows()
{
    if (m_pLeftArrow == 0)
        m_pLeftArrow = FJ_Factory::createObject("fourj.FJ_ScrollArrowH", m_pRenderNode, true, 0);
    if (m_pRightArrow == 0)
        m_pRightArrow = FJ_Factory::createObject("fourj.FJ_ScrollArrowH", m_pRenderNode, true, 0);

    const float scale = 0.5f;
    fuiRect rect;
    m_pLeftArrow->setX(getX() - m_pLeftArrow->getWidth() * scale + (rect = getRect(this)).xMin);
    m_pLeftArrow->setY(getY() + getHeight() * scale + (rect = getRect(this)).yMin);
    m_pRightArrow->setX(getX() + getWidth() + m_pRightArrow->getWidth() * scale + (rect = getRect(this)).xMin);
    m_pRightArrow->setY(m_pLeftArrow->getY());
}

void FJ_TextInput::HandleScrollArrows(bool addArrows)
{
    if (addArrows)
    {
        getParent()->addChild(m_pLeftArrow);
        getParent()->addChild(m_pRightArrow);
        PositionScrollArrows();
        static_cast<FJ_ScrollArrowH*>(m_pLeftArrow)->ChangeState(0);
        static_cast<FJ_ScrollArrowH*>(m_pRightArrow)->ChangeState(2);
    }
    if (m_pTextField != 0)
    {
        if (m_pTextField->getScrollH() > 0.0f)
            m_pLeftArrow->setVisible(isVisible());
        else
            m_pLeftArrow->setVisible(false);
        if (m_pTextField->m_textWidth - m_pTextField->getScrollH() > m_pTextField->getWidth())
            m_pRightArrow->setVisible(isVisible());
        else
            m_pRightArrow->setVisible(false);
    }
}

void FJ_TextInput::UpdateScrollPosition(float caretPosition, bool movingRight, bool force)
{
    if (!movingRight)
    {
        if (caretPosition < m_pTextField->getScrollH() || (force && !m_pRightArrow->isVisible()))
        {
            float scroll = caretPosition - static_cast<float>(m_scrollWidth);
            if (scroll < 0.0f)
                scroll = 0.0f;
            m_pTextField->setScrollH(scroll);
            m_scrollOffset = -m_pTextField->getScrollH();
            if (m_pLeftArrow != 0)
                static_cast<FJ_ScrollArrowH*>(m_pLeftArrow)->ChangeState(1);
        }
    }
    else
    {
        float width = static_cast<float>(static_cast<int>(m_pTextField->getWidth()));
        float scrollH = m_pTextField->getScrollH();
        float scroll;
        if (!force)
        {
            if (caretPosition - scrollH <= width)
            {
                HandleScrollArrows(false);
                return;
            }
            scroll = caretPosition - width + 5.0f + static_cast<float>(m_scrollWidth);
        }
        else
        {
            scroll = caretPosition - width + 5.0f;
        }
        if (scroll < 0.0f || caretPosition - m_pTextField->getScrollH() < width)
            scroll = 0.0f;
        m_pTextField->setScrollH(scroll);
        m_scrollOffset = -m_pTextField->getScrollH();
        if (m_pRightArrow != 0)
            static_cast<FJ_ScrollArrowH*>(m_pRightArrow)->ChangeState(3);
    }
    HandleScrollArrows(false);
}

void FJ_TextInput::SetCaretIndex(int index, bool movingRight, bool force)
{
    if (index > 0)
    {
        if (m_label.size() < index)
            index = m_label.size();
        m_caretIndex = index;
        float caretPosition = 0.0f;
        for (int i = 0; i < index; ++i)
        {
            if (i < m_label.size())
            {
                fuiRect bounds = m_pTextField->getCharBoundaries(i);
                caretPosition += bounds.xMax - bounds.xMin;
            }
        }
        UpdateScrollPosition(caretPosition, movingRight, force);
        m_pCaret->setX(m_pTextField->getX() + caretPosition + m_scrollOffset);
        m_pCaret->setY(m_pTextField->getY() + m_pTextField->getHeight() - m_pCaret->getHeight());
        return;
    }
    float caretPosition = 0.0f;
    m_caretIndex = 0;
    UpdateScrollPosition(caretPosition, movingRight, force);
    m_pCaret->setX(m_pTextField->getX() + caretPosition + m_scrollOffset);
    m_pCaret->setY(m_pTextField->getY() + m_pTextField->getHeight() - m_pCaret->getHeight());
}

void FJ_TextInput::focusInHandler(FJ_Event*)
{
    ChangeState(0);
    if (m_firstFocus)
    {
        SetCaretIndex(static_cast<int>(m_label.size()), true, false);
        m_firstFocus = false;
    }
}

void FJ_TextInput::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TextInput*>(node)->focusInHandler(event);
}

void FJ_TextInput::focusOutHandler(FJ_Event*)
{
    ChangeState(1);
}

void FJ_TextInput::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TextInput*>(node)->focusOutHandler(event);
}

void FJ_TextInput::MoveCaretLeft()
{
    m_caretIndex = m_caretIndex - 1;
    SetCaretIndex(m_caretIndex, false, false);
}

void FJ_TextInput::MoveCaretRight()
{
    m_caretIndex = m_caretIndex + 1;
    SetCaretIndex(m_caretIndex, true, false);
}

void FJ_TextInput::keyDownHandler(FJ_Event* event)
{
    FJ_KeyboardEvent* keyEvent = static_cast<FJ_KeyboardEvent*>(event);
    if (keyEvent->m_iKeyCode == 5)
        MoveCaretLeft();
    else if (keyEvent->m_iKeyCode == 6)
        MoveCaretRight();
    if (keyEvent->m_iKeyCode == 0)
        getRoot()->m_pCallbackScene->handlePress(m_id, 0);
}

void FJ_TextInput::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TextInput*>(node)->keyDownHandler(event);
}

void FJ_TextInput::Init(std::wstring name, int id)
{
    m_pCaret = getChildByName(std::string("Caret"))->asMovieClip();
    m_pOutline = getChildByName(std::string("Outline"))->asMovieClip();
    m_pCaret->setScaleX(1.0f / getScaleX());
    m_pCaret->setScaleY(1.0f / getScaleY());
    if (m_pLeftArrow == 0)
        m_pLeftArrow = FJ_Factory::createObject("fourj.FJ_ScrollArrowH", m_pRenderNode, true, 0);
    if (m_pRightArrow == 0)
        m_pRightArrow = FJ_Factory::createObject("fourj.FJ_ScrollArrowH", m_pRenderNode, true, 0);
    ChangeState(1);
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    FJ_FuiNode* movieClip = getChildByName(std::string("FJ_TextContainer"))->getChildAt(0)->asMovieClip();
    fuiRenderNode* timelineNode = movieClip->m_pRenderNode->asTimeline();
    if (timelineNode != 0)
    {
        fuiRenderNodeTimeline* timeline = static_cast<fuiRenderNodeTimeline*>(timelineNode);
        m_pTextField = timeline->getChildAt(0)->asEditText();
    }
    m_pTextField->setScrollRect(m_pTextField->getBounds());
    SetLabelAlignment(4);
    m_isLabel = false;
    if (!m_bUseScale)
    {
        if (getScaleX() != 1.0f)
            setScaleX(1.0f);
        if (getScaleY() != 1.0f)
            setScaleY(1.0f);
    }
    InitBase(name, id);
    HandleScrollArrows(true);
}

void FJ_TextInput::setX(float x)
{
    FJ_FuiNode::setX(x);
    PositionScrollArrows();
}

void FJ_TextInput::setY(float y)
{
    FJ_FuiNode::setY(y);
    PositionScrollArrows();
}

void FJ_TextInput::setVisible(bool visible)
{
    FJ_FuiNode::setVisible(visible);
    HandleScrollArrows(false);
}

void FJ_TextInput::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabel(label);
    m_pTextField->setScrollH(0.0f);
    SetCaretIndex(m_label.size(), true, true);
}

void FJ_TextInput::SetCharLimit(int limit)
{
    m_charLimit = limit;
}

void FJ_TextInput::HandleDirectInput(unsigned int character)
{
    if (character == 8)
    {
        int index = m_caretIndex;
        if (m_label.size() != 0 && index > 0)
        {
            m_label = m_label.substr(0, index - 1) + m_label.substr(m_caretIndex);
            SetLabel(m_label);
            SetCaretIndex(index - 1, false, true);
        }
    }
    else if (m_label.size() < m_charLimit || m_charLimit == -1)
    {
        int index = m_caretIndex;
        m_label = m_label.substr(0, index) + static_cast<wchar_t>(character) + m_label.substr(m_caretIndex);
        SetLabel(m_label);
        SetCaretIndex(index + 1, true, false);
    }
}

void FJ_TextInput::HandleCharInput(unsigned int character)
{
    HandleDirectInput(character);
}

void FJ_TextInput::GetLabelTooWide(fuiRenderNodeEditText*)
{
}

void FJ_TextInput::SetNewObjectWidth(int)
{
}
