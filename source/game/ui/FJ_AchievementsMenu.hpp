#ifndef GAME_UI_FJ_ACHIEVEMENTSMENU_HPP
#define GAME_UI_FJ_ACHIEVEMENTSMENU_HPP

#include <string>

#include "game/ui/FJ_Document.hpp"

class FJ_FuiNode;
class FJ_List2D;
class fuiRenderNode;

class FJ_AchievementsMenu : public FJ_Document
{
private:
    FJ_List2D* m_achievementsList;

public:
    FJ_AchievementsMenu(fuiRenderNode* renderNode);

    void PositionAchievementDescription();
    static void _PositionAchievementDescription(FJ_FuiNode* node);
    static FJ_AchievementsMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void SetAchievementDescription(const std::wstring& description);
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
