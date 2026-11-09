#ifndef GAME_UI_UICONTROL_SAVELIST_HPP
#define GAME_UI_UICONTROL_SAVELIST_HPP

#include <string>

#include "game/ui/UIControl_ButtonList.hpp"

class UIControl_SaveList : public UIControl_ButtonList
{
public:
    virtual ~UIControl_SaveList() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void addItem(const std::wstring& label, const std::wstring& textureName, int index);
    void addItem(const std::wstring& label, const std::wstring& textureName);
    void setTextureName(int item, const std::wstring& textureName);
    void addItem(const std::wstring& label);
    void addItem(const std::string& label, const std::wstring& textureName, int index);
    void addItem(const std::string& label, const std::wstring& textureName);
    void addItem(const std::string& label);
    void addItem(const std::wstring& label, int index);
    void addItem(const std::string& label, int index);
};

#endif
