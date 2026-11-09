#ifndef GAME_UI_UICONTROL_LAYERICONBUTTONLIST_HPP
#define GAME_UI_UICONTROL_LAYERICONBUTTONLIST_HPP

#include <string>

#include "game/ui/UIControl_List.hpp"

class UIControl_LayerIconButtonList : public UIControl_List
{
public:
    UIControl_LayerIconButtonList();
    virtual ~UIControl_LayerIconButtonList() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void UseIggyIcons();
    void AddNewItem(const std::wstring& label, const std::wstring& secondaryText, int id,
                    const std::wstring& textureName);
    void SetLayerLabel(int index, const std::wstring& label);
    void SetTextureName(int index, const std::wstring& textureName);
    void SetItem(int index, const std::wstring& label, const std::wstring& layerLabel, const std::wstring& textureName);
};

#endif
