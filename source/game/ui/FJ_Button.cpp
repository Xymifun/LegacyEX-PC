#include "game/ui/FJ_Button.hpp"
#include <cmath>
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/util/Math.hpp"

static std::string s_buttonStates[4][2] = {
    {std::string("Active_First_Selected", 21), std::string("Inactive_Selected", 17)},
    {std::string("Active_Selected", 15), std::string("Inactive_Selected", 17)},
    {std::string("Active_Unselected", 17), std::string("Inactive_Unselected", 19)},
    {std::string("Active_Pressed", 14), std::string("Inactive_Selected", 17)}};

class FJ_ButtonRenderNodeAccess : public fuiRenderNode
{
public:
    static void setTint(fuiRenderNode* node, float value)
    {
        FJ_ButtonRenderNodeAccess* access = static_cast<FJ_ButtonRenderNodeAccess*>(node);
        access->m_redAdd = value;
        access->m_greenAdd = value;
        access->m_blueAdd = value;
    }
};

FJ_Button::FJ_Button(fuiRenderNode* renderNode)
    : FJ_Base(renderNode),
      m_buttonPressed(false),
      m_showOutline(false),
      m_enabled(true),
      m_keyDownDelegate(0),
      m_focused(false),
      m_hiddenUntilInit(false),
      m_hasSecondaryText(false),
      m_secondaryText(),
      m_secondaryWidth(-1.0f),
      m_secondaryX(-1.0f),
      m_updateLabelDelegate(0),
      m_secondaryHtml(false),
      m_secondaryAutoSize(false)
{
    ChangeState(2);
    ToggleOutline(false);
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
}

void FJ_Button::setButtonPressed(bool pressed)
{
    m_buttonPressed = pressed;
}

void FJ_Button::ChangeState(int state)
{
    int variant = m_enabled ? 0 : 1;
    std::string* stateName = s_buttonStates[state];
    if ((state == 0 || state == 3) && m_enabled)
        gotoAndPlay(stateName[variant]);
    else
        gotoAndStop(stateName[variant]);
    m_currentState = state;
}

void FJ_Button::ToggleOutline(bool show)
{
    FJ_FuiNode* outline = getChildByName(std::string("Outline"));
    if (outline != 0)
    {
        FJ_FuiNode* movieClip = outline->asMovieClip();
        if (movieClip != 0)
        {
            if (show)
                movieClip->gotoAndPlay(std::string("Outline_Enabled"));
            else
                movieClip->gotoAndPlay(std::string("Outline_Disabled"));
        }
    }
    m_showOutline = show;
}

void FJ_Button::focusInHandler(FJ_Event*)
{
    ChangeState(1);
    m_focused = true;
}

void FJ_Button::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Button*>(node)->focusInHandler(event);
}

void FJ_Button::focusOutHandler(FJ_Event*)
{
    ChangeState(2);
    m_focused = false;
}

void FJ_Button::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Button*>(node)->focusOutHandler(event);
}

void FJ_Button::keyDownHandler(FJ_Event* event)
{
    if (m_enabled && !m_buttonPressed && static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 0)
    {
        ChangeState(3);
        m_buttonPressed = true;
        dispatchEvent(new FJ_Event(eFJEventType_Press, true, false));
        if (m_keyDownDelegate != 0)
            m_keyDownDelegate(m_keyDownDelegateNode, m_id);
    }
}

void FJ_Button::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Button*>(node)->keyDownHandler(event);
}

void FJ_Button::keyUpHandler(FJ_Event* event)
{
    if (!m_buttonPressed || static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode != 0)
        return;
    m_buttonPressed = false;
}

void FJ_Button::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Button*>(node)->keyUpHandler(event);
}

void FJ_Button::HideUntilInit()
{
    setVisible(false);
    m_hiddenUntilInit = true;
}

void FJ_Button::HandleUpdateLabel(FJ_Event*)
{
    UpdateLabel();
}

void FJ_Button::_HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Button*>(node)->HandleUpdateLabel(event);
}

void FJ_Button::Init(const std::wstring& name, int id)
{
    if (m_hiddenUntilInit)
    {
        setVisible(true);
        m_hiddenUntilInit = false;
    }
    InitBase(name, id);
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
}

void FJ_Button::SetKeyDownDelegate(void (*delegate)(FJ_FuiNode*, int), FJ_FuiNode* node)
{
    m_keyDownDelegate = delegate;
    m_keyDownDelegateNode = node;
}

