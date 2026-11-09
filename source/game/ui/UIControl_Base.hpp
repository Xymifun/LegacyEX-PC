#ifndef GAME_UI_UICONTROL_BASE_HPP
#define GAME_UI_UICONTROL_BASE_HPP

#include "game/ui/UIControl.hpp"
#include "game/ui/UIString.hpp"

class UIControl_Base : public UIControl
{
public:
    UIControl_Base();
    virtual ~UIControl_Base();
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void tick();
    virtual bool hasFocus();
    virtual void setLabel(UIString label, bool update, bool force);
    virtual void setAllPossibleLabels(int firstLabel, unsigned int labelCount, wchar_t (*labels)[256]);
    const wchar_t* getLabel();

protected:
    bool m_labelChanged;
    UIString m_label;
};

#endif
