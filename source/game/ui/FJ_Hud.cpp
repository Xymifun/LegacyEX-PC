#include "game/ui/FJ_Hud.hpp"

#include <cstdio>

#include "game/core/StringConversion.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_BitmapIcon.hpp"
#include "game/ui/FJ_DragonHealth.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_ExperienceBar.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Helpers.hpp"
#include "game/ui/FJ_HorseJumpBar.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LabelOutline.hpp"
#include "game/ui/FJ_Label_HUD_Splash.hpp"
#include "game/ui/FJ_Label_HUD_SplashBig.hpp"
#include "game/ui/FJ_MobEffect_Tile.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/util/Math.hpp"

static std::string s_crosshairNames[3] = {std::string("CrosshairSmall"), std::string("CrosshairNormal"),
                                          std::string("CrosshairLarge")};

static std::string s_offhandSlotNames[3] = {std::string("OffhandSlot_Small"), std::string("OffhandSlot_Normal"),
                                            std::string("OffhandSlot_Large")};

static std::string s_hudNames[3] = {std::string("HudSmall"), std::string("HudNormal"), std::string("HudLarge")};

static std::string s_xpLevelNames[3] = {std::string("HUD_XP_Level_Small"), std::string("HUD_XP_Level_Normal"),
                                        std::string("HUD_XP_Level_Large")};

static std::string s_scaleNames[3] = {std::string("ScaleSmall"), std::string("ScaleNormal"), std::string("ScaleLarge")};

static std::string s_airBubbleFrames[3] = {std::string("AirBubble_Full"), std::string("AirBubble_Pop"),
                                           std::string("AirBubble_Empty")};

static const int s_healthFrames[6] = {1, 4, 3, 2, 6, 5};
static const int s_poisonedHealthFrames[6] = {1, 8, 7, 2, 10, 9};
static const int s_witheredHealthFrames[6] = {1, 12, 11, 2, 14, 13};
static const int s_foodFrames[6] = {1, 1, 3, 5, 2, 4};
static const int s_armourFrames[3] = {1, 2, 3};
static const int s_effectSlots[28] = {-1, 12, 9,  2,  14, 11, 5,  3,  6,  13, 16, 7,  8,  1,
                                      0,  4,  10, 15, 17, 18, 19, 20, 21, 22, 23, -1, 25, 24};

void FJ_Hud::SetPlayerLevel(int level)
{
    if (level == 0)
        m_xpLevelLabel->SetLabel(L"X");
    else
        m_xpLevelLabel->SetLabel(_toString<int>(level));
    m_xpLevelLabel->setVisible(level != 0);
    m_playerLevel = level;

    fuiRect rect = m_xpLevelLabel->getRect(m_xpLevelLabel);
    int width = (int)(rect.xMin + rect.xMin + m_xpLevelLabel->getWidth());
    m_xpLevelLabel->setVisible(level != 0 ? m_expBar->isVisible() : false);
    m_xpLevelLabel->setX(Math::round(m_hud->getX() + m_hudWidth * 0.5f - width * 0.5f));
    m_xpLevelLabel->setY(Math::round(m_hud->getY() - m_xpLevelLabel->getHeight() * 0.5f
                                     + m_healthBar->getY() * m_hud->getScaleY()
                                     + m_healthBarHeight * m_hud->getScaleY() * 0.5f - 1.0f));
}

void FJ_Hud::SetActiveSlot(int slot)
{
    char name[64];
    sprintf(name, "iggy_slot_%d", slot);
    FJ_FuiNode* slotClip = m_hotBar->getChildByName(name)->asMovieClip();
    if (slotClip != NULL)
    {
        m_hotbarSelector->setX(slotClip->getX() - 12.0f);
        m_activeSlot = slot;
    }
}

void FJ_Hud::SetHealth(int health, bool highlight, bool poisoned, bool withered)
{
    for (int i = 0; i < 10; ++i)
    {
        int threshold = (i + 1) * 2;
        int state = health >= threshold ? 2 : health >= threshold - 1 ? 1 : 0;
        if (poisoned && withered)
            m_healthIcons[i]->gotoAndStop(s_witheredHealthFrames[highlight * 3 + state]);
        else if (poisoned)
            m_healthIcons[i]->gotoAndStop(s_poisonedHealthFrames[highlight * 3 + state]);
        else
            m_healthIcons[i]->gotoAndStop(s_healthFrames[highlight * 3 + state]);
    }
    m_poisoned = poisoned;
    m_withered = withered;
    m_health = health;
}

void FJ_Hud::SetFood(int food, bool poisoned)
{
    for (int row = 0; row < 2; ++row)
    {
        for (int column = 0; column < 5; ++column)
        {
            int threshold = (row * 5 + column + 1) * 2;
            int state = food >= threshold ? 2 : food >= threshold - 1 ? 1 : 0;
            m_foodIcons[row * 5 + column]->gotoAndStop(s_foodFrames[state * 2 + poisoned]);
        }
    }
    m_food = food;
    m_poisoned = poisoned;
}

void FJ_Hud::SetAir(int air)
{
    for (int i = 0; i < 10; ++i)
    {
        if (i < air)
            m_airIcons[i]->gotoAndStop(s_airBubbleFrames[0]);
        else if (air < 10 && i == air)
            m_airIcons[i]->gotoAndPlay(s_airBubbleFrames[1]);
        else
            m_airIcons[i]->gotoAndStop(s_airBubbleFrames[2]);
    }
    m_air = air;
}

void FJ_Hud::SetArmour(int armour)
{
    for (int i = 1; i <= 10; ++i)
    {
        int state = armour >= i * 2 ? 2 : armour >= i * 2 - 1 ? 1 : 0;
        m_armourIcons[i - 1]->gotoAndStop(s_armourFrames[state]);
    }
    m_armour = armour;
}

void FJ_Hud::SetRegenerationEffect(bool enabled)
{
    m_regenerationEffect = enabled;
    if (!enabled)
        m_regenerationIndex = -1;
}

void FJ_Hud::SetJukeBoxLabelPos()
{
    if (m_gameMode == 0)
    {
        getChildByName("Jukebox")->setY(Math::round(getChildByName("SelectedLabel")->getY()
                                                    - getChildByName("SelectedLabel")->getHeight() * 0.5f
                                                    - m_jukeboxHeight * 0.5f));
    }
    else if (m_viewportWidth != -1 && m_viewportHeight != -1)
    {
        getChildByName("Jukebox")->setY(m_safeZoneTop + (getStage()->getStageHeight() - m_viewportHeight));
        if (m_gamertagLabel != NULL)
            getChildByName("Jukebox")->incY(m_gamertagLabel->getHeight());
    }
    else
    {
        getChildByName("Jukebox")->setY(m_safeZoneTop);
    }
}

void FJ_Hud::SetSelectedLabel(const std::wstring& label, bool visible)
{
    if (visible)
    {
        getChildByName("SelectedLabel.Label")->asFJ_Label()->SetLabel(label);
        getChildByName("SelectedLabel")->gotoAndStop("On");
        if (m_viewportWidth != -1)
        {
            getChildByName("SelectedLabel")->setX(Math::round(m_viewportWidth * 0.5f - m_selectedLabelWidth * 0.5f));
        }
    }
    getChildByName("SelectedLabel")
        ->setY(
            Math::round(m_hud->getY() - getChildByName("SelectedLabel")->getHeight() * 0.5f - m_selectedLabelHeight));
    if (!m_horseHealthBar->isVisible() && !m_healthBar->isVisible() && !m_foodBar->isVisible())
    {
        getChildByName("SelectedLabel")->incY(Math::round(m_healthBarHeight * m_hud->getScaleY() * 2.0f));
    }
    else
    {
        if (m_armourBar->isVisible() && m_healthAbsorbBar->isVisible())
            getChildByName("SelectedLabel")->decY(m_healthAbsorbBar->getHeight() * m_hud->getScaleY());
    }
    SetJukeBoxLabelPos();
}

void FJ_Hud::SetHealthAbsorb(bool visible, int health)
{
    m_healthAbsorbBar->setVisible(visible);
    m_showHealthAbsorb = visible;
    for (int i = 0; i < 10; ++i)
    {
        if (i * 2 >= health)
            m_healthAbsorbIcons[i]->setVisible(false);
        else
            m_healthAbsorbIcons[i]->setVisible(true);
        int threshold = (i + 1) * 2;
        if (health >= threshold)
            m_healthAbsorbIcons[i]->gotoAndStop(1);
        else if (health >= threshold - 1)
            m_healthAbsorbIcons[i]->gotoAndStop(2);
        else
            m_healthAbsorbIcons[i]->gotoAndStop(0);
    }
    if (visible && health > 0)
        m_armourBar->setY(-m_healthBar->getY());
    else
        m_armourBar->setY(0.0f);
    m_healthAbsorb = health;
    m_healthAbsorbVisible = visible;
    SetSelectedLabel(L"", false);
}

void FJ_Hud::SetFoodSaturationLevel(int level)
{
    m_foodSaturationLevel = level;
}

void FJ_Hud::ShowExpBar(bool visible)
{
    m_expBar->setVisible(visible);
    m_xpLevelLabel->setVisible(m_playerLevel != 0 ? visible : false);
}

