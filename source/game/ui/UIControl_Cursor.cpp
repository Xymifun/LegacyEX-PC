#include "game/ui/UIControl_Cursor.hpp"

#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_Cursor::UIControl_Cursor()
{
}

bool UIControl_Cursor::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_Cursor;
    return UIControl_Base::setupControl(scene, node, name);
}
