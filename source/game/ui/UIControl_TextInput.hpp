#ifndef GAME_UI_UICONTROL_TEXTINPUT_HPP
#define GAME_UI_UICONTROL_TEXTINPUT_HPP

#include "game/ui/UIControl_Base.hpp"

class UIControl_TextInput : public UIControl_Base
{
public:
    UIControl_TextInput();
    virtual ~UIControl_TextInput() {}
    void SetCharLimit(int limit);
    void init(UIString label, int id);
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void ReInit();
    virtual void setFocus(bool focus);
    virtual bool hasFocus();
    void MoveCaretRight();
    void MoveCaretLeft();

private:
    bool m_focus;
};

#endif