void FJ_Hud::SetRidingHorse(bool riding, bool showJump, int health)
{
    if (riding)
    {
        for (int i = 0; i < 30; ++i)
        {
            if (i * 2 >= health)
                m_horseHealthIcons[i]->setVisible(false);
            else
                m_horseHealthIcons[i]->setVisible(true);
            if (m_horseHealthIcons[i]->isVisible())
            {
                int threshold = (i + 1) * 2;
                int state = health >= threshold ? 2 : health >= threshold - 1 ? 1 : 0;
                m_horseHealthIcons[i]->gotoAndStop(s_healthFrames[state]);
            }
        }
    }
    m_foodBar->setVisible(!riding && m_healthBar->isVisible());
    m_showFood = m_foodBar->isVisible();
    ShowExpBar((!riding || !showJump) && m_healthBar->isVisible() && !m_aliveDeadBar->isVisible());
    m_horseJumpBar->setVisible(riding && showJump);
    m_showHorseJump = showJump;
    m_horseMaxHealth = health;
    m_ridingHorse = riding;
}

void FJ_Hud::SetHorseHealth(int health, bool highlight)
{
    if (!m_ridingHorse)
        return;

    for (int i = 0; i < 30; ++i)
    {
        if (health > m_horseMaxHealth)
        {
            if (i * 2 >= health)
                m_horseHealthIcons[i]->setVisible(false);
            else
                m_horseHealthIcons[i]->setVisible(true);
        }
        if (m_horseHealthIcons[i]->isVisible())
        {
            int state = health >= (i + 1) * 2 ? 2 : health >= (i + 1) * 2 - 1 ? 1 : 0;
            m_horseHealthIcons[i]->gotoAndStop(s_healthFrames[highlight * 3 + state]);
        }
    }
    if (health > m_horseMaxHealth)
        m_horseMaxHealth = health;
    m_horseHealth = health;
}

void FJ_Hud::SetExpBarProgress(float progress)
{
    m_expBar->setProgress(progress);
    m_expBarProgress = progress;
}

void FJ_Hud::SetHorseJumpBarProgress(float progress)
{
    if (m_ridingHorse && m_showHorseJump)
    {
        m_horseJumpBar->setProgress(progress);
        m_horseJumpBarProgress = progress;
    }
}

void FJ_Hud::ShowQuickSelect(bool visible)
{
    m_hotBar->setVisible(visible);
    getChildByName("SelectedLabel")->setVisible(visible);
    m_offhandSlot->setVisible(m_offhandSlotVisible && visible);
    if (m_aliveDeadBar->isVisible())
    {
        if (visible)
        {
            m_aliveDeadBar->setY(m_expBar->getY());
        }
        else
        {
            m_aliveDeadBar->setY(Math::round(m_hotBar->getY() + m_hotBar->getHeight() / m_hud->getScaleY()
                                             - m_aliveDeadBar->getHeight() / m_hud->getScaleY()));
        }
    }
    m_showQuickSelect = visible;
}

void FJ_Hud::ShowCrosshair(bool visible)
{
    m_crosshair->setVisible(visible);
    m_showCrosshair = visible;
}

void FJ_Hud::UpdateBossBarPositions()
{
    for (int i = 0; i < m_numBossBars; ++i)
    {
        if (m_bossBars[i] == NULL)
            continue;

        if (m_viewportWidth != -1 && m_viewportHeight != -1)
        {
            m_bossBars[i]->setX(Math::round(m_viewportWidth / 2 - m_bossBars[i]->getWidth() * 0.5f));
            m_bossBars[i]->setY((m_safeZoneTop + (getStage()->getStageHeight() - m_viewportHeight))
                                + m_bossBars[i]->getHeight() * i);
        }
        else
        {
            m_bossBars[i]->setX(Math::round(getStage()->getStageWidth() * 0.5f - m_bossBars[i]->getWidth() * 0.5f));
            m_bossBars[i]->setY(m_safeZoneTop + m_bossBars[i]->getHeight() * i);
        }
    }
}

void FJ_Hud::SetGamertag(const std::wstring& gamertag)
{
    if (m_gamertagLabel == NULL)
        return;

    m_gamertag = gamertag;
    m_gamertagLabel->SetLabel(gamertag);
    m_gamertagLabel->setX(m_viewportWidth - m_gamertagLabel->getWidth() - m_safeZoneRight);
    m_gamertagLabel->setY(m_safeZoneTop + (getStage()->getStageHeight() - m_viewportHeight));
    if (!gamertag.empty())
        UpdateBossBarPositions();
}

void FJ_Hud::PositionGameModeComponents()
{
    if (m_gameMode != 2)
        return;

    SetJukeBoxLabelPos();
    int bottom = m_safeZoneBottom;
    int width = m_viewportWidth;
    if (m_tooltipsEnabled)
        bottom += m_tooltipHeight;
    if (width == -1)
        width = (int)getStage()->getStageWidth();

    int top = (int)(getStage()->getStageHeight() / 5.0f + m_safeZoneTop);
    if (m_viewportWidth != -1 && m_viewportHeight != -1)
    {
        getStage()->getStageHeight();
        top = Math::round(getStage()->getStageHeight() - m_viewportHeight + m_viewportHeight / 5.0f + m_safeZoneTop);
    }

    m_raceProgress->setY(getStage()->getStageHeight() - bottom
                         - Math::round(m_raceBarHeight * m_raceBar->getScaleX() * 1.5f));
    m_pickup->setX(width - m_safeZoneRight - m_pickup->getWidth());
    m_pickup->setY(getStage()->getStageHeight() - bottom - m_pickup->getHeight());

    m_timeIcon->setX(width - m_safeZoneRight - m_timeIcon->getWidth());
    m_speedIcon->setX(width - m_safeZoneRight - m_speedIcon->getWidth());
    m_scoreIcon->setX(width - m_safeZoneRight - m_scoreIcon->getWidth());
    m_timeIcon->setY(top);
    m_speedIcon->setY(Math::round(m_timeIcon->getY() + m_timeIcon->getHeight() * 1.25f));
    m_scoreIcon->setY(Math::round(m_speedIcon->getY() + m_speedIcon->getHeight() * 1.25f));

    m_timeLabel->setX(Math::round(m_timeIcon->getX() - m_timeLabelWidth - m_timeIcon->getWidth() * 0.25f));
    m_speedLabel->setX(Math::round(m_timeIcon->getX() - m_speedLabelWidth - m_timeIcon->getWidth() * 0.25f));
    m_scoreLabel->setX(Math::round(m_timeIcon->getX() - m_speedLabelWidth - m_timeIcon->getWidth() * 0.25f));
    m_timeLabel->setY(m_timeIcon->getY() + Math::round((m_timeIcon->getHeight() - m_timeLabel->getHeight()) * 0.5f));
    m_speedLabel->setY(m_speedIcon->getY()
                       + Math::round((m_speedIcon->getHeight() - m_speedLabel->getHeight()) * 0.5f));
    m_scoreLabel->setY(m_scoreIcon->getY()
                       + Math::round((m_scoreIcon->getHeight() - m_scoreLabel->getHeight()) * 0.5f));

    m_gold->setX(m_safeZoneLeft);
    m_silver->setX(m_safeZoneLeft);
    m_bronze->setX(m_safeZoneLeft);
    m_gold->setY(top);
    m_silver->setY(Math::round(m_gold->getY() + m_gold->getHeight() * 1.25f));
    m_bronze->setY(Math::round(m_silver->getY() + m_silver->getHeight() * 1.25f));

    m_positionOne->setX(m_safeZoneLeft + m_gold->getWidth());
    m_positionTwo->setX(m_safeZoneLeft + m_silver->getWidth());
    m_positionThree->setX(m_safeZoneLeft + m_bronze->getWidth());
    m_positionOne->setY(m_gold->getY() + Math::round((m_gold->getHeight() - m_positionOne->getHeight()) * 0.5f));
    m_positionTwo->setY(m_silver->getY() + Math::round((m_silver->getHeight() - m_positionTwo->getHeight()) * 0.5f));
    m_positionThree->setY(m_bronze->getY()
                          + Math::round((m_bronze->getHeight() - m_positionThree->getHeight()) * 0.5f));

    int wrongWayX = Math::round((width - m_wrongWayLabelWidth) * 0.5f);
    float wrongWayY = getChildByName("Jukebox")->getY() + m_jukeboxHeight;
    m_wrongWayLabel->setX(wrongWayX);
    m_wrongWayLabel->setY(wrongWayY);

    m_roundNumber->setX(m_raceProgress->getX() + m_raceProgress->getWidth()
                        + Math::round(m_raceBarHeight * m_raceBar->getScaleX() * 0.5f));
    m_roundNumber->setY(
        m_raceProgress->getY()
        + Math::round(m_raceBarHeight * m_raceBar->getScaleY() * 0.5f - m_roundNumber->getHeight() * 0.5f - 1.0f));
}

