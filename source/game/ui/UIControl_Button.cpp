#include "game/ui/UIControl_Button.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/ConsoleUIController.hpp"

extern ConsoleUIController gConsoleUIController;

UIControl_Button::UIControl_Button() : UIControl_Base(), m_focus(false), m_enablePending(false), m_enabled(true)
{
}

bool UIControl_Button::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_Button;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Button::init(UIString label, int id)
{
    m_label = label;
    m_controlId = id;
    if (m_fuiRenderNode->getFuiNode()->isFJ_Button())
    {
        m_fuiRenderNode->getFuiNode()->asFJ_Button()->Init(label.getString(), id);
    }
    switch (m_scene->m_layer->m_layerId)
    {
    case EUILayer_Alert:
    case EUILayer_Fullscreen:
    case EUILayer_Scene:
    case EUILayer_HUD:
        gConsoleUIController.TouchBoxAdd(this, m_scene);
        break;
    }
}

void UIControl_Button::ReInit()
{
    UIControl::ReInit();
    init(m_label, m_controlId);
}

void UIControl_Button::setEnable(bool enable, bool immediate)
{
    if (enable == m_enabled)
    {
        return;
    }
    m_enabled = enable;
    if (!immediate)
    {
        m_enablePending = true;
        return;
    }
    m_fuiRenderNode->getFuiNode()->asFJ_Button()->EnableButton(enable);
}

void UIControl_Button::tick()
{
    UIControl_Base::tick();
    if (m_enablePending)
    {
        setEnable(m_enabled, true);
    }
}

void UIControl_Button::setFocus(bool focus)
{
    if (m_focus == focus)
    {
        return;
    }
    m_focus = focus;
    m_fuiRenderNode->getFuiNode()->asFJ_Button()->ChangeState(focus ? 1 : 2);
}
