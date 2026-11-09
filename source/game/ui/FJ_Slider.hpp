#ifndef GAME_UI_FJ_SLIDER_HPP
#define GAME_UI_FJ_SLIDER_HPP

#include <stdint.h>
#include <string>
#include "game/ui/FJ_Base.hpp"

class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_Slider : public FJ_Base
{
public:
    explicit FJ_Slider(fuiRenderNode* renderNode);
    virtual ~FJ_Slider() {}
    uint8_t getEnabled();
    uint32_t getSliderCurrent();
    virtual int getOldWidth();
    void UpdateSliderScaleMask();
    void SetSlider(int value);
    void ChangeState(int state);
    void InitSliderMask();
    void focusInHandler(FJ_Event* event);
    static void _focusInHandler(FJ_FuiNode* node, FJ_Event* event);
    void focusOutHandler(FJ_Event* event);
    static void _focusOutHandler(FJ_FuiNode* node, FJ_Event* event);
    void keyUpHandler(FJ_Event* event);
    static void _keyUpHandler(FJ_FuiNode* node, FJ_Event* event);
    void MoveSlider(int amount);
    void keyDownHandler(FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    void UpdateLabel();
    void HandleUpdateLabel(FJ_Event* event);
    static void _HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event);
    virtual void ASConstructor();
    void Init(const std::wstring& name, int id, int minValue, int maxValue, int value, bool repeat, int repeatDelay,
              int repeatStep);
    void SetRelativeSliderPos(float position);
    float GetRealWidth();
    void SetSliderValue(int value);
    void EnableSlider(bool enabled);
    virtual void GetLabelTooWide(fuiRenderNodeEditText* textField);
    virtual void SetNewObjectWidth(int width);
    uint32_t getCurrentState();
    void AdjustDisabledTint();

private:
    int m_minValue;
    int m_maxValue;
    int m_sliderCurrent;
    int m_currentState;
    float m_leftPosition;
    float m_rightPosition;
    bool m_enabled;
    bool m_maskInitialized;
    FJ_FuiNode* m_sliderMask;
    FJ_FuiNode* m_tintActiveMask;
    FJ_FuiNode* m_tintInactiveMask;
    int m_keyPressDelta;
    int m_keyPressTicks;
    bool m_repeat;
    int m_repeatDelay;
    int m_repeatStep;
};

#endif