void FJ_Hud::PositionIggySplash()
{
    if (m_viewportWidth != -1 && m_viewportHeight != -1)
    {
        m_gameModeSplash->setX(Math::round(m_viewportWidth * 0.5f - m_gameModeSplash->getWidth() * 0.5f));
        m_gameModeSplash->setY(
            m_safeZoneTop
            + Math::round(getStage()->getStageHeight() - m_viewportHeight + Math::round(m_viewportHeight / 3.0f)));
    }
    else
    {
        m_gameModeSplash->setX(Math::round(getStage()->getStageWidth() * 0.5f - m_gameModeSplash->getWidth() * 0.5f));
        m_gameModeSplash->setY(m_safeZoneTop + Math::round(getStage()->getStageHeight() * 0.25f));
    }
}

void FJ_Hud::RepositionHud(int width, int height)
{
    m_viewportWidth = width;
    m_viewportHeight = height;
    m_hud->setX(Math::round(width * 0.5f - m_hudWidth * 0.5f));
    m_raceProgress->setX(Math::round(width * 0.5f - m_raceProgress->getWidth() * 0.5f));
    getChildByName("Jukebox")->setX(Math::round((width - m_jukeboxWidth) * 0.5f));
    getChildByName("SelectedLabel")->setX(Math::round(m_viewportWidth * 0.5f - m_selectedLabelWidth * 0.5f));

    FJ_FuiNode* crosshair = m_crosshair->getChildAt(0)->getChildByName("Crosshair")->asMovieClip();
    m_crosshair->setX(Math::round(width / 2 - m_crosshair->getWidth() * 0.5f));
    m_crosshair->setY(Math::round(getStage()->getStageHeight() - crosshair->getHeight() * 0.5f - height * 0.5f));

    m_armourBarIcons->setX(m_safeZoneLeft > 0 ? (float)m_safeZoneLeft : m_armourBarIcons->getWidth());
    m_armourBarIcons->setY(
        Math::round(getStage()->getStageHeight() - m_armourBarIcons->getHeight() * 0.5f - height * 0.5f));

    if (m_gamertagLabel != NULL)
        SetGamertag(m_gamertag);
    PositionGameModeComponents();
    PositionIggySplash();
    UpdateBossBarPositions();
}

void FJ_Hud::SetAlpha(float alpha)
{
    m_hud->setAlpha(alpha);
    getChildByName("SelectedLabel")->setAlpha(alpha);
    m_crosshair->setAlpha(alpha);
    m_armourBarIcons->setAlpha(alpha);
    if (m_gamertagLabel != NULL)
        m_gamertagLabel->setAlpha(alpha);
    for (int i = 0; i < m_numBossBars; ++i)
    {
        if (m_bossBars[i] != NULL)
            m_bossBars[i]->setAlpha(alpha);
    }
    for (int i = 0; i < m_numStatusEffects; ++i)
    {
        if (m_statusEffectTiles[i] != NULL)
            m_statusEffectTiles[i]->setAlpha(alpha);
    }
    m_xpLevelLabel->setAlpha(alpha);
    m_armourBarIcons->setAlpha(alpha);
    m_raceProgress->setAlpha(alpha);
    m_gold->setAlpha(alpha);
    m_silver->setAlpha(alpha);
    m_bronze->setAlpha(alpha);
    m_positionOne->setAlpha(alpha);
    m_positionTwo->setAlpha(alpha);
    m_positionThree->setAlpha(alpha);
    m_roundNumber->setAlpha(alpha);
    m_timeLabel->setAlpha(alpha);
    m_timeIcon->setAlpha(alpha);
    m_speedLabel->setAlpha(alpha);
    m_speedIcon->setAlpha(alpha);
    m_scoreLabel->setAlpha(alpha);
    m_scoreIcon->setAlpha(alpha);
    m_pickup->setAlpha(alpha);
    m_wrongWayLabel->setAlpha(alpha);
    m_offhandSlot->setAlpha(alpha);
    m_alpha = alpha;
    m_alphaSet = true;
}

void FJ_Hud::InitDamageIndicators()
{
    FJ_FuiNode* indicators = m_crosshair->getChildAt(0)->getChildByName("DamageIndicators");
    for (int i = 0; i < 8; ++i)
    {
        char name[64];
        sprintf(name, "Damage_0%d", i);
        m_damageIndicators[i] = indicators->getChildByName(name);
        m_damageIndicatorTicks[i] = 0.0f;
        m_damageIndicators[i]->setAlpha(0.0f);
    }
}

int FJ_Hud::GetHotBarX()
{
    if (m_hud != NULL && m_hotBar != NULL)
        return (int)(m_hud->getX() + m_hotBar->getX() * m_hud->getScaleX());
    return 0;
}

int FJ_Hud::GetHotBarWidth()
{
    if (m_hud != NULL && m_hotBar != NULL)
        return (int)(m_hotBar->getWidth() * m_hud->getScaleX());
    return 0;
}

int FJ_Hud::GetHotBarY()
{
    if (m_hud != NULL && m_hotBar != NULL)
        return (int)(m_hud->getY() + m_hotBar->getY() * m_hud->getScaleY());
    return 0;
}

void FJ_Hud::SetupOffhandSlot()
{
    m_offhandSlot->setVisible(m_offhandSlotVisible && m_hotBar->isVisible());
    float gap = Math::round(m_offhandSlot->getWidth() * 0.25f);
    if (m_rightHanded)
        m_offhandSlot->setX(GetHotBarX() - m_offhandSlot->getWidth() - gap);
    else
        m_offhandSlot->setX(GetHotBarX() + GetHotBarWidth() + gap);
    m_offhandSlot->setY(GetHotBarY());
}

void FJ_Hud::ShowPulser(bool visible)
{
    m_gameModeSplash->setVisible(visible);
}

void FJ_Hud::ShowAnimatedLogoText(FJ_Event* event)
{
    m_animatedLogoTextShown = true;
    int offset = 6;
    if (m_viewportWidth != -1 && m_viewportHeight != -1)
        offset = m_viewportWidth == 640 || m_viewportHeight == 360 ? 7 : 8;
    m_animatedSplashText->setY(
        m_animatedSplash->getY() + m_animatedSplash->getChildByName("TextPanel")->getY() * m_animatedSplash->getScaleY()
        - m_animatedSplash->getChildByName("TextPanel")->getHeight() * m_animatedSplash->getScaleY() * 0.5f
        - m_animatedSplashText->getHeight() * 0.5f + offset * m_animatedSplash->getScaleY());
    removeEventListener(eFJEventType_ShowAnimatedLogoText, this, _ShowAnimatedLogoText);
}

void FJ_Hud::_ShowAnimatedLogoText(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Hud*>(node)->ShowAnimatedLogoText(event);
}

void FJ_Hud::ShowAnimatedSplash(const std::wstring& label, bool visible)
{
    m_animatedSplash->setVisible(visible);
    m_animatedSplashText->setVisible(visible);
    m_animatedSplashVisible = visible;
    m_animatedSplashLabel = label;
    if (visible)
    {
        m_animatedSplashFrames = 5;
        ShowPulser(false);
        addEventListener(eFJEventType_ShowAnimatedLogoText, this, _ShowAnimatedLogoText, false);
        m_animatedSplash->gotoAndStop(1);
        m_animatedSplash->getChildByName("TextPanel")->getChildAt(0)->setScaleX(1.0f);
        m_animatedSplash->getChildByName("TextPanel")->getChildAt(0)->setScaleY(1.0f);
        if (m_viewportWidth != -1 && m_viewportHeight != -1)
        {
            m_animatedSplash->setX(Math::round(m_viewportWidth * 0.5f));
            m_animatedSplash->setY((getChildByName("Jukebox")->getY() + getChildByName("Jukebox")->getHeight())
                                   + m_animatedSplashHeight * 0.5f);
            m_animatedSplashText->setX(Math::round((m_viewportWidth - m_animatedSplashTextWidth) * 0.5f));
            FJ_FuiNode* panel = m_animatedSplash->getChildByName("TextPanel")->getChildAt(0);
            panel->setHeight(
                Math::round(m_animatedSplash->getChildByName("TextPanel")->getChildAt(0)->getHeight() * 1.2f));
        }
        else
        {
            m_animatedSplash->setX(Math::round(getStage()->getStageWidth() * 0.5f));
            m_animatedSplash->setY((getChildByName("Jukebox")->getY() + getChildByName("Jukebox")->getHeight())
                                   + m_animatedSplashHeight * 0.5f);
        }
        m_animatedSplashText->SetLabel(label);
        m_animatedSplashText->setAlpha(0.0f);

        int padding = (int)(m_animatedSplash->getChildByName("TextPanel")->getChildAt(0)->getWidth() * 0.25f);
        m_animatedSplash->getChildByName("TextPanel")
            ->getChildAt(0)
            ->setWidth((float)Math::round(m_animatedSplashText->getWidth() / m_animatedSplash->getScaleX())
                       + (float)padding);
        m_animatedSplash->gotoAndPlay(2);
    }
    else
    {
        m_animatedSplash->gotoAndStop(1);
        m_animatedSplashText->SetLabel(label);
    }
}

