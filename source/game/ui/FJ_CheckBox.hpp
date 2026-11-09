#ifndef GAME_UI_FJ_CHECKBOX_HPP
#define GAME_UI_FJ_CHECKBOX_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_CheckBox : public FJ_Base
{
public:
    explicit FJ_CheckBox(fuiRenderNode* renderNode);
    virtual ~FJ_CheckBox() {}

    void UpdateCheckBox();
    void SetCheckBox(bool checked);
    void ChangeState(int state);
    void EnableCheckBox(bool enabled);
    bool getChecked();
    virtual int getOldWidth();
    void focusInHandler(FJ_Event* event);
    static void _focusInHandler(FJ_FuiNode* node, FJ_Event* event);
    void focusOutHandler(FJ_Event* event);
    static void _focusOutHandler(FJ_FuiNode* node, FJ_Event* event);
    void keyDownHandler(FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    void keyUpHandler(FJ_Event* event);
    static void _keyUpHandler(FJ_FuiNode* node, FJ_Event* event);
    void UpdateLabel();
    void HandleUpdateLabel(FJ_Event* event);
    static void _HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event);
    void Init(const std::wstring& name, int id, bool checked);
    virtual void GetLabelTooWide(fuiRenderNodeEditText* textField);
    virtual void SetNewObjectWidth(int width);
    uint8_t getEnabled();
    uint32_t getCurrentState();

    static bool s_keyDown;

private:
    bool m_checked;
    bool m_enabled;
    int m_currentState;
    int m_labelWidth;
    int m_oldWidth;
};

#endif
