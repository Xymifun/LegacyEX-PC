#ifndef GAME_UI_UICONTROL_HTMLLABEL_HPP
#define GAME_UI_UICONTROL_HTMLLABEL_HPP

#include "game/ui/UIControl_Label.hpp"

class UIControl_HTMLLabel : public UIControl_Label
{
public:
    UIControl_HTMLLabel();
    virtual ~UIControl_HTMLLabel() {}

    int GetRealWidth();
    int GetRealHeight();
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void ReInit();
    virtual void SetupTouch();
    virtual void TouchScroll(int offset, bool active);
    void startAutoScroll();
};

#endif
