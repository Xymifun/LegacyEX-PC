#include "game/ui/FJ_Slider.hpp"

#include <algorithm>
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

static std::string s_sliderStates[2][2] = {{std::string("Active_Selected"), std::string("Inactive_Selected")},
                                           {std::string("Active_Unselected"), std::string("Inactive_Unselected")}};

FJ_Slider::FJ_Slider(fuiRenderNode* renderNode) : FJ_Base(renderNode)
{
}

uint8_t FJ_Slider::getEnabled()
{
    return m_enabled;
}

uint32_t FJ_Slider::getSliderCurrent()
{
    return m_sliderCurrent;
}

uint32_t FJ_Slider::getCurrentState()
{
    return static_cast<uint32_t>(m_currentState);
}

int FJ_Slider::getOldWidth()
{
    return m_wrapWidth;
}

void FJ_Slider::UpdateSliderScaleMask()
{
    FJ_FuiNode* slider = getChildByName(std::string("Slider"))->asMovieClip();
    FJ_FuiNode* background = getChildByName(std::string("SliderBackground"))->asMovieClip();
    FJ_FuiNode* outline = getChildByName(std::string("Outline"))->asMovieClip();
    FJ_FuiNode* terminator = getChildByName(std::string("SliderTrack_TerminatorContainer"))->asMovieClip();
    m_sliderMask = getChildByName(std::string("SliderBackgroundHighlight"))->asMovieClip();
    m_tintActiveMask = getChildByName(std::string("TintActive"))->asMovieClip();
    m_tintInactiveMask = getChildByName(std::string("TintInactive"))->asMovieClip();

    slider->setScaleX(1.0f / getScaleX());
    m_leftPosition = background->getX();
    m_rightPosition = background->getX() + background->getWidth() - slider->getWidth();

    if (getScaleX() != 1.0f)
    {
        const float backgroundWidth = background->getWidth();
        const float outlineExtra = outline->getWidth() - background->getWidth();
        const float sliderMaskExtra = m_sliderMask->getWidth() - background->getWidth();
        background->setScaleX(1.0f / getScaleX());
        outline->setScaleX(1.0f / getScaleX());
        m_sliderMask->setScaleX(1.0f / getScaleX());
        m_tintActiveMask->setScaleX(1.0f / getScaleX());
        m_tintInactiveMask->setScaleX(1.0f / getScaleX());
        outline->setX(-(outline->getWidth() - background->getWidth()) * 0.5f);
        m_sliderMask->setX(-(m_sliderMask->getWidth() - background->getWidth()) * 0.5f);
        m_tintActiveMask->setX(background->getX());
        m_tintInactiveMask->setX(background->getX());

        fuiRect rect;
        rect.xMin = 0.0f;
        rect.yMin = 0.0f;
        rect.xMax = backgroundWidth * getScaleX();
        rect.yMax = background->getHeight();
        fuiRenderNodeCodeGenRect* maskRender = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "bgMask");
        FJ_FuiNode* mask = new FJ_FuiNode(maskRender);
        maskRender->setFuiNode(mask);
        background->addMask(mask);
        maskRender = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "bgTintActiveMask");
        mask = new FJ_FuiNode(maskRender);
        maskRender->setFuiNode(mask);
        m_tintActiveMask->addMask(mask);
        maskRender = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "bgTintInactiveMask");
        mask = new FJ_FuiNode(maskRender);
        maskRender->setFuiNode(mask);
        m_tintInactiveMask->addMask(mask);
        rect.xMax = backgroundWidth * getScaleX() + sliderMaskExtra;
        rect.yMax = m_sliderMask->getHeight();
        maskRender = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "bghMask");
        mask = new FJ_FuiNode(maskRender);
        maskRender->setFuiNode(mask);
        m_sliderMask->addMask(mask);
        rect.xMax = backgroundWidth * getScaleX() + outlineExtra;
        rect.yMax = outline->getHeight();
        maskRender = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "outlineMask");
        mask = new FJ_FuiNode(maskRender);
        maskRender->setFuiNode(mask);
        outline->addMask(mask);
        m_centerBiasX = -((background->getWidth() - backgroundWidth) * 0.5f);
        terminator->setScaleX(1.0f / getScaleX());
        terminator->setX(backgroundWidth - terminator->getWidth());
        terminator->setY(background->getY());
    }
}

