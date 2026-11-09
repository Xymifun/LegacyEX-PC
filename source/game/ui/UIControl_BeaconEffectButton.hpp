#ifndef GAME_UI_UICONTROL_BEACONEFFECTBUTTON_HPP
#define GAME_UI_UICONTROL_BEACONEFFECTBUTTON_HPP

#include "game/ui/UIControl.hpp"

class UIControl_BeaconEffectButton : public UIControl
{
private:
    int m_state;
    int m_data;
    int m_icon;
    bool m_selected;
    bool m_active;
    bool m_focused;

public:
    UIControl_BeaconEffectButton();
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void SetIcon(int icon);
    void UpdateButtonState();
    virtual void ReInit();
    void SetData(int data, int icon, bool active, bool selected);
    unsigned int GetData();
    void SetButtonSelected(bool selected);
    unsigned char IsButtonSelected();
    void SetButtonActive(bool active);
    virtual void setFocus(bool focus);
};

#endif
