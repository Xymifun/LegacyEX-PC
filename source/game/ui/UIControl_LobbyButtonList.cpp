#include "game/ui/UIControl_LobbyButtonList.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

bool UIControl_LobbyButtonList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_LobbyButtonList;
    return UIControl_ButtonList::setupControl(scene, node, name);
}