void FJ_Slider::SetSlider(int value)
{
    const int offset = value - m_minValue;
    const int range = m_maxValue - m_minValue;
    const float ratio = (float)offset / (float)range;
    const float width = m_rightPosition - m_leftPosition;
    const float position = width * ratio + m_leftPosition;
    FJ_FuiNode* slider = getChildByName(std::string("Slider"))->asMovieClip();
    slider->setX(position);
}

void FJ_Slider::ChangeState(int state)
{
    const std::string* row = s_sliderStates[state];
    gotoAndStop(row[m_enabled ^ 1]);
    m_currentState = state;
    FJ_FuiNode* outline = getChildByName(std::string("Outline"))->asMovieClip();
    if (outline != NULL)
        outline->setVisible(state == 0);
    if (m_sliderMask != NULL)
        m_sliderMask->setVisible(state == 0);
    if (m_tintActiveMask != NULL)
        m_tintActiveMask->setVisible(state == 0);
    if (m_tintInactiveMask != NULL)
        m_tintInactiveMask->setVisible(state != 0);
}

void FJ_Slider::InitSliderMask()
{
    UpdateSliderScaleMask();
    SetSlider(m_sliderCurrent);
    ChangeState(1);
    m_maskInitialized = true;
}

void FJ_Slider::focusInHandler(FJ_Event*)
{
    ChangeState(0);
}

void FJ_Slider::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Slider*>(node)->focusInHandler(event);
}

void FJ_Slider::focusOutHandler(FJ_Event*)
{
    ChangeState(1);
    m_keyPressTicks = 0;
    m_keyPressDelta = 0;
}

void FJ_Slider::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Slider*>(node)->focusOutHandler(event);
}

void FJ_Slider::keyUpHandler(FJ_Event* event)
{
    if (!m_enabled)
        return;
    const int keyCode = static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode;
    if (keyCode == 5 || keyCode == 6)
    {
        m_keyPressTicks = 0;
        m_keyPressDelta = 0;
    }
}

void FJ_Slider::_keyUpHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Slider*>(node)->keyUpHandler(event);
}

void FJ_Slider::MoveSlider(int amount)
{
    const int oldValue = m_sliderCurrent;
    m_sliderCurrent += amount;
    m_sliderCurrent = std::max(m_minValue, std::min(m_maxValue, m_sliderCurrent));
    if (oldValue != m_sliderCurrent)
    {
        SetSlider(m_sliderCurrent);
        if (getObjListParent() != NULL)
        {
            getRoot()->m_pCallbackScene->handleSliderElementMove(getObjListParent()->getId(), getId(), m_sliderCurrent);
            return;
        }
        getRoot()->m_pCallbackScene->handleSliderMove(getId(), m_sliderCurrent);
    }
}

void FJ_Slider::keyDownHandler(FJ_Event* event)
{
    if (!m_enabled)
        return;
    if (!m_repeat || m_keyPressTicks + 1 < m_repeatDelay)
    {
        m_keyPressTicks++;
    }
    else
    {
        m_keyPressTicks = 0;
        m_keyPressDelta += m_repeatStep;
    }
    const int keyCode = static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode;
    if (keyCode == 5)
        MoveSlider(~m_keyPressDelta);
    else if (keyCode == 6)
        MoveSlider(m_keyPressDelta + 1);
}

void FJ_Slider::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Slider*>(node)->keyDownHandler(event);
}

void FJ_Slider::UpdateLabel()
{
    SetLabel(m_label);
}

void FJ_Slider::HandleUpdateLabel(FJ_Event*)
{
    UpdateLabel();
}

void FJ_Slider::_HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Slider*>(node)->HandleUpdateLabel(event);
}

