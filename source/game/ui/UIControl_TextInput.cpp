#include "game/ui/UIControl_TextInput.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_SignInput.hpp"
#include "game/ui/FJ_TextInput.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_TextInput::UIControl_TextInput() : UIControl_Base(), m_focus(false)
{
}

void UIControl_TextInput::SetCharLimit(int limit)
{
    if (m_fuiRenderNode->getFuiNode()->isFJ_TextInput())
        static_cast<FJ_TextInput*>(m_fuiRenderNode->getFuiNode())->SetCharLimit(limit);
}

void UIControl_TextInput::init(UIString label, int id)
{
    m_label = label;
    m_controlId = id;

    if (m_fuiRenderNode->getFuiNode()->isFJ_TextInput())
    {
        std::wstring& text = label.getString();
        m_fuiRenderNode->getFuiNode()->asFJ_TextInput()->Init(text, id);
    }
    else if (m_fuiRenderNode->getFuiNode()->isFJ_SignInput())
    {
        std::wstring& text = label.getString();
        m_fuiRenderNode->getFuiNode()->asFJ_SignInput()->Init(text, id);
    }

    switch (m_scene->m_layer->m_layerId)
    {
    case EUILayer_Fullscreen:
    case EUILayer_Scene:
    case EUILayer_HUD:
        gConsoleUIController.TouchBoxAdd(this, m_scene);
        break;
    }

    SetCharLimit(101);
}

bool UIControl_TextInput::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_TextInput;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_TextInput::ReInit()
{
    UIControl::ReInit();
    init(m_label, m_controlId);
}

void UIControl_TextInput::setFocus(bool focus)
{
    if (m_focus == focus)
        return;

    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    m_focus = focus;
    if (node->isFJ_TextInput())
        static_cast<FJ_TextInput*>(m_fuiRenderNode->getFuiNode())->ChangeState(focus ^ 1);
}

bool UIControl_TextInput::hasFocus()
{
    if (!m_focus)
        return m_scene->controlHasFocus(this);
    return true;
}
