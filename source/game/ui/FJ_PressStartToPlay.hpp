#ifndef GAME_UI_FJ_PRESSSTARTTOPLAY_HPP
#define GAME_UI_FJ_PRESSSTARTTOPLAY_HPP

#include <string>

#include "game/ui/FJ_Document.hpp"
#include "game/ui/FJ_FuiNode.hpp"

class FJ_PressStartToPlay : public FJ_Document
{
private:
    int m_panelWidth;
    int m_panelHeight;
    int m_panelX;
    FJ_FuiNode* m_mainPanel;
    FJ_FuiNode* m_achievementToast;
    int m_achievementToastX;

public:
    FJ_PressStartToPlay(fuiRenderNode* renderNode);

    static FJ_PressStartToPlay* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void ShowAchievementToast(const std::wstring& title, const std::wstring& description, const std::wstring& gamerPic,
                              const std::wstring& achievementIcon);
    void HideAchievementToast();
    void ResizePanel(bool multiLine);
    void SetText(const std::wstring& text);
    void SetMultiLineText(const std::wstring& text);
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    void ShowController(int controller, bool show);
    void HideChestRefillAnimation();
    void ShowChestRefillAnimation(bool left, int yOffset);
    void ShowBetaWatermark();
};

#endif
