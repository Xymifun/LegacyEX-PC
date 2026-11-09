#include "game/ui/UIControl_Touch.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Base.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/ConsoleUIController.hpp"

extern ConsoleUIController gConsoleUIController;

UIControl_Touch::UIControl_Touch() : UIControl_Base()
{
}

bool UIControl_Touch::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_Touch;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Touch::init(int controlId)
{
    int layerId = m_scene->m_layer->m_layerId;
    m_controlId = controlId;
    switch (layerId)
    {
    case EUILayer_Alert:
    case EUILayer_Fullscreen:
    case EUILayer_Scene:
    case EUILayer_HUD:
        gConsoleUIController.TouchBoxAdd(this, m_scene);
        break;
    }
}

void UIControl_Touch::ReInit()
{
    UIControl::ReInit();
    init(m_controlId);
}
