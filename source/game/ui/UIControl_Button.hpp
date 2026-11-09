#ifndef GAME_UI_UICONTROL_BUTTON_HPP
#define GAME_UI_UICONTROL_BUTTON_HPP

#include "game/ui/UIControl_Base.hpp"

class UIControl_Button : public UIControl_Base
{
public:
    UIControl_Button();
    virtual ~UIControl_Button() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void init(UIString label, int id);
    virtual void ReInit();
    void setEnable(bool enable, bool immediate);
    virtual void tick();
    virtual void setFocus(bool focus);

private:
    bool m_focus;
    bool m_enablePending;
    bool m_enabled;
};

#endif
