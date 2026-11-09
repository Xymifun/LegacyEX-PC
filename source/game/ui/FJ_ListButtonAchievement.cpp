#include "game/ui/FJ_ListButtonAchievement.hpp"

#include "game/ui/FJ_BitmapIcon.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

static std::string s_buttonStateLabels[2] = {std::string("Button_Enabled"), std::string("Button_Disabled")};

FJ_ListButtonAchievement::FJ_ListButtonAchievement(fuiRenderNode* pRenderNode) : FJ_Base(pRenderNode)
{
    m_bitmapIcon = getChildByName("BitmapIcon")->asFJ_BitmapIcon();
    m_highlight = getChildByName("AchievementEntry_Highlight")->asMovieClip();
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    m_highlight->setVisible(false);
}

void FJ_ListButtonAchievement::EnableButton(bool enabled)
{
    gotoAndStop(s_buttonStateLabels[enabled ^ 1]);
    m_bitmapIcon->setVisible(enabled);
    m_enabled = enabled;
}

void FJ_ListButtonAchievement::focusInHandler(FJ_Event*)
{
    m_highlight->setVisible(true);
}

void FJ_ListButtonAchievement::_focusInHandler(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    static_cast<FJ_ListButtonAchievement*>(pNode)->focusInHandler(pEvent);
}

void FJ_ListButtonAchievement::focusOutHandler(FJ_Event*)
{
    m_highlight->setVisible(false);
}

void FJ_ListButtonAchievement::_focusOutHandler(FJ_FuiNode* pNode, FJ_Event* pEvent)
{
    static_cast<FJ_ListButtonAchievement*>(pNode)->focusOutHandler(pEvent);
}

void FJ_ListButtonAchievement::SetTextureName(const std::wstring& textureName)
{
    m_bitmapIcon->SetTextureName(textureName);
}

void FJ_ListButtonAchievement::Init(int, const std::wstring& textureName)
{
    SetTextureName(textureName);
}
