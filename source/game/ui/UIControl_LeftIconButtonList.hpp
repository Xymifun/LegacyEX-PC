#ifndef GAME_UI_UICONTROL_LEFTICONBUTTONLIST_HPP
#define GAME_UI_UICONTROL_LEFTICONBUTTONLIST_HPP

#include <string>

#include "game/ui/UIControl_ButtonList.hpp"

class UIScene;
class fuiRenderNode;

class UIControl_LeftIconButtonList : public UIControl_ButtonList
{
public:
    UIControl_LeftIconButtonList();
    virtual ~UIControl_LeftIconButtonList() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void UseIggyIcons();
    void AddNewItem(const std::wstring& label, int id, const std::wstring& textureName);
    void AddNewItem(const std::wstring& label, int id, const std::wstring& textureName,
                    const std::wstring& textureName1);
    void AddNewItem(const std::wstring& label, const std::wstring& textureName);
    void AddNewItem(const std::wstring& label, const std::wstring& textureName, const std::wstring& textureName1);
    void SetTextureName(int index, const std::wstring& textureName);
    void SetItem(int index, const std::wstring& label, const std::wstring& textureName);
};

#endif