void FJ_Hud::UpdateOffsets(FJ_Event* event)
{
    m_regenerationIndex = m_regenerationEffect ? m_tickCount % 25 : -1;
    for (int i = 0; i < 10; ++i)
    {
        float healthOffset = 0.0f;
        float absorbOffset = 0.0f;
        float foodOffset = 0.0f;
        if (m_health <= 4)
            healthOffset = FJ_Helpers::RetInstance()->nextInt(2) * (m_healthShakeOffset + 1);
        if (m_regenerationIndex == i)
        {
            healthOffset = m_healthShakeOffset * -2 - 2;
        }
        else if (m_healthAbsorbVisible)
        {
            if (m_regenerationIndex == i + 10)
                absorbOffset = m_healthShakeOffset * -2 - 2;
            if (m_healthAbsorbIcons[i]->isVisible() && absorbOffset != m_healthAbsorbIcons[i]->getY())
                m_healthAbsorbIcons[i]->setY(absorbOffset);
        }
        if (healthOffset != m_healthIcons[i]->getY())
            m_healthIcons[i]->setY(healthOffset);

        if (m_foodSaturationLevel <= 0 && m_tickCount % (m_food * 3 + 1) == 0)
            foodOffset = (float)(FJ_Helpers::RetInstance()->nextInt(3) - 1) * (float)(m_healthShakeOffset + 1);
        if (foodOffset != m_foodIcons[i]->getY())
            m_foodIcons[i]->setY(foodOffset);
    }
    m_tickCount = m_tickCount < 0x7fffffff ? m_tickCount + 1 : 0;

    for (int i = 0; i < 8; ++i)
    {
        if (m_damageIndicatorTicks[i] > 0.0f)
        {
            m_damageIndicators[i]->setAlpha(m_damageIndicatorTicks[i] / 30.0f);
            m_damageIndicatorTicks[i] -= 1.0f;
        }
        else
        {
            m_damageIndicators[i]->setAlpha(0.0f);
        }
    }

    if (m_animatedLogoTextShown)
    {
        m_animatedSplashText->incAlpha(0.1f);
        if (m_animatedSplashText->getAlpha() >= 1.0f)
        {
            m_animatedSplashText->setAlpha(1.0f);
            m_animatedLogoTextShown = false;
        }
    }

    if (!m_animatedSplashVisible && m_animatedSplash->currentFrame() != 1)
        ShowAnimatedSplash(L"", m_animatedSplashVisible);
    if (m_animatedSplashVisible && m_animatedSplash->currentFrame() == 1)
    {
        if (m_animatedSplashFrames <= 0)
            ShowAnimatedSplash(m_animatedSplashLabel, m_animatedSplashVisible);
        else
            --m_animatedSplashFrames;
    }
}

void FJ_Hud::_UpdateOffsets(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Hud*>(node)->UpdateOffsets(event);
}

void FJ_Hud::InitHud(FJ_Event* event)
{
    m_loadedHudSize = m_hudSize;
    setVisible(true);
    if (m_resetOnInit)
    {
        m_regenerationEffect = false;
        m_ridingHorse = false;
        m_showHorseJump = true;
        m_poisoned = false;
        m_withered = false;
        m_healthAbsorbVisible = false;
        m_tickCount = 0;
        m_foodSaturationLevel = 0;
        m_regenerationIndex = -1;
        m_activeSlot = 0;
        m_healthShakeOffset = 1;
        m_horseMaxHealth = 0;
        m_playerLevel = 0;
        m_numBossBars = 0;
        m_numStatusEffects = 0;
        for (int i = 0; i < 5; ++i)
            m_bossBars[i] = NULL;
        m_expBarProgress = 0.0f;
        m_horseJumpBarProgress = 0.0f;
        for (int i = 0; i < 27; ++i)
            m_statusEffectTiles[i] = NULL;
    }

    m_armourBarIcons = getChildByName("ArmourBarIcons")->asMovieClip();
    m_crosshair = getChildByName(s_crosshairNames[m_hudSize])->asMovieClip();
    m_hud = getChildByName(s_hudNames[m_hudSize])->asMovieClip();
    m_hud->setX(Math::round(m_hud->getX()));
    m_healthBar = m_hud->getChildByName("FJ_HealthBar")->asMovieClip();
    m_healthAbsorbBar = m_hud->getChildByName("FJ_HealthAbsorbBar")->asMovieClip();
    m_horseHealthBar = m_hud->getChildByName("FJ_HorseHealthBar")->asMovieClip();
    m_foodBar = m_hud->getChildByName("FJ_FoodBar")->asMovieClip();
    m_airBar = m_hud->getChildByName("FJ_AirBar")->asMovieClip();
    m_armourBar = m_hud->getChildByName("FJ_ArmourBar")->asMovieClip();
    m_expBar = m_hud->getChildByName("ExpBar")->asFJ_ExperienceBar();
    m_horseJumpBar = m_hud->getChildByName("HorseJumpBar")->asFJ_HorseJumpBar();
    m_aliveDeadBar = m_hud->getChildByName("AliveDeadBar")->asMovieClip();
    m_xpLevelLabel = getChildByName(s_xpLevelNames[m_hudSize])->asFJ_LabelOutline();
    m_hotBar = m_hud->getChildByName("HotBar")->asMovieClip();
    m_hotbarSelector = m_hotBar->getChildByName("HotbarSelector")->asMovieClip();
    m_offhandSlot = getChildByName(s_offhandSlotNames[m_hudSize])->asMovieClip();
    m_healthBarHeight = (int)m_healthBar->getHeight();
    m_hudWidth = (int)m_hud->getWidth();

    char name[64];
    for (int i = 0; i < 10; ++i)
    {
        sprintf(name, "FJ_Health_0%d", i);
        m_healthIcons[i] = m_healthBar->getChildByName(name);
        m_healthIcons[i]->gotoAndStop(3);
        sprintf(name, "FJ_FoodBar_0%d", i);
        m_foodIcons[i] = m_foodBar->getChildByName(name);
        m_foodIcons[i]->gotoAndStop(2);
        sprintf(name, "FJ_AirBubble_0%d", i);
        m_airIcons[i] = m_airBar->getChildByName(name);
        m_airIcons[i]->gotoAndStop(s_airBubbleFrames[0]);
        sprintf(name, "FJ_Armour_0%d", i);
        m_armourIcons[i] = m_armourBar->getChildByName(name);
        m_armourIcons[i]->gotoAndStop(1);
        sprintf(name, "FJ_Health_0%d", i);
        m_healthAbsorbIcons[i] = m_healthAbsorbBar->getChildByName(name);
        m_healthAbsorbIcons[i]->gotoAndStop(1);
    }

    m_aliveDeadBarWidth = (int)(m_aliveDeadBar->getWidth() / m_aliveDeadBar->getScaleX());
    m_aliveIconWidth = (int)m_aliveDeadBar->getChildByName("Alive00")->getWidth();
    m_aliveIconSpacing = Math::round((float)(m_aliveDeadBarWidth - m_aliveIconWidth * m_maxAliveIcons)
                                     / ((float)m_maxAliveIcons - 1.0f));
    for (int i = 0; i < m_maxAliveIcons; ++i)
    {
        sprintf(name, "Alive0%d", i);
        m_aliveIcons[i] = m_aliveDeadBar->getChildByName(name);
        m_aliveIcons[i]->gotoAndStop(m_isFullHd ? 18 : 10);
        m_aliveIcons[i]->setVisible(false);
    }
    if (m_isFullHd)
    {
        m_smallAliveIconWidth = (int)m_aliveDeadBar->getChildByName("SmallAlive00")->getWidth();
        m_smallAliveIconSpacing = Math::round(
            (float)(m_aliveDeadBarWidth - m_smallAliveIconWidth * m_maxSmallAliveIcons)
            / ((float)m_maxSmallAliveIcons - 1.0f));
        for (int i = 0; i < m_maxSmallAliveIcons; ++i)
        {
            sprintf(name, "SmallAlive%02d", i);
            m_smallAliveIcons[i] = m_aliveDeadBar->getChildByName(name);
            m_smallAliveIcons[i]->gotoAndStop(18);
            m_smallAliveIcons[i]->setVisible(false);
        }
    }
    for (int i = 0; i < m_maxGameModePlayers; ++i)
    {
        sprintf(name, "Player_%d", i);
        m_racePlayers[i] = m_raceBar->getChildByName(name);
        m_racePlayers[i]->gotoAndStop(m_isFullHd ? 18 : 10);
        m_racePlayers[i]->setVisible(false);
        if (i > 0)
            m_racePlayers[i]->getChildByName("CircleOutline")->setVisible(false);
    }
    for (int i = 0; i < 30; ++i)
    {
        sprintf(name, "FJ_Health_%02d", i);
        m_horseHealthIcons[i] = m_horseHealthBar->getChildByName(name);
        m_horseHealthIcons[i]->gotoAndStop(3);
    }

    if (!m_resetOnInit)
    {
        m_hud->setVisible(m_showHud);
        m_healthBar->setVisible(m_showHealth);
        m_horseHealthBar->setVisible(m_showHorseHealth);
        m_healthAbsorbBar->setVisible(m_showHealthAbsorb);
        m_foodBar->setVisible(m_showFood);
        m_airBar->setVisible(m_showAirBar);
        m_armourBar->setVisible(m_showArmour);
        m_expBar->setVisible(m_showExpBar);
        m_aliveDeadBar->setVisible(m_showGameModePlayers);
        m_xpLevelLabel->setVisible(m_showExpBar);
        m_armourBarIcons->setVisible(m_showArmorIcons);
        SetPlayerLevel(m_playerLevel);
        SetActiveSlot(m_activeSlot);
        SetHealth(m_health, false, m_poisoned, m_withered);
        SetFood(m_food, m_poisoned);
        SetAir(m_air);
        SetArmour(m_armour);
        SetRegenerationEffect(m_regenerationEffect);
        SetHealthAbsorb(m_healthAbsorbVisible, m_healthAbsorb);
        SetFoodSaturationLevel(m_foodSaturationLevel);
        SetRidingHorse(m_ridingHorse, m_showHorseJump, m_horseMaxHealth);
        SetHorseHealth(m_horseHealth, false);
        SetExpBarProgress(m_expBarProgress);
        SetHorseJumpBarProgress(m_horseJumpBarProgress);
        ShowQuickSelect(m_showQuickSelect);
        ShowCrosshair(m_showCrosshair);
        if (m_viewportWidth != -1 && m_viewportHeight != -1)
            RepositionHud(m_viewportWidth, m_viewportHeight);
    }
    else
    {
        SetPlayerLevel(0);
        SetActiveSlot(0);
        m_aliveDeadBar->setVisible(false);
        m_showGameModePlayers = false;
        m_armourBarIcons->setVisible(false);
        m_showArmorIcons = false;
        m_airBar->setVisible(false);
        m_showAirBar = false;
        getChildByName("SelectedLabel.Label")->asFJ_Label()->setUseHtmlText(true);
        getChildByName("SelectedLabel.Label")->asFJ_Label()->SetLabel(L"");
        m_resetOnInit = false;
    }
    m_hudY = (int)m_hud->getY();

    if (m_viewportWidth != -1 && m_viewportHeight != -1)
        getChildByName("Jukebox")->setY(m_safeZoneTop + (getStage()->getStageHeight() - m_viewportHeight));
    m_hud->setY(m_hudY - m_safeZoneHudBottom);
    SetPlayerLevel(m_playerLevel);
    SetSelectedLabel(L"", false);
    if (m_alphaSet)
        SetAlpha(m_alpha);
    InitDamageIndicators();
    SetupOffhandSlot();
    m_gameModeSplash->SetLabel(L"");
    addEventListener(eFJEventType_EnterFrame, this, _UpdateOffsets, false);
    m_hudLoading = false;
}

