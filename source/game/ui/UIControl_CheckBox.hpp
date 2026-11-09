#ifndef GAME_UI_UICONTROL_CHECKBOX_HPP
#define GAME_UI_UICONTROL_CHECKBOX_HPP

#include "game/ui/UIControl_Base.hpp"

class UIControl_CheckBox : public UIControl_Base
{
public:
    UIControl_CheckBox();
    virtual ~UIControl_CheckBox() {}
    unsigned char IsEnabled();
    void setChecked(bool checked);
    bool IsChecked();
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void init(UIString label, int id, bool checked);
    unsigned char TouchIsChecked();
    void SetEnable(bool enable);
    void updateCheckedStatus(bool checked);
    virtual void ReInit();

private:
    bool m_checked;
    bool m_enabled;
};

#endif
