#include "game/ui/UIControl_BeaconEffectButton.hpp"

#include "game/ui/FJ_BeaconEffectButton.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_BeaconEffectButton::UIControl_BeaconEffectButton() : UIControl()
{
    m_icon = 0;
    m_state = 0;
    m_data = 0;
    m_active = false;
    m_focused = false;
    m_selected = false;
}

bool UIControl_BeaconEffectButton::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    return UIControl::setupControl(scene, node, name);
}

void UIControl_BeaconEffectButton::SetIcon(int icon)
{
    if (icon == m_icon)
    {
        return;
    }
    m_icon = icon;
    static_cast<FJ_BeaconEffectButton*>(m_fuiRenderNode->getFuiNode())->SetIcon(icon);
}

void UIControl_BeaconEffectButton::UpdateButtonState()
{
    int state = 0;
    if (m_active)
    {
        if (m_selected)
        {
            state = 3;
            if (state == m_state)
            {
                return;
            }
        }
        else
        {
            state = m_focused ? 2 : 1;
        }
    }
    if (state == m_state)
    {
        return;
    }
    static_cast<FJ_BeaconEffectButton*>(m_fuiRenderNode->getFuiNode())->ChangeState(state);
    m_state = state;
}

void UIControl_BeaconEffectButton::ReInit()
{
    UIControl::ReInit();
    int icon = m_icon;
    m_icon = 0;
    SetIcon(icon);
    m_state = 0;
    UpdateButtonState();
}

void UIControl_BeaconEffectButton::SetData(int data, int icon, bool active, bool selected)
{
    m_data = data;
    m_active = active;
    m_selected = selected;
    SetIcon(icon);
    UpdateButtonState();
}

unsigned int UIControl_BeaconEffectButton::GetData()
{
    return (unsigned int)m_data;
}

void UIControl_BeaconEffectButton::SetButtonSelected(bool selected)
{
    if (selected == m_selected)
    {
        return;
    }
    m_selected = selected;
    UpdateButtonState();
}

unsigned char UIControl_BeaconEffectButton::IsButtonSelected()
{
    return (unsigned char)m_selected;
}

void UIControl_BeaconEffectButton::SetButtonActive(bool active)
{
    if (active == m_active)
    {
        return;
    }
    m_active = active;
    UpdateButtonState();
}

void UIControl_BeaconEffectButton::setFocus(bool focus)
{
    if (focus == m_focused)
    {
        return;
    }
    m_focused = focus;
    UpdateButtonState();
}
