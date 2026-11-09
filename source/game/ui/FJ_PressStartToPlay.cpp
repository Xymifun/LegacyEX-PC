#include "game/ui/FJ_PressStartToPlay.hpp"

#include <string>

#include "game/ui/FJ_BitmapIcon.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_ListButtonAchievement.hpp"
#include "game/util/Math.hpp"

FJ_PressStartToPlay::FJ_PressStartToPlay(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    m_mainPanel = 0;
    if (getChildByName("MainPanel") != 0)
    {
        m_mainPanel = getChildByName("MainPanel")->asMovieClip();
        m_mainPanel->getChildByName("ControllerQuadrant")
            ->getChildAt(0)
            ->getChildByName("Controller1")
            ->setVisible(false);
        m_mainPanel->getChildByName("ControllerQuadrant")
            ->getChildAt(0)
            ->getChildByName("Controller2")
            ->setVisible(false);
        m_mainPanel->getChildByName("ControllerQuadrant")
            ->getChildAt(0)
            ->getChildByName("Controller3")
            ->setVisible(false);
        m_mainPanel->getChildByName("ControllerQuadrant")
            ->getChildAt(0)
            ->getChildByName("Controller4")
            ->setVisible(false);
        m_panelWidth = m_mainPanel->getChildByName("PressStartLabel")->getWidth();
        m_mainPanel->getChildByName("PressStartLabel")->asFJ_Label()->setUseHtmlText(true);
        m_mainPanel->getChildByName("PressStartLabel")->asFJ_Label()->SetLabel(L"");
        m_mainPanel->getChildByName("WordWrapLabel")->asFJ_Label()->SetLabel(L"");
        m_panelX = m_mainPanel->getChildByName("BackgroundPanel")->getChildAt(0)->getX();
        m_panelHeight = m_mainPanel->getChildByName("BackgroundPanel")->getChildAt(0)->getHeight();
    }

    m_achievementToast = 0;
    if (getChildByName("AchievementToastAnimation") != 0)
    {
        m_achievementToast = FJ_FuiNode::asMovieClip(getChildByName("AchievementToastAnimation"))
                                 ->getChildByName("AchievementToast")
                                 ->asMovieClip();
        m_achievementToastX = m_achievementToast->getChildByName("Achievement_Unlocked")->getX();
    }

    if (getChildByName("ChestRefillAnimation") != 0)
        FJ_FuiNode::asMovieClip(getChildByName("ChestRefillAnimation")->getChildAt(0))->setVisible(false);

    if (getChildByName("BETAWatermark") != 0)
        FJ_FuiNode::asMovieClip(getChildByName("BETAWatermark"))->setVisible(false);
}

FJ_PressStartToPlay* FJ_PressStartToPlay::Create(fuiRenderNode* renderNode)
{
    return new FJ_PressStartToPlay(renderNode);
}

unsigned int FJ_PressStartToPlay::getType()
{
    return 0x26;
}