void FJ_Hud::_InitHud(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Hud*>(node)->InitHud(event);
}

void FJ_Hud::LoadHud(int hudSize)
{
    if (m_hudSize == hudSize)
        return;

    removeEventListener(eFJEventType_EnterFrame, this, _UpdateOffsets);
    gotoAndStop(s_scaleNames[hudSize]);
    m_hudLoading = true;
    m_hudSize = hudSize;
    setVisible(false);
    if (m_hudSize == m_loadedHudSize)
    {
        setVisible(true);
        addEventListener(eFJEventType_EnterFrame, this, _UpdateOffsets, false);
        m_hudLoading = false;
    }
}

void FJ_Hud::init(FJ_Event* event)
{
    m_health = 20;
    m_food = 20;
    m_armour = 0;
    m_air = 10;
    m_viewportWidth = -1;
    m_viewportHeight = -1;
    m_gamertag = L"";
    LoadHud(1);
}

void FJ_Hud::_init(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Hud*>(node)->init(event);
}

FJ_Hud::FJ_Hud(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
    m_safeZoneTop = 0;
    m_showHud = true;
    m_showHealth = true;
    m_showHorseHealth = true;
    m_showHealthAbsorb = true;
    m_showFood = true;
    m_showAirBar = true;
    m_showArmour = true;
    m_showExpBar = true;
    m_showGameModePlayers = true;
    m_showArmorIcons = true;
    m_showQuickSelect = true;
    m_safeZoneBottom = 0;
    m_safeZoneLeft = 0;
    m_showCrosshair = true;
    m_safeZoneRight = 0;
    m_isFullHd = getStage()->getStageHeight() == 1080.0f || getStage()->getStageHeight() == 540.0f;
    if (m_isFullHd)
    {
        m_hudSize = -1;
        m_loadedHudSize = -1;
        m_maxAliveIcons = 10;
        m_maxGameModePlayers = 16;
        m_resetOnInit = true;
        m_gameMode = 0;
        m_maxSmallAliveIcons = 16;
    }
    else
    {
        m_maxAliveIcons = 8;
        m_hudSize = -1;
        m_loadedHudSize = -1;
        m_maxGameModePlayers = 8;
        m_maxSmallAliveIcons = 0;
        m_resetOnInit = true;
        m_gameMode = 0;
    }
    addEventListener(eFJEventType_InitHud, this, _InitHud, false);
    addEventListener(eFJEventType_Init, this, _init, false);

    m_jukeboxHeight = (int)getChildByName("Jukebox")->getHeight();
    m_selectedLabelHeight = (int)getChildByName("SelectedLabel")->getHeight();
    m_label1Y = (int)getChildByName("Label1")->getY();
    m_selectedLabelWidth = (int)getChildByName("SelectedLabel")->getWidth();
    m_jukeboxWidth = (int)getChildByName("Jukebox")->getWidth();
    m_animatedSplashTextWidth = (int)getChildByName("AnimatedSplashText")->getWidth();
    m_wrongWayLabelWidth = (int)getChildByName("WrongWayLabel")->getWidth();

    m_alphaSet = false;
    m_rightHanded = true;
    m_offhandSlotVisible = false;
    m_alpha = 1.0f;
    m_hudLoading = false;
    setVisible(false);
    m_tooltipsEnabled = true;
    m_animatedLogoTextShown = false;

    if (getStage()->getStageHeight() == 720.0f || getStage()->getStageHeight() == 360.0f)
    {
        m_gamertagLabel = NULL;
        m_tooltipHeight = 35;
    }
    else if (getStage()->getStageHeight() == 1080.0f || getStage()->getStageHeight() == 540.0f)
    {
        m_gamertagLabel = NULL;
        m_tooltipHeight = 50;
    }
    else if (getStage()->getStageHeight() == 480.0f || getStage()->getStageHeight() == 408.0f)
    {
        m_gamertagLabel = NULL;
        m_tooltipHeight = 32;
    }
    else
    {
        m_gamertagLabel = NULL;
        m_tooltipHeight = 30;
    }

    if (getChildByName("LabelGamertag") != NULL)
    {
        m_gamertagLabel = getChildByName("LabelGamertag")->asFJ_Label();
        m_gamertagLabel->Init(L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ!");
    }

    m_wrongWayLabel = getChildByName("WrongWayLabel")->asFJ_Label();
    m_gameModeSplash = getChildByName("GameModeSplash")->asFJ_Label();
    m_animatedSplash = getChildByName("AnimatedSplash")->asMovieClip();
    m_animatedSplashHeight = (int)m_animatedSplash->getHeight();
    m_animatedSplash->setVisible(false);
    m_animatedSplashText = getChildByName("AnimatedSplashText")->asFJ_Label();
    m_animatedSplashVisible = false;
    m_roundNumber = getChildByName("RoundNumber")->asFJ_Label();
    m_timeLabel = getChildByName("TimeLabel")->asFJ_Label();
    m_timeIcon = getChildByName("TimeIcon")->asMovieClip();
    m_speedLabel = getChildByName("SpeedLabel")->asFJ_Label();
    m_speedIcon = getChildByName("SpeedIcon")->asMovieClip();
    m_scoreLabel = getChildByName("ScoreLabel")->asFJ_Label();
    m_scoreIcon = getChildByName("ScoreIcon")->asMovieClip();
    m_raceProgress = getChildByName("RaceProgress")->asMovieClip();
    m_raceBar = m_raceProgress->getChildByName("RaceBar")->asMovieClip();
    m_glideHealthBar = m_raceBar->getChildByName("GlideHealthBar")->asMovieClip();
    m_pickup = getChildByName("Pickup")->asMovieClip();
    m_scoreIconX = (int)m_scoreIcon->getX();
    m_timeLabelWidth = (int)m_timeLabel->getWidth();
    m_speedLabelWidth = (int)m_speedLabel->getWidth();
    m_raceBarHeight = (int)m_raceBar->getChildByName("Bar")->getHeight();
    m_raceBarWidth = (int)m_raceBar->getChildByName("Bar")->getWidth();
    m_scoreIconValue = 0;
    m_previousScoreIconValue = -1;
    m_gold = getChildByName("Gold")->asMovieClip();
    m_silver = getChildByName("Silver")->asMovieClip();
    m_bronze = getChildByName("Bronze")->asMovieClip();
    m_positionOne = getChildByName("PositionOne")->asFJ_Label();
    m_positionTwo = getChildByName("PositionTwo")->asFJ_Label();
    m_positionThree = getChildByName("PositionThree")->asFJ_Label();

    m_gameModeSplash->Init(L"");
    if (m_wrongWayLabel->isFJ_Label_HUD_SplashBig())
        m_wrongWayLabel->asFJ_Label_HUD_SplashBig()->Init(L"");
    else if (m_wrongWayLabel->isFJ_Label_HUD_Splash())
        m_wrongWayLabel->asFJ_Label_HUD_Splash()->Init(L"");
    m_animatedSplashText->Init(L"");
    static_cast<FJ_Label*>(getChildByName("SelectedLabel.Label"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Jukebox"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Label1"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Label2"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Label3"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Label4"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Label5"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Label6"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Label7"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Label8"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Label9"))->Init(L"");
    static_cast<FJ_Label*>(getChildByName("Label10"))->Init(L"");
    init(NULL);
}

FJ_Hud::~FJ_Hud()
{
    removeEventListener(eFJEventType_EnterFrame, this, _UpdateOffsets);
}

FJ_Hud* FJ_Hud::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Hud(pRenderNode);
}

