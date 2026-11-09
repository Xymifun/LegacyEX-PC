#ifndef GAME_UI_UICONTROL_SLIDER_HPP
#define GAME_UI_UICONTROL_SLIDER_HPP

#include <string>
#include <vector>

#include "game/ui/UIControl_Base.hpp"

class UIScene;
class fuiRenderNode;

class UIControl_Slider : public UIControl_Base
{
public:
    UIControl_Slider();
    virtual ~UIControl_Slider() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void ReInit();
    virtual void setFocus(bool focus);
    virtual void setAllPossibleLabels(int firstLabel, unsigned int labelCount, wchar_t (*labels)[256]);
    void init(UIString label, int controlId, int minimum, int maximum, int value, bool repeat, int repeatDelay,
              int repeatStep);
    void handleSliderMove(int value);
    void SetSliderTouchPos(float position);
    void SetSliderValue(int value);
    void MoveSliderValue(int amount);
    unsigned int getCurrentValue() const;
    int GetRealWidth();

private:
    int m_minimum;
    int m_maximum;
    unsigned int m_currentValue;
    bool m_focus;
    bool m_repeat;
    int m_repeatDelay;
    int m_repeatStep;
    std::vector<std::wstring> m_possibleLabels;
};

#endif
