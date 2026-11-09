#include "game/ui/FJ_AchievementsMenu.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_HtmlTextPanel.hpp"
#include "game/ui/FJ_HtmlTextPanelLarge.hpp"
#include "game/ui/FJ_List2D.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_AchievementsMenu::FJ_AchievementsMenu(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    if (getChildByName(std::string("GroupMoreDetails")) != NULL)
        getChildByName(std::string("GroupMoreDetails"))->setVisible(false);

    if (getChildByName(std::string("Overlay")) != NULL)
        getChildByName(std::string("Overlay"))->setVisible(false);

    m_root = getChildByName(std::string("AchievementsListContainer"))->asMovieClip();
    m_achievementsList = static_cast<FJ_List2D*>(m_root->getChildByName(std::string("AchievementsList")));
    m_achievementsList->SetScrollDelegate(&_PositionAchievementDescription, this);

    FJ_FuiNode* achievementName = getChildByName(std::string("AchievementName"));
    FJ_FuiNode* achievementDescription = getChildByName(std::string("AchievementDescription"));
    if (achievementDescription->isFJ_HtmlTextPanelLarge())
    {
        FJ_HtmlTextPanelLarge* panel = achievementDescription->asFJ_HtmlTextPanelLarge();
        panel->SetFixedWidth(Math::round(achievementName->getWidth() / 3.0f));
    }
    else if (achievementDescription->isFJ_HtmlTextPanel())
    {
        FJ_HtmlTextPanel* panel = achievementDescription->asFJ_HtmlTextPanel();
        panel->SetFixedWidth(Math::round(achievementName->getWidth() / 3.0f));
    }
}

void FJ_AchievementsMenu::PositionAchievementDescription()
{
    FJ_FuiNode* listContainer = getChildByName(std::string("AchievementsListContainer"));
    FJ_FuiNode* description = getChildByName(std::string("AchievementDescription"));

    int descriptionRight = static_cast<int>(listContainer->getX()
                                            + (m_achievementsList->getActiveItem()->getX()
                                               + m_achievementsList->getActiveItem()->getWidth()
                                               + description->getWidth()));
    if (descriptionRight < static_cast<int>(getStage()->getStageWidth() - static_cast<float>(getISafeZoneRight())))
    {
        fuiRect itemRect = m_achievementsList->getActiveItem()->getRect(m_achievementsList->getActiveItem());
        description->setX(
            static_cast<float>(Math::round(listContainer->getX() + m_achievementsList->getActiveItem()->getX()
                                           + m_achievementsList->getActiveItem()->getWidth() + itemRect.xMin
                                           + static_cast<float>(m_achievementsList->m_itemSpacing))));
    }
    else
    {
        description->setX(static_cast<float>(
            Math::round(listContainer->getX() + m_achievementsList->getActiveItem()->getX() - description->getWidth()
                        - 2.0f * static_cast<float>(m_achievementsList->m_itemSpacing))));
    }

    description->setY(static_cast<float>(Math::round(listContainer->getY() + m_achievementsList->getActiveItem()->getY()
                                                     + m_achievementsList->getActiveItem()->getHeight() * 0.5f
                                                     - description->getHeight() * 0.5f)));
}

void FJ_AchievementsMenu::_PositionAchievementDescription(FJ_FuiNode* node)
{
    static_cast<FJ_AchievementsMenu*>(node)->PositionAchievementDescription();
}

FJ_AchievementsMenu* FJ_AchievementsMenu::Create(fuiRenderNode* renderNode)
{
    return new FJ_AchievementsMenu(renderNode);
}

unsigned int FJ_AchievementsMenu::getType()
{
    return 26;
}

void FJ_AchievementsMenu::SetAchievementDescription(const std::wstring& description)
{
    if (m_achievementsList->getActiveItem() == NULL)
        return;

    FJ_FuiNode* descriptionNode = getChildByName(std::string("AchievementDescription"));
    if (descriptionNode->isFJ_HtmlTextPanelLarge())
        descriptionNode->asFJ_HtmlTextPanelLarge()->SetLabel(description);
    else if (descriptionNode->isFJ_HtmlTextPanel())
        descriptionNode->asFJ_HtmlTextPanel()->SetLabel(description);

    PositionAchievementDescription();
}

void FJ_AchievementsMenu::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
    {
        setX(static_cast<float>(-right));
        setY(static_cast<float>(top));
    }
}