int FJ_Hud::GetHotBarHeight()
{
    if (m_hud != NULL && m_hotBar != NULL)
        return (int)(m_hotBar->getHeight() * m_hud->getScaleY());
    return 0;
}

unsigned int FJ_Hud::getType()
{
    return 0x59;
}

bool FJ_Hud::GetHudLoading()
{
    return m_hudLoading;
}

void FJ_Hud::SetSafeZone(int top, int bottom, int left, int right)
{
    m_safeZoneTop = top;
    m_safeZoneBottom = bottom;
    m_safeZoneLeft = left;
    m_safeZoneRight = right;
    m_safeZoneHudBottom = m_tooltipsEnabled ? bottom : bottom - m_tooltipHeight;
    if (!m_hudLoading)
    {
        if (m_viewportWidth != -1 && m_viewportHeight != -1)
            getChildByName("Jukebox")->setY(m_safeZoneTop + (getStage()->getStageHeight() - m_viewportHeight));
        m_hud->setY(m_hudY - m_safeZoneHudBottom);
        SetupOffhandSlot();
    }

    if (m_safeZoneLeft > 0)
        m_armourBarIcons->setX(m_safeZoneLeft);
    else
        m_armourBarIcons->setX(m_armourBarIcons->getWidth());
    if (m_viewportHeight == -1)
    {
        m_armourBarIcons->setY(Math::round(getStage()->getStageHeight() * 0.5f - m_armourBarIcons->getHeight() * 0.5f));
    }
    else
    {
        m_armourBarIcons->setY(
            Math::round(getStage()->getStageHeight() - m_armourBarIcons->getHeight() * 0.5f - m_viewportHeight * 0.5f));
    }

    getChildByName("Label1")->setX(m_safeZoneLeft);
    getChildByName("Label2")->setX(m_safeZoneLeft);
    getChildByName("Label3")->setX(m_safeZoneLeft);
    getChildByName("Label4")->setX(m_safeZoneLeft);
    getChildByName("Label5")->setX(m_safeZoneLeft);
    getChildByName("Label6")->setX(m_safeZoneLeft);
    getChildByName("Label7")->setX(m_safeZoneLeft);
    getChildByName("Label8")->setX(m_safeZoneLeft);
    getChildByName("Label9")->setX(m_safeZoneLeft);
    getChildByName("Label10")->setX(m_safeZoneLeft);

    getChildByName("Label1")->setY(m_label1Y - m_safeZoneHudBottom);
    getChildByName("Label1Background")->setY(getChildByName("Label1")->getY());
    getChildByName("Label2")->setY(getChildByName("Label1")->getY() - getChildByName("Label1Background")->getHeight());
    getChildByName("Label2Background")->setY(getChildByName("Label2")->getY());
    getChildByName("Label3")->setY(getChildByName("Label2")->getY() - getChildByName("Label2Background")->getHeight());
    getChildByName("Label3Background")->setY(getChildByName("Label3")->getY());
    getChildByName("Label4")->setY(getChildByName("Label3")->getY() - getChildByName("Label3Background")->getHeight());
    getChildByName("Label4Background")->setY(getChildByName("Label4")->getY());
    getChildByName("Label5")->setY(getChildByName("Label4")->getY() - getChildByName("Label4Background")->getHeight());
    getChildByName("Label5Background")->setY(getChildByName("Label5")->getY());
    getChildByName("Label6")->setY(getChildByName("Label5")->getY() - getChildByName("Label5Background")->getHeight());
    getChildByName("Label6Background")->setY(getChildByName("Label6")->getY());
    getChildByName("Label7")->setY(getChildByName("Label6")->getY() - getChildByName("Label6Background")->getHeight());
    getChildByName("Label7Background")->setY(getChildByName("Label7")->getY());
    getChildByName("Label8")->setY(getChildByName("Label7")->getY() - getChildByName("Label7Background")->getHeight());
    getChildByName("Label8Background")->setY(getChildByName("Label8")->getY());
    getChildByName("Label9")->setY(getChildByName("Label8")->getY() - getChildByName("Label8Background")->getHeight());
    getChildByName("Label9Background")->setY(getChildByName("Label9")->getY());
    getChildByName("Label10")->setY(getChildByName("Label9")->getY() - getChildByName("Label9Background")->getHeight());
    getChildByName("Label10Background")->setY(getChildByName("Label10")->getY());

    PositionGameModeComponents();
    PositionIggySplash();
}

void FJ_Hud::SetAnimatedSplashTexture(const std::wstring& texture)
{
    m_animatedSplash->getChildByName("AnimatedLogo.AnimatedLogoTexture")->asFJ_BitmapIcon()->SetTextureName(texture);
}

void FJ_Hud::SetPulserText(const std::wstring& text)
{
    if (m_gameModeSplash->GetLabel() != text)
    {
        m_gameModeSplash->SetLabel(text);
        PositionIggySplash();
    }
}

void FJ_Hud::SetTooltipsEnabled(bool enabled)
{
    m_tooltipsEnabled = enabled;
    SetSafeZone(m_safeZoneTop, m_safeZoneBottom, m_safeZoneLeft, m_safeZoneRight);
    if (!m_hudLoading)
    {
        SetSelectedLabel(L"", false);
        SetPlayerLevel(m_playerLevel);
        if (m_viewportWidth != -1 && m_viewportHeight != -1)
            RepositionHud(m_viewportWidth, m_viewportHeight);
    }
}

void FJ_Hud::ShowOffhandSlot(bool visible)
{
    m_offhandSlotVisible = visible;
    m_offhandSlot->setVisible(visible && m_hotBar->isVisible());
}

void FJ_Hud::SetIsRightHanded(bool rightHanded)
{
    m_rightHanded = rightHanded;
    SetupOffhandSlot();
}

void FJ_Hud::ShowDirectionalDamage(int direction, int ticks, float alpha)
{
    float indicatorTicks = alpha * 30.0f;
    if (direction == 8)
    {
        for (int i = 0; i < 8; ++i)
        {
            m_damageIndicators[i]->setAlpha(1.0f);
            m_damageIndicatorTicks[i] = indicatorTicks;
        }
        return;
    }
    m_damageIndicators[direction]->setAlpha(1.0f);
    m_damageIndicatorTicks[direction] = indicatorTicks;
}

void FJ_Hud::SetHudGameMode(int gameMode)
{
    m_gameMode = gameMode;
    SetJukeBoxLabelPos();
    if (m_gameMode == 2)
    {
        m_hud->setVisible(false);
        m_showHud = false;
        m_raceProgress->setVisible(true);
        m_wrongWayLabel->setVisible(true);
        m_roundNumber->setVisible(true);
        m_timeLabel->setVisible(true);
        m_timeIcon->setVisible(true);
        m_speedLabel->setVisible(true);
        m_speedIcon->setVisible(true);
        m_scoreLabel->setVisible(false);
        m_scoreIcon->setVisible(false);
        m_pickup->setVisible(false);
        m_positionOne->setVisible(false);
        m_positionTwo->setVisible(false);
        m_positionThree->setVisible(false);
        m_gold->setVisible(false);
        m_silver->setVisible(false);
        m_bronze->setVisible(false);
        if (m_wrongWayLabel->isFJ_Label_HUD_SplashBig())
            m_wrongWayLabel->asFJ_Label_HUD_SplashBig()->SetLabel(L"");
        else if (m_wrongWayLabel->isFJ_Label_HUD_Splash())
            m_wrongWayLabel->asFJ_Label_HUD_Splash()->SetLabel(L"");
        m_roundNumber->SetLabel(L"");
        m_timeLabel->SetLabel(L"");
        m_speedLabel->SetLabel(L"");
        m_scoreLabel->SetLabel(L"");
        m_positionOne->SetLabel(L"");
        m_positionTwo->SetLabel(L"");
        m_positionThree->SetLabel(L"");
        PositionGameModeComponents();

        char name[64];
        for (int i = 0; i < 3; ++i)
        {
            sprintf(name, "FJ_Health_0%d", i);
            m_glideHealthIcons[i] = m_glideHealthBar->getChildByName(name);
            m_glideHealthIcons[i]->gotoAndStop(3);
        }
    }
    else
    {
        m_hud->setVisible(true);
        m_showHud = true;
        m_wrongWayLabel->setVisible(false);
        m_roundNumber->setVisible(false);
        m_timeLabel->setVisible(false);
        m_timeIcon->setVisible(false);
        m_speedLabel->setVisible(false);
        m_speedIcon->setVisible(false);
        m_scoreLabel->setVisible(false);
        m_scoreIcon->setVisible(false);
        m_raceProgress->setVisible(false);
        m_pickup->setVisible(false);
        m_positionOne->setVisible(false);
        m_positionTwo->setVisible(false);
        m_positionThree->setVisible(false);
        m_gold->setVisible(false);
        m_silver->setVisible(false);
        m_bronze->setVisible(false);
    }
}