void FJ_Slider::ASConstructor()
{
    m_enabled = true;
    m_minValue = 0;
    m_maxValue = 100;
    m_sliderCurrent = 50;
    m_currentState = 0;
    m_leftPosition = 0.0f;
    m_rightPosition = 0.0f;
    m_wrapWidth = (int)getWidth();
    m_maskInitialized = false;
    m_sliderMask = NULL;
    m_tintActiveMask = NULL;
    m_tintInactiveMask = NULL;
    m_keyPressDelta = 0;
    m_keyPressTicks = 0;
    m_repeat = false;
    m_repeatDelay = 10;
    m_repeatStep = 1;
    if (getParent() != NULL)
        InitSliderMask();
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    addEventListener(eFJEventType_KeyUp, this, _keyUpHandler, false);
    addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
}

void FJ_Slider::Init(const std::wstring& name, int id, int minValue, int maxValue, int value, bool repeat,
                     int repeatDelay, int repeatStep)
{
    if (!m_maskInitialized)
        InitSliderMask();
    SetLabelAlignment(2);
    InitBase(name, id);
    m_minValue = minValue;
    m_maxValue = maxValue;
    m_sliderCurrent = value;
    SetSlider(value);
    m_repeat = repeat;
    m_repeatDelay = repeatDelay;
    m_repeatStep = repeatStep;
}

void FJ_Slider::SetRelativeSliderPos(float position)
{
    const int oldValue = m_sliderCurrent;
    position = std::max(0.0f, std::min(1.0f, position));
    const int range = m_maxValue - m_minValue;
    const float bias = m_minValue > 0 && position > 0.0f ? (float)m_minValue : 0.0f;
    m_sliderCurrent = Math::round((float)range * position + bias);
    m_sliderCurrent = std::max(m_minValue, std::min(m_maxValue, m_sliderCurrent));
    if (oldValue != m_sliderCurrent)
    {
        SetSlider(m_sliderCurrent);
        if (getObjListParent() != NULL)
        {
            getRoot()->m_pCallbackScene->handleSliderElementMove(getObjListParent()->getId(), getId(), m_sliderCurrent);
            return;
        }
        getRoot()->m_pCallbackScene->handleSliderMove(getId(), m_sliderCurrent);
    }
}

float FJ_Slider::GetRealWidth()
{
    return m_wrapWidth;
}

void FJ_Slider::SetSliderValue(int value)
{
    m_sliderCurrent = std::max(m_minValue, std::min(m_maxValue, value));
    SetSlider(m_sliderCurrent);
}

void FJ_Slider::EnableSlider(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    ChangeState(m_currentState);
}

void FJ_Slider::GetLabelTooWide(fuiRenderNodeEditText* textField)
{
    const float labelWidth = static_cast<float>(m_maxLabelWidth);
    const int widest = static_cast<int>(labelWidth > textField->m_textWidth ? labelWidth : textField->m_textWidth);
    const float scaledWidth = static_cast<float>(widest) * 1.1f;
    const float currentWidth = static_cast<float>(m_newWidth);
    const int newWidth = static_cast<int>(scaledWidth > currentWidth ? scaledWidth : currentWidth);
    m_newWidth = newWidth > m_newWidth ? newWidth : m_newWidth;
    if (!m_labelOverflowResolved && widest > m_wrapWidth)
    {
        m_labelTooWide = true;
        m_labelOverflowResolved = true;
        m_ellipsisWidth = static_cast<int>(static_cast<float>(m_wrapWidth) * 0.8f);
    }
}

void FJ_Slider::SetNewObjectWidth(int width)
{
    if (width > m_wrapWidth)
    {
        SetTextOnly(std::wstring(L" "));
        setWidth(getWidth() + (float)(width - m_wrapWidth) / getScaleX());
        m_wrapWidth = width;
        fuiRect rect = getRect(this);
        m_objectWidth = getWidth() + (rect.xMin + rect.xMin);
        m_labelTooWide = false;
        m_labelOverflowResolved = false;
        FJ_FuiNode* slider = getChildByName(std::string("Slider"))->asMovieClip();
        FJ_FuiNode* background = getChildByName(std::string("SliderBackground"))->asMovieClip();
        if (slider != NULL && background != NULL)
        {
            const float oldSliderWidth = slider->getWidth();
            slider->setScaleX(1.0f / getScaleX());
            m_rightPosition += oldSliderWidth - slider->getWidth();
            SetSlider(m_sliderCurrent);
        }
        SetLabel(m_label);
    }
}
