#include "game/ui/UIControl.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl::UIControl() : m_scene(0), m_opacity(1.0f)
{
    m_name = "";
    m_controlType = eUIControlType_Unknown;
    m_removed = false;
    m_fuiRenderNode = 0;
    m_visible = true;
}

void UIControl::setVisible(bool visible)
{
    if (m_fuiRenderNode != 0)
    {
        FJ_FuiNode* fuiNode = m_fuiRenderNode->getFuiNode();
        if (fuiNode == 0)
        {
            m_fuiRenderNode->setVisibility(visible);
        }
        else
        {
            fuiNode->setVisible(visible);
        }
        m_visible = visible;
    }
}

float UIControl::getWidth()
{
    return m_width;
}

float UIControl::getHeight()
{
    return m_height;
}

float UIControl::getXPos()
{
    return m_x;
}

float UIControl::getYPos()
{
    return m_y;
}

bool UIControl::getVisible()
{
    bool visible = false;
    if (m_fuiRenderNode != 0)
    {
        visible = m_fuiRenderNode->isVisible();
    }
    m_visible = visible;
    return visible;
}

void UIControl::removeUINodeRef()
{
    m_fuiRenderNode = 0;
}

bool UIControl::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_scene = scene;
    m_name.assign(name, 0, std::string::npos);
    m_fuiRenderNode = node->findNode(name.c_str());
    if (m_fuiRenderNode != 0)
    {
        m_x = m_fuiRenderNode->getX();
        m_y = m_fuiRenderNode->getY();
        m_width = m_fuiRenderNode->getWidth();
        m_height = m_fuiRenderNode->getHeight();
    }
    return m_fuiRenderNode != 0;
}

void UIControl::UpdateControl()
{
    if (m_fuiRenderNode != 0)
    {
        m_x = m_fuiRenderNode->getX();
        m_y = m_fuiRenderNode->getY();
        m_width = m_fuiRenderNode->getWidth();
        m_height = m_fuiRenderNode->getHeight();
    }
}

fuiRenderNode* UIControl::getFuiRenderNode()
{
    return m_fuiRenderNode;
}

void UIControl::ReInit()
{
    if (m_fuiRenderNode != 0)
    {
        m_fuiRenderNode->setAlpha(m_opacity);
        m_fuiRenderNode->setVisibility(m_visible);
    }
}

void UIControl::setWidth(float width)
{
    if (m_fuiRenderNode != 0)
    {
        m_fuiRenderNode->setWidth(width);
        m_width = width;
    }
}

void UIControl::setOpacity(float opacity)
{
    if (opacity == m_opacity)
    {
        return;
    }
    m_opacity = opacity;
    if (m_fuiRenderNode != 0)
    {
        m_fuiRenderNode->setAlpha(opacity);
    }
}

void UIControl::tick()
{
}

void UIControl::setFocus(bool)
{
}

bool UIControl::hasFocus()
{
    return false;
}

UIControl& UIControl::operator=(const UIControl& other)
{
    m_controlType = other.m_controlType;
    m_controlId = other.m_controlId;
    m_removed = other.m_removed;
    m_name = other.m_name;
    m_fuiRenderNode = other.m_fuiRenderNode;
    m_scene = other.m_scene;
    m_x = other.m_x;
    m_y = other.m_y;
    m_width = other.m_width;
    m_height = other.m_height;
    m_opacity = other.m_opacity;
    m_visible = other.m_visible;
    return *this;
}