void FJ_Hud::SetPosition(int position, const std::wstring& label, bool visible)
{
    switch (position)
    {
    case 0:
        m_positionOne->SetLabel(label);
        m_positionOne->setVisible(visible);
        m_gold->setVisible(visible);
        break;
    case 1:
        m_positionTwo->SetLabel(label);
        m_positionTwo->setVisible(visible);
        m_silver->setVisible(visible);
        break;
    case 2:
        m_positionThree->SetLabel(label);
        m_positionThree->setVisible(visible);
        m_bronze->setVisible(visible);
        break;
    }
}

void FJ_Hud::SetSpeedometer(int speed)
{
    FJ_FuiNode* icon = m_speedIcon;
    if (speed <= 0)
    {
        icon->getChildAt(0)->gotoAndStop(1);
    }
    else
    {
        if (speed > 12)
            speed = 12;
        icon->getChildAt(0)->gotoAndStop(speed);
    }
}

void FJ_Hud::SetTimerIcon(int minutes, int seconds)
{
    FJ_FuiNode* clock = m_timeIcon->getChildAt(0)->asMovieClip();
    FJ_FuiNode* minuteHand = clock->getChildByName("minutes")->asMovieClip();
    FJ_FuiNode* secondHand = clock->getChildByName("seconds")->asMovieClip();
    int minuteFrame = (int)(minutes / 60.0f * 12.0f);
    int secondFrame = (int)(seconds / 60.0f * 12.0f);
    minuteHand->gotoAndStop(minuteFrame + 1);
    secondHand->gotoAndStop(secondFrame + 1);
}

void FJ_Hud::SetScoreIcon(int score)
{
    FJ_FuiNode* icon = m_scoreIcon->getChildAt(0);
    FJ_FuiNode* background = icon->getChildByName("background")->asMovieClip();
    FJ_FuiNode* animation = icon->getChildByName("animation")->asMovieClip();
    background->gotoAndStop(score + 1);
    m_scoreIconValue = score;
    if (score == m_previousScoreIconValue)
        return;

    switch (m_previousScoreIconValue)
    {
    case 0:
        animation->gotoAndPlay("PlayGrey");
        break;
    case 1:
        animation->gotoAndPlay("PlayGreen");
        break;
    case 2:
        animation->gotoAndPlay("PlayGold");
        break;
    case 3:
        animation->gotoAndPlay("PlayDiamond");
        break;
    }
    m_previousScoreIconValue = m_scoreIconValue;
}

void FJ_Hud::ShowScore(bool visible)
{
    m_scoreLabel->setVisible(visible);
    m_scoreIcon->setVisible(visible);
}

void FJ_Hud::ShowArmorBar(bool visible)
{
    m_armourBarIcons->setVisible(visible);
}

void FJ_Hud::ShowArmorIcons(int armour, bool visible)
{
    if (m_gameMode != 1)
        return;

    switch (armour)
    {
    case 0:
        m_armourBarIcons->getChildAt(0)
            ->getChildByName("Armor_Icons")
            ->getChildByName("Armor_Head")
            ->setVisible(visible);
        break;
    case 1:
        m_armourBarIcons->getChildAt(0)
            ->getChildByName("Armor_Icons")
            ->getChildByName("Armor_Chest")
            ->setVisible(visible);
        break;
    case 2:
        m_armourBarIcons->getChildAt(0)
            ->getChildByName("Armor_Icons")
            ->getChildByName("Armor_Legs")
            ->setVisible(visible);
        break;
    case 3:
        m_armourBarIcons->getChildAt(0)
            ->getChildByName("Armor_Icons")
            ->getChildByName("Armor_Boots")
            ->setVisible(visible);
        break;
    }
}

void FJ_Hud::SetControlAlpha(const std::string& control, float alpha)
{
    getChildByName(control)->setAlpha(alpha);
    getChildByName(control)->setVisible(alpha > 0.0f);
}

void FJ_Hud::SetWrongWayLabel(const std::wstring& label)
{
    if (m_wrongWayLabel->isFJ_Label_HUD_SplashBig())
    {
        m_wrongWayLabel->asFJ_Label_HUD_SplashBig()->SetLabel(label);
        return;
    }
    if (m_wrongWayLabel->isFJ_Label_HUD_Splash())
    {
        m_wrongWayLabel->asFJ_Label_HUD_Splash()->SetLabel(label);
        return;
    }
    m_wrongWayLabel->SetLabel(label);
}

unsigned int FJ_Hud::GetStatusEffectsYOffset()
{
    return m_statusEffectsYOffset;
}

void FJ_Hud::HideSelectedLabel()
{
    getChildByName("SelectedLabel")->gotoAndPlay("Fade");
}

void FJ_Hud::UpdateStatusEffectTile(int count, int index, int effect, bool ambient, float alpha)
{
    for (int i = count; i < 27; ++i)
    {
        if (m_statusEffectTiles[i] != NULL)
        {
            removeChild(m_statusEffectTiles[i], true);
            m_statusEffectTiles[i] = NULL;
        }
    }
    m_numStatusEffects = count;
    m_statusEffectsYOffset = 0;
    if (count == 0)
        return;

    int slot = s_effectSlots[effect];
    if (index < count && m_statusEffectTiles[index] != NULL)
    {
        m_statusEffectTiles[index]->SetupEffectType(slot + 2);
        m_statusEffectAmbient[index] = ambient;
    }
    else
    {
        FJ_MobEffect_Tile* tile = NULL;
        if (getStage()->getStageHeight() == 1080.0f || getStage()->getStageHeight() == 720.0f)
        {
            if (m_viewportWidth != -1 && m_viewportHeight != -1)
            {
                tile = static_cast<FJ_MobEffect_Tile*>(
                    FJ_Factory::createObject("fourj.MobEffect.FJ_MobEffect_Tile_Small", m_pRenderNode, true, NULL));
            }
            else
            {
                tile = static_cast<FJ_MobEffect_Tile*>(
                    FJ_Factory::createObject("fourj.MobEffect.FJ_MobEffect_Tile_Normal", m_pRenderNode, true, NULL));
            }
        }
        else if (getStage()->getStageHeight() == 480.0f || getStage()->getStageHeight() == 408.0f)
        {
            tile = static_cast<FJ_MobEffect_Tile*>(
                FJ_Factory::createObject("fourj.MobEffect.FJ_MobEffect_Tile_Small", m_pRenderNode, true, NULL));
        }
        tile->Init();
        m_statusEffectTiles[index] = tile;
        m_statusEffectAmbient[index] = ambient;
        addChild(tile);
    }
    if (m_statusEffectTiles[index] != NULL)
    {
        m_statusEffectTiles[index]->SetupEffectType(slot + 2);
        m_statusEffectTiles[index]->SetEffectAlpha(alpha);
    }

    int column = 0;
    int ambientColumn = 0;
    for (int i = 0; i < m_numStatusEffects; ++i)
    {
        FJ_MobEffect_Tile* tile = m_statusEffectTiles[i];
        if (tile == NULL)
            continue;

        float y = 0.0f;
        if (m_gamertagLabel != NULL && m_gamertagLabel->isVisible() && !m_gamertag.empty())
        {
            if (getStage()->getStageHeight() == 1080.0f)
                y = m_gamertagLabel->getHeight() * 1.25f;
            else
                y = m_gamertagLabel->getHeight() * 1.5f;
        }

        float x;
        if (m_statusEffectAmbient[i])
        {
            x = ambientColumn * m_statusEffectTiles[i]->getWidth();
            ++ambientColumn;
        }
        else
        {
            x = column * m_statusEffectTiles[i]->getWidth();
            y += m_statusEffectTiles[i]->getHeight();
            ++column;
        }

        if (m_viewportWidth != -1 && m_viewportHeight != -1)
        {
            m_statusEffectTiles[i]->setX(m_viewportWidth - m_safeZoneRight - m_statusEffectTiles[i]->getWidth() - x);
            m_statusEffectTiles[i]->setY(y + m_safeZoneTop + (getStage()->getStageHeight() - m_viewportHeight));
        }
        else
        {
            m_statusEffectTiles[i]->setX(
                Math::round(getStage()->getStageWidth() - m_safeZoneRight - m_statusEffectTiles[i]->getWidth() - x));
            m_statusEffectTiles[i]->setY(Math::round(y + m_safeZoneTop));
            int bottom = Math::round(y + m_safeZoneTop) + (int)m_statusEffectTiles[i]->getHeight();
            if (bottom > m_statusEffectsYOffset)
                m_statusEffectsYOffset = bottom;
        }
    }
}

void FJ_Hud::SetNumBossBars(int count)
{
    m_numBossBars = count;
    for (int i = count; i < 5; ++i)
    {
        if (m_bossBars[i] != NULL)
        {
            removeChild(m_bossBars[i], true);
            m_bossBars[i] = NULL;
        }
    }
}

