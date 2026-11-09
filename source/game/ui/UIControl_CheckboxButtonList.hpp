#ifndef GAME_UI_UICONTROL_CHECKBOXBUTTONLIST_HPP
#define GAME_UI_UICONTROL_CHECKBOXBUTTONLIST_HPP

#include <stdint.h>
#include <string>

#include "game/ui/UIControl_List.hpp"

class UIScene;
class fuiRenderNode;

class UIControl_CheckboxButtonList : public UIControl_List
{
public:
    UIControl_CheckboxButtonList();
    virtual ~UIControl_CheckboxButtonList() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void UseIggyIcons();
    void AddNewItem(const std::wstring& label, int id, bool checked, const std::wstring& textureName);
    void SetCheckbox(int index, bool checked);
    uint8_t GetCheckbox(int index);
    void SetTextureName(int index, const std::wstring& textureName);
};

#endif
