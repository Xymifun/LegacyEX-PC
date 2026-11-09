#ifndef GAME_UI_UICONTROL_DYNAMICLABEL_HPP
#define GAME_UI_UICONTROL_DYNAMICLABEL_HPP

#include "game/ui/UIControl_Label.hpp"

class UIControl_DynamicLabel : public UIControl_Label
{
public:
    UIControl_DynamicLabel();
    virtual ~UIControl_DynamicLabel() {}
    int GetRealWidth();
    int GetRealHeight();
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void addText(const std::wstring& text, bool setup);
    virtual void ReInit();
    virtual void SetupTouch();
    virtual void TouchScroll(int offset, bool active);
};

#endif
