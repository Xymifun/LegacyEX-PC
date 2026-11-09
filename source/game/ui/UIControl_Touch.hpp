#ifndef GAME_UI_UICONTROL_TOUCH_HPP
#define GAME_UI_UICONTROL_TOUCH_HPP

#include <string>
#include "game/ui/UIControl_Base.hpp"

class UIControl_Touch : public UIControl_Base
{
public:
    UIControl_Touch();
    virtual ~UIControl_Touch() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void init(int controlId);
    virtual void ReInit();
};

#endif
