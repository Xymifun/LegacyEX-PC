#include "game/ui/UIControl_Slider.hpp"

#include <algorithm>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_Slider.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_Slider::UIControl_Slider() : UIControl_Base()
{
    m_currentValue = 0;
    m_repeatDelay = 10;
    m_maximum = 100;
    m_repeatStep = 1;
    m_focus = false;
    m_repeat = false;
    m_controlId = 0;
    m_minimum = 0;
}

void UIControl_Slider::init(UIString label, int controlId, int minimum, int maximum, int value, bool repeat,
                            int repeatDelay, int repeatStep)
{
    m_label = label;
    m_controlId = controlId;
    m_minimum = minimum;
    m_maximum = maximum;
    m_currentValue = (unsigned int)value;
    m_repeatStep = repeatStep;
    m_repeatDelay = repeatDelay;
    m_repeat = repeat;

    static_cast<FJ_Slider*>(m_fuiRenderNode->getFuiNode())
        ->Init(label.getString(), controlId, minimum, maximum, value, repeat, repeatDelay, repeatStep);

    switch (m_scene->m_layer->m_layerId)
    {
    case 4:
    case 6:
    case 7:
        gConsoleUIController.TouchBoxAdd(this, m_scene);
        break;
    }
}

bool UIControl_Slider::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_Slider;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Slider::setFocus(bool focus)
{
    if (m_focus == focus)
        return;
    m_focus = focus;
    static_cast<FJ_Slider*>(m_fuiRenderNode->getFuiNode())->ChangeState(!focus);
}

void UIControl_Slider::handleSliderMove(int value)
{
    if (m_currentValue != (unsigned int)value)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
        m_currentValue = (unsigned int)value;
        if ((unsigned int)value < m_possibleLabels.size())
            setLabel(UIString(m_possibleLabels[value]), false, false);
    }
}

void UIControl_Slider::SetSliderTouchPos(float position)
{
    static_cast<FJ_Slider*>(m_fuiRenderNode->getFuiNode())->SetRelativeSliderPos(position);
}

void UIControl_Slider::SetSliderValue(int value)
{
    static_cast<FJ_Slider*>(m_fuiRenderNode->getFuiNode())->SetSliderValue(value);
}

void UIControl_Slider::MoveSliderValue(int amount)
{
    int value = std::max(m_minimum, std::min(m_maximum, amount + (int)m_currentValue));
    SetSliderValue(value);
    handleSliderMove(value);
}

void UIControl_Slider::setAllPossibleLabels(int firstLabel, unsigned int labelCount, wchar_t (*labels)[256])
{
    m_possibleLabels.clear();

    unsigned int i;
    for (i = 0; i < labelCount; ++i)
        m_possibleLabels.push_back(std::wstring(labels[i]));

    UIControl_Base::setAllPossibleLabels(firstLabel, labelCount, labels);
}

unsigned int UIControl_Slider::getCurrentValue() const
{
    return m_currentValue;
}

int UIControl_Slider::GetRealWidth()
{
    return (int)static_cast<FJ_Slider*>(m_fuiRenderNode->getFuiNode())->GetRealWidth();
}

void UIControl_Slider::ReInit()
{
    UIControl::ReInit();
    init(m_label, m_controlId, m_minimum, m_maximum, (int)m_currentValue, m_repeat, m_repeatDelay, m_repeatStep);
}
