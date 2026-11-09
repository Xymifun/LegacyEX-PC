#ifndef GAME_UI_UICONTROL_LOBBYBUTTONLIST_HPP
#define GAME_UI_UICONTROL_LOBBYBUTTONLIST_HPP

#include <string>

#include "game/ui/UIControl_ButtonList.hpp"

class UIScene;
class fuiRenderNode;

class UIControl_LobbyButtonList : public UIControl_ButtonList
{
public:
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
};

#endif