void FJ_Hud::SetBossBar(int index, float progress, const std::wstring& name)
{
    if (m_bossBars[index] == NULL)
    {
        FJ_DragonHealth* bar = NULL;
        if (getStage()->getStageHeight() == 1080.0f || getStage()->getStageHeight() == 720.0f)
        {
            if (m_viewportWidth != -1 && m_viewportHeight != -1)
            {
                bar = static_cast<FJ_DragonHealth*>(
                    FJ_Factory::createObject("fourj.ProgressBars.FJ_DragonHealth_01", m_pRenderNode, true, NULL));
            }
            else
            {
                bar = static_cast<FJ_DragonHealth*>(
                    FJ_Factory::createObject("fourj.ProgressBars.FJ_DragonHealth_03", m_pRenderNode, true, NULL));
            }
        }
        else if (getStage()->getStageHeight() == 480.0f || getStage()->getStageHeight() == 408.0f)
        {
            bar = static_cast<FJ_DragonHealth*>(
                FJ_Factory::createObject("fourj.ProgressBars.FJ_DragonHealth_02", m_pRenderNode, true, NULL));
        }
        m_bossBars[index] = bar;
        addChild(bar);
    }
    m_bossBars[index]->setProgress(progress);
    m_bossBars[index]->SetLabel(name);
    UpdateBossBarPositions();
}

void FJ_Hud::ShowGameModePlayers(bool visible)
{
    m_aliveDeadBar->setVisible(visible);
    m_showGameModePlayers = visible;
}

void FJ_Hud::SetGameModeNumPlayers(int count, bool alignLeft)
{
    if (m_isFullHd)
    {
        if (m_gameMode == 2)
        {
            for (int i = 0; i < m_maxGameModePlayers; ++i)
            {
                if (i < count)
                    m_racePlayers[i]->setVisible(true);
                else
                    m_racePlayers[i]->setVisible(false);
            }
        }
        else
        {
            int smallSpacing = 0;
            int last = count - 1;
            int offset;
            if (count <= 10)
            {
                if (count <= 8)
                    m_aliveIconSpacing = Math::round((m_aliveDeadBarWidth - m_aliveIconWidth * 8.0f) / 7.0f);
                else
                    m_aliveIconSpacing = Math::round((float)(m_aliveDeadBarWidth - m_aliveIconWidth * count)
                                                     / (float)last);
                offset = (m_aliveDeadBarWidth - (m_aliveIconWidth * count + last * m_aliveIconSpacing)) / 2;
            }
            else
            {
                smallSpacing = (m_aliveDeadBarWidth - m_smallAliveIconWidth * count) / last;
                offset = (m_aliveDeadBarWidth - (m_smallAliveIconWidth * count + last * smallSpacing)) / 2;
            }

            for (int i = 0; i < m_maxAliveIcons; ++i)
            {
                if (count <= 10)
                {
                    if (i > last)
                    {
                        m_aliveIcons[i]->setVisible(false);
                    }
                    else
                    {
                        int stride = m_aliveIconWidth + m_aliveIconSpacing;
                        if (alignLeft)
                            m_aliveIcons[i]->setX((float)stride * (float)i);
                        else
                            m_aliveIcons[i]->setX((float)offset + (float)(stride * i));
                        m_aliveIcons[i]->setVisible(true);
                    }
                }
                else
                {
                    m_aliveIcons[i]->setVisible(false);
                }
            }
            for (int i = 0; i < m_maxSmallAliveIcons; ++i)
            {
                if (count <= 10)
                {
                    m_smallAliveIcons[i]->setVisible(false);
                }
                else
                {
                    if (alignLeft)
                        m_smallAliveIcons[i]->setX((float)(m_smallAliveIconWidth + m_smallAliveIconSpacing) * (float)i);
                    else
                        m_smallAliveIcons[i]->setX((float)offset + (float)((m_smallAliveIconWidth + smallSpacing) * i));
                    m_smallAliveIcons[i]->setVisible(i <= last);
                }
            }
        }
    }
    else
    {
        if (count > 8)
            count = 8;
        if (m_gameMode == 2)
        {
            for (int i = 0; i < m_maxGameModePlayers; ++i)
            {
                if (i < count)
                    m_racePlayers[i]->setVisible(true);
                else
                    m_racePlayers[i]->setVisible(false);
            }
        }
        else
        {
            m_aliveIconSpacing = Math::round((m_aliveDeadBarWidth - m_aliveIconWidth * 8.0f) / 7.0f);
            int last = count - 1;
            int offset = (m_aliveDeadBarWidth - (m_aliveIconWidth * count + last * m_aliveIconSpacing)) / 2;
            for (int i = 0; i < m_maxAliveIcons; ++i)
            {
                if (i <= last)
                {
                    if (alignLeft)
                        m_aliveIcons[i]->setX((m_aliveIconWidth + m_aliveIconSpacing) * i);
                    else
                        m_aliveIcons[i]->setX(offset + (m_aliveIconWidth + m_aliveIconSpacing) * i);
                }
                else
                {
                    m_aliveIcons[i]->setX(offset);
                }
                m_aliveIcons[i]->setVisible(i <= last);
            }
        }
    }
    m_numGameModePlayers = count;
}

void FJ_Hud::SetGameModePlayerState(int player, int state, bool alive)
{
    if (!m_isFullHd)
    {
        if (m_gameMode == 2)
        {
            if (player < m_maxGameModePlayers)
            {
                m_racePlayers[player]->gotoAndStop(state + 1);
                m_racePlayers[player]->setAlpha(alive ? 1.0f : 0.5f);
            }
        }
        else if (m_numGameModePlayers <= 8 && player < m_maxAliveIcons)
        {
            m_aliveIcons[player]->gotoAndStop(state + 1);
            m_aliveIcons[player]->setAlpha(alive ? 1.0f : 0.5f);
        }
    }
    else if (m_gameMode == 2)
    {
        if (player < m_maxGameModePlayers)
        {
            m_racePlayers[player]->gotoAndStop(state + 1);
            m_racePlayers[player]->setAlpha(alive ? 1.0f : 0.5f);
        }
    }
    else if (m_numGameModePlayers <= m_maxAliveIcons)
    {
        if (player < m_maxAliveIcons)
        {
            m_aliveIcons[player]->gotoAndStop(state + 1);
            m_aliveIcons[player]->setAlpha(alive ? 1.0f : 0.5f);
        }
    }
    else if (player < m_maxSmallAliveIcons)
    {
        m_smallAliveIcons[player]->gotoAndStop(state + 1);
        m_smallAliveIcons[player]->setAlpha(alive ? 1.0f : 0.5f);
    }
}

void FJ_Hud::SetPercentageMapTraversed(int player, float percentage)
{
    m_racePlayers[player]->setX(
        Math::round(percentage / 100.0f * (m_raceBarWidth - m_racePlayers[player]->getWidth())));
    m_racePlayers[player]->setY(m_raceBarHeight * 0.5f - m_racePlayers[player]->getHeight() * 0.5f);
}

void FJ_Hud::AddCheckpointAtPercentage(float percentage)
{
    FJ_FuiNode* marker = FJ_Factory::createObject("fourj.Misc.FJ_CheckpointMarker", m_pRenderNode, true, NULL);
    marker->gotoAndStop(1);
    marker->setY(Math::round(m_raceBarHeight * 0.5f) * m_raceBar->getScaleY());
    marker->setX(Math::round(percentage / 100.0f * (m_raceBarWidth - marker->getWidth()) * m_raceBar->getScaleX()));
    marker->setScaleX(m_raceBar->getScaleX());
    marker->setScaleY(m_raceBar->getScaleY());
    m_raceProgress->addChild(marker);
    m_checkpoints.push_back(marker);
}

void FJ_Hud::ClearCheckpointPositions()
{
    for (unsigned int i = 0; i < m_checkpoints.size(); ++i)
        m_raceProgress->removeChild(m_checkpoints[i], true);
    m_checkpoints.clear();
}

void FJ_Hud::AnimateCheckpoint(int index)
{
    if (static_cast<unsigned int>(index) < m_checkpoints.size())
        m_checkpoints[index]->gotoAndPlay(2);
}

void FJ_Hud::SetGlideHealth(int health)
{
    char name[64];
    for (int i = 0; i < 3; ++i)
    {
        int state = health >= i + 1 ? 2 : 0;
        sprintf(name, "FJ_Health_0%d", i);
        m_glideHealthIcons[i] = m_glideHealthBar->getChildByName(name);
        m_glideHealthIcons[i]->gotoAndStop(s_healthFrames[state]);
    }
}

void FJ_Hud::ShowHealth(bool visible)
{
    m_healthBar->setVisible(visible);
    m_showHealth = visible;
}

void FJ_Hud::ShowHorseHealth(bool visible)
{
    m_horseHealthBar->setVisible(visible);
    m_showHorseHealth = visible;
}

void FJ_Hud::ShowFood(bool visible)
{
    m_foodBar->setVisible(visible);
    m_showFood = visible;
}

void FJ_Hud::ShowAir(bool visible)
{
    m_airBar->setVisible(visible);
    m_showAir = visible;
}

void FJ_Hud::ShowArmour(bool visible)
{
    m_armourBar->setVisible(visible);
    m_showArmour = visible;
}