void FJ_PressStartToPlay::ShowAchievementToast(const std::wstring& title, const std::wstring& description,
                                               const std::wstring& gamerPic, const std::wstring& achievementIcon)
{
    if (m_achievementToast != 0)
    {
        m_achievementToast->getChildByName("Achievement_PressButton")->asFJ_Label()->setUseHtmlText(true);
        m_achievementToast->getChildByName("Achievement_Unlocked")->asFJ_Label()->SetLabel(title);
        m_achievementToast->getChildByName("Achievement_PressButton")->asFJ_Label()->SetLabel(description);
        m_achievementToast->getChildByName("GamerPic")->asFJ_BitmapIcon()->SetTextureName(gamerPic);
        m_achievementToast->getChildByName("AchievementIcon")
            ->asFJ_ListButtonAchievement()
            ->SetTextureName(achievementIcon);

        FJ_FuiNode* unlocked = m_achievementToast->getChildByName("Achievement_Unlocked");
        FJ_FuiNode* pressButton = m_achievementToast->getChildByName("Achievement_PressButton");
        FJ_FuiNode* panel = m_achievementToast->getChildByName("AchievementPanel");
        FJ_FuiNode* icon = m_achievementToast->getChildByName("AchievementIcon");
        FJ_FuiNode* gamerPicture = m_achievementToast->getChildByName("GamerPic");

        int widest = unlocked->getWidth() > pressButton->getWidth() ? unlocked->getWidth() : pressButton->getWidth();
        panel->getChildAt(0)->setWidth((widest + m_achievementToastX * 2)
                                       - (gamerPicture->getWidth() - icon->getWidth()));
        icon->setX(m_achievementToastX
                   + ((widest + m_achievementToastX) - (gamerPicture->getX() + gamerPicture->getWidth())));
        unlocked->setX(m_achievementToast->getWidth() * 0.5f - unlocked->getWidth() * 0.5f);
        pressButton->setX(m_achievementToast->getWidth() * 0.5f - pressButton->getWidth() * 0.5f);
        m_achievementToast->setX(getStage()->getStageWidth() * 0.5f - m_achievementToast->getWidth() * 0.5f);
        gotoAndPlay("ShowToast");
    }
}

void FJ_PressStartToPlay::HideAchievementToast()
{
    if (m_achievementToast != 0)
    {
        gotoAndPlay(std::string("HideToast"));
    }
}

void FJ_PressStartToPlay::ResizePanel(bool multiLine)
{
    if (m_mainPanel != 0)
    {
        FJ_FuiNode* pressStart = m_mainPanel->getChildByName("PressStartLabel");
        FJ_FuiNode* wordWrap = m_mainPanel->getChildByName("WordWrapLabel");
        FJ_FuiNode* background = m_mainPanel->getChildByName("BackgroundPanel");
        int width = pressStart->getWidth();
        if (multiLine)
            width = wordWrap->getWidth();

        background->getChildAt(0)->setY(m_panelX);
        background->getChildAt(0)->setHeight(m_panelHeight);
        if (wordWrap->getHeight() > m_panelHeight)
        {
            background->getChildAt(0)->setHeight(wordWrap->getHeight() + 10.0f);
            background->getChildAt(0)->decY(Math::round((wordWrap->getHeight() + 10.0f - m_panelHeight) * 0.5f));
        }
        if (width != m_panelWidth)
        {
            background->getChildAt(0)->incWidth(width - m_panelWidth);
            m_mainPanel->setX(Math::round((getStage()->getStageWidth() - (width + pressStart->getX())) * 0.5f));
            m_panelWidth = width;
        }
    }
}

void FJ_PressStartToPlay::SetText(const std::wstring& text)
{
    if (m_mainPanel != 0)
    {
        m_mainPanel->getChildByName(std::string("PressStartLabel"))->asFJ_Label()->SetLabel(text);
        m_mainPanel->getChildByName(std::string("WordWrapLabel"))->asFJ_Label()->SetLabel(L"");
        ResizePanel(false);
    }
}

void FJ_PressStartToPlay::SetMultiLineText(const std::wstring& text)
{
    if (m_mainPanel != 0)
    {
        m_mainPanel->getChildByName(std::string("PressStartLabel"))->asFJ_Label()->SetLabel(L"");
        m_mainPanel->getChildByName(std::string("WordWrapLabel"))->asFJ_Label()->SetLabel(text);
        ResizePanel(true);
    }
}

void FJ_PressStartToPlay::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    FJ_FuiNode* saveIcon = getChildByName("SaveIcon");
    saveIcon->setX(getStage()->getStageWidth() - saveIcon->getWidth() - static_cast<float>(right) - 15.0f);
    saveIcon->setY(static_cast<float>(top + 15));
    if (getChildByName("PlayerName") != 0)
    {
        getChildByName("PlayerName")
            ->setX(getStage()->getStageWidth() - getChildByName("PlayerName")->getWidth() - getISafeZoneRight());
        getChildByName("PlayerName")
            ->setY(getStage()->getStageHeight() - getChildByName("PlayerName")->getHeight() - getISafeZoneBottom());
    }
}

