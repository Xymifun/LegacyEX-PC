#ifndef GAME_UI_UICONTROL_DLCBUTTONLIST_HPP
#define GAME_UI_UICONTROL_DLCBUTTONLIST_HPP

#include "game/ui/UIControl_ButtonList.hpp"
#include <string>

class UIScene;
class fuiRenderNode;

class UIControl_DLCButtonList : public UIControl_ButtonList
{
public:
    virtual ~UIControl_DLCButtonList() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void addItem(const std::string& label, bool enabled, int index);
    void addItem(const std::wstring& label, bool enabled, int index);
    void showTick(int item, bool show);
};

#endif