fuiRenderNodeEditText* FJ_Button::GetSecondaryTextField()
{
    fuiRenderNodeEditText* field = 0;
    FJ_FuiNode* movieClip = getChildByName(std::string("FJ_TextContainer_2"))->asMovieClip();
    if (movieClip != 0 && movieClip->getNumChildren() != 0)
    {
        FJ_FuiNode* child = movieClip->getChildAt(0);
        if (child != 0)
        {
            fuiRenderNode* timelineNode = child->m_pRenderNode->asTimeline();
            if (timelineNode != 0)
                field = static_cast<fuiRenderNodeTimeline*>(timelineNode)->getChildAt(0)->asEditText();
            m_hasSecondaryText = true;
        }
    }
    return field;
}

void FJ_Button::SetSecondaryText(const std::wstring& text, bool html, bool autoSize)
{
    m_secondaryHtml = html;
    m_secondaryAutoSize = autoSize;
    FJ_FuiNode* movieClip = getChildByName(std::string("FJ_TextContainer_2"))->asMovieClip();
    fuiRenderNodeEditText* field = GetSecondaryTextField();
    if (m_secondaryWidth == -1.0f)
        m_secondaryWidth = movieClip->getWidth();
    if (m_secondaryX == -1.0f)
        m_secondaryX = -((movieClip->getX() + movieClip->getWidth()) * getScaleX() - m_objectWidth);
    if (text.compare(L"") == 0)
        m_secondaryText = L" ";
    else
        m_secondaryText = text;
    if (autoSize)
        field->setHTMLText(m_secondaryText);
    else
        field->setText(m_secondaryText);
    if (movieClip != 0)
    {
        movieClip->setScaleX(1.0f);
        movieClip->setScaleY(1.0f);
    }
    field->setScaleX(1.0f / getScaleX());
    field->setScaleY(1.0f / getScaleY());
    field->setAutoSize(html ? TextFieldAutoSizeRight : TextFieldAutoSizeLeft);
    if (movieClip != 0)
        movieClip->setY(0.0f);
    if (!html)
    {
        field->setX(static_cast<float>(
            Math::round((m_secondaryWidth / getScaleX() - field->getWidth()) * 0.5f + m_centerBiasX)));
        field->setY(static_cast<float>(Math::round((m_objectHeight / getScaleY() - field->getHeight()) * 0.5f)));
    }
    else
    {
        field->setX(static_cast<float>(Math::round(m_secondaryWidth / getScaleX() - field->getWidth())));
        if (!autoSize)
        {
            field->setY(static_cast<float>(Math::round((m_objectHeight / getScaleY() - field->getHeight()) * 0.5f)));
        }
        else if (getStage()->getStageHeight() == 720.0f || getStage()->getStageHeight() == 360.0f
                 || getStage()->getStageHeight() == 408.0f || getStage()->getStageHeight() == 480.0f)
        {
            field->setY(-1.0f);
        }
        else
        {
            field->setY(2.0f);
        }
        movieClip->setX((m_objectWidth - m_secondaryWidth) / getScaleX() - m_secondaryX);
    }
}

void FJ_Button::UpdateLabel()
{
    SetLabel(m_label);
    if (m_hasSecondaryText)
        SetSecondaryText(m_secondaryText, m_secondaryHtml, m_secondaryAutoSize);
    if (m_updateLabelDelegate != 0)
        m_updateLabelDelegate(this);
}

void FJ_Button::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabel(label);
}

void FJ_Button::AdjustDisabledTint()
{
    FJ_ButtonRenderNodeAccess::setTint(m_pRenderNode, m_enabled ? 0.0f : 0.09803922f);
}

void FJ_Button::EnableButton(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    ChangeState(m_currentState);
    AdjustDisabledTint();
}

void FJ_Button::GetLabelTooWide(fuiRenderNodeEditText* textField)
{
    int widest = m_maxLabelWidth;
    if (static_cast<float>(widest) <= textField->m_textWidth)
        widest = static_cast<int>(textField->m_textWidth);
    int scaledWidth = static_cast<int>(widest * 1.26);
    if (m_newWidth < scaledWidth)
        m_newWidth = scaledWidth;
    if (!m_labelTooWide && m_preScaleWidth < scaledWidth)
    {
        m_labelTooWide = true;
        if (!m_labelOverflowResolved && m_preScaleWidth < static_cast<int>(widest * 1.05))
        {
            m_ellipsisWidth = static_cast<int>(m_objectWidth * 0.8);
            m_labelOverflowResolved = true;
        }
    }
}
