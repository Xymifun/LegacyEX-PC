#ifndef GAME_UI_UICONTROL_ACHIEVEMENTSLIST_HPP
#define GAME_UI_UICONTROL_ACHIEVEMENTSLIST_HPP

#include <string>

#include "game/ui/UIControl_GridList.hpp"

class UIControl_AchievementsList : public UIControl_GridList
{
public:
    UIControl_AchievementsList();
    virtual ~UIControl_AchievementsList() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void AddNewItem(int id, const std::wstring& textureName);
};

#endif
