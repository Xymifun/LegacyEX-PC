#ifndef GAME_UI_UICONTROL_CURSOR_HPP
#define GAME_UI_UICONTROL_CURSOR_HPP

#include "game/ui/UIControl_Base.hpp"

class UIControl_Cursor : public UIControl_Base
{
public:
    UIControl_Cursor();
    virtual ~UIControl_Cursor() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
};

#endif