void FJ_PressStartToPlay::ShowController(int controller, bool show)
{
    if (m_mainPanel != 0)
    {
        m_mainPanel->getChildByName("ControllerQuadrant")
            ->getChildAt(0)
            ->getChildByName("Controller1")
            ->setVisible(false);
        m_mainPanel->getChildByName("ControllerQuadrant")
            ->getChildAt(0)
            ->getChildByName("Controller2")
            ->setVisible(false);
        m_mainPanel->getChildByName("ControllerQuadrant")
            ->getChildAt(0)
            ->getChildByName("Controller3")
            ->setVisible(false);
        m_mainPanel->getChildByName("ControllerQuadrant")
            ->getChildAt(0)
            ->getChildByName("Controller4")
            ->setVisible(false);

        if (controller == 0)
            m_mainPanel->getChildByName("ControllerQuadrant")
                ->getChildAt(0)
                ->getChildByName("Controller1")
                ->setVisible(true);
        else if (controller == 1)
            m_mainPanel->getChildByName("ControllerQuadrant")
                ->getChildAt(0)
                ->getChildByName("Controller2")
                ->setVisible(true);
        else if (controller == 2)
            m_mainPanel->getChildByName("ControllerQuadrant")
                ->getChildAt(0)
                ->getChildByName("Controller3")
                ->setVisible(true);
        else
            m_mainPanel->getChildByName("ControllerQuadrant")
                ->getChildAt(0)
                ->getChildByName("Controller4")
                ->setVisible(true);

        if (show)
            m_mainPanel->getChildByName("ControllerQuadrant")->getChildAt(0)->gotoAndStop("Stop");
        else
            m_mainPanel->getChildByName("ControllerQuadrant")->getChildAt(0)->gotoAndPlay("Play");

        m_mainPanel->getChildByName("PressStartLabel")
            ->setY(Math::round((m_mainPanel->getChildByName("BackgroundPanel")->getChildAt(0)->getHeight()
                                - m_mainPanel->getChildByName("PressStartLabel")->getHeight())
                               * 0.5f));
    }
}

void FJ_PressStartToPlay::HideChestRefillAnimation()
{
    if (getChildByName("ChestRefillAnimation") != 0)
        FJ_FuiNode::asMovieClip(getChildByName("ChestRefillAnimation")->getChildAt(0))->setVisible(false);
}

void FJ_PressStartToPlay::ShowChestRefillAnimation(bool left, int yOffset)
{
    FJ_FuiNode* animation = getChildByName("ChestRefillAnimation");
    FJ_FuiNode::asMovieClip(animation->getChildAt(0))->setVisible(true);
    if (left)
    {
        animation->setX(getStage()->getStageWidth() * 0.5f - animation->getWidth() * 0.5f);
        animation->setY(getStage()->getStageHeight() * 0.5f - animation->getHeight() * 0.5f);
    }
    else
    {
        animation->setX(getStage()->getStageWidth() - animation->getWidth() - getISafeZoneRight() - 15.0f);
        animation->setY(getISafeZoneTop() + 15 + yOffset);
    }
    FJ_FuiNode::asMovieClip(animation->getChildAt(0))->gotoAndPlay(2);
}

void FJ_PressStartToPlay::ShowBetaWatermark()
{
    FJ_FuiNode* watermark = getChildByName("BETAWatermark");
    if (watermark != 0)
    {
        FJ_FuiNode::asMovieClip(watermark)->setVisible(true);
        watermark->setX(getStage()->getStageWidth() - watermark->getWidth() - getISafeZoneRight() - 15.0f);
        watermark->setY(getISafeZoneTop() + 15);
    }
}
