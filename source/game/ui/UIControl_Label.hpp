#ifndef GAME_UI_UICONTROL_LABEL_HPP
#define GAME_UI_UICONTROL_LABEL_HPP

#include "game/ui/UIControl_Base.hpp"

class UIControl_Label : public UIControl_Base
{
public:
    UIControl_Label();
    virtual ~UIControl_Label() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void init(UIString label);
    virtual void ReInit();

private:
    bool m_reinit;
};

#endif
