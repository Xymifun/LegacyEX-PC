#include "game/ui/UIControl_Label.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LabelOutline.hpp"
#include "game/ui/FJ_Label_Blink.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiRenderNode.hpp"

#include <string>

UIControl_Label::UIControl_Label() : UIControl_Base(), m_reinit(true)
{
}

bool UIControl_Label::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_Label;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Label::init(UIString label)
{
    m_label = label;

    if (m_fuiRenderNode == 0)
        return;

    if (m_fuiRenderNode->getFuiNode()->isFJ_Label())
    {
        m_fuiRenderNode->getFuiNode()->asFJ_Label()->Init(label.getString());
        return;
    }

    if (m_fuiRenderNode->getFuiNode()->isFJ_Label_Blink())
    {
        std::wstring& text = label.getString();
        m_fuiRenderNode->getFuiNode()->asFJ_Label_Blink()->Init(text);
        return;
    }

    if (m_fuiRenderNode->getFuiNode()->isFJ_HtmlText())
    {
        m_fuiRenderNode->getFuiNode()->asFJ_HtmlText()->Init(label.getString());
        return;
    }

    if (m_fuiRenderNode->getFuiNode()->isFJ_LabelOutline())
    {
        std::wstring& text = label.getString();
        m_fuiRenderNode->getFuiNode()->asFJ_LabelOutline()->Init(text);
    }
}

void UIControl_Label::ReInit()
{
    UIControl::ReInit();
    if (m_reinit)
    {
        init(m_label);
    }
}
