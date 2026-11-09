#ifndef GAME_UI_UICONTROL_BUTTONLIST_HPP
#define GAME_UI_UICONTROL_BUTTONLIST_HPP

#include <string>

#include "game/ui/UIControl_List.hpp"

class UIScene;
class fuiRenderNode;

class UIControl_ButtonList : public UIControl_List
{
public:
    UIControl_ButtonList();
    virtual ~UIControl_ButtonList() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void addItem(const std::wstring& label, int index);
    void addItem(const std::wstring& label);
    void setButtonLabel(int item, const std::wstring& label);
    void addItem(const std::string& label, int index);
    void addItem(const std::string& label);
};

#endif
