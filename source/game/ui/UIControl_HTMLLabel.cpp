#include "game/ui/UIControl_HTMLLabel.hpp"

#include "game/ui/FJ_DynamicHtmlText.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern ConsoleUIController gConsoleUIController;

UIControl_HTMLLabel::UIControl_HTMLLabel() : UIControl_Label()
{
}

int UIControl_HTMLLabel::GetRealWidth()
{
    if (m_fuiRenderNode->getFuiNode()->isFJ_HtmlText())
        return (int)m_fuiRenderNode->getFuiNode()->asFJ_HtmlText()->GetRealWidth();

    int size = 0;
    if (m_fuiRenderNode->getFuiNode()->isFJ_DynamicHtmlText())
        size = (int)m_fuiRenderNode->getFuiNode()->asFJ_DynamicHtmlText()->GetRealWidth();

    return size;
}

int UIControl_HTMLLabel::GetRealHeight()
{
    if (m_fuiRenderNode->getFuiNode()->isFJ_HtmlText())
        return (int)m_fuiRenderNode->getFuiNode()->asFJ_HtmlText()->GetRealHeight();

    int size = 0;
    if (m_fuiRenderNode->getFuiNode()->isFJ_DynamicHtmlText())
        size = (int)m_fuiRenderNode->getFuiNode()->asFJ_DynamicHtmlText()->GetRealHeight();

    return size;
}

bool UIControl_HTMLLabel::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_HTMLLabel;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_HTMLLabel::ReInit()
{
    UIControl::ReInit();
    init(UIString(L""));
}

void UIControl_HTMLLabel::SetupTouch()
{
    int layerId = m_scene->m_layer->m_layerId;
    switch (layerId)
    {
    case 4:
    case 6:
    case 7:
        gConsoleUIController.TouchBoxAdd(this, m_scene);
        break;
    default:
        break;
    }
}

void UIControl_HTMLLabel::TouchScroll(int offset, bool active)
{
    if (m_fuiRenderNode->getFuiNode()->isFJ_HtmlText())
    {
        m_fuiRenderNode->getFuiNode()->asFJ_HtmlText()->TouchScroll(offset, active);
        return;
    }

    if (m_fuiRenderNode->getFuiNode()->isFJ_DynamicHtmlText())
        m_fuiRenderNode->getFuiNode()->asFJ_DynamicHtmlText()->TouchScroll(offset, active);
}
