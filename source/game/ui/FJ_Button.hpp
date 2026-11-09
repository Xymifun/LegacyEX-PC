#ifndef GAME_UI_FJ_BUTTON_HPP
#define GAME_UI_FJ_BUTTON_HPP

#include <stdint.h>
#include <string>
#include "game/ui/FJ_Base.hpp"

class FJ_Button;
class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;
class fuiRenderNodeEditText;

typedef void FJ_ButtonKeyDownDelegateFn(FJ_FuiNode*, int);
typedef void FJ_ButtonUpdateLabelDelegateFn(FJ_Button*);

class FJ_Button : public FJ_Base
{
public:
    explicit FJ_Button(fuiRenderNode* renderNode);
    virtual ~FJ_Button() {}

    void setButtonPressed(bool pressed);
    virtual void Init(const std::wstring& name, int id);
    void ToggleOutline(bool show);
    void focusInHandler(FJ_Event* event);
    static void _focusInHandler(FJ_FuiNode* node, FJ_Event* event);
    void focusOutHandler(FJ_Event* event);
    static void _focusOutHandler(FJ_FuiNode* node, FJ_Event* event);
    void keyDownHandler(FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    void keyUpHandler(FJ_Event* event);
    static void _keyUpHandler(FJ_FuiNode* node, FJ_Event* event);
    void HideUntilInit();
    void HandleUpdateLabel(FJ_Event* event);
    static void _HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event);
    virtual void ChangeState(int state);
    void SetKeyDownDelegate(void (*delegate)(FJ_FuiNode*, int), FJ_FuiNode* node);
    fuiRenderNodeEditText* GetSecondaryTextField();
    void SetSecondaryText(const std::wstring& text, bool html, bool autoSize);
    virtual void UpdateLabel();
    virtual void SetLabel(const std::wstring& label);
    void AdjustDisabledTint();
    virtual void EnableButton(bool enabled);
    virtual void GetLabelTooWide(fuiRenderNodeEditText* textField);

    bool getButtonPressed() { return m_buttonPressed; }
    bool getShowOutline() { return m_showOutline; }
    bool getEnabled() { return m_enabled; }
    int getCurrentState() { return m_currentState; }

private:
    friend class FJ_KeyboardButtons;
    friend class FJ_ListButtonCheckbox;

    bool m_buttonPressed;
    bool m_showOutline;
    bool m_enabled;
    int m_currentState;
    FJ_ButtonKeyDownDelegateFn* m_keyDownDelegate;
    FJ_FuiNode* m_keyDownDelegateNode;
    bool m_focused;
    bool m_hiddenUntilInit;
    bool m_hasSecondaryText;
    std::wstring m_secondaryText;
    float m_secondaryWidth;
    float m_secondaryX;
    FJ_ButtonUpdateLabelDelegateFn* m_updateLabelDelegate;
    bool m_secondaryHtml;
    bool m_secondaryAutoSize;
};

#endif
