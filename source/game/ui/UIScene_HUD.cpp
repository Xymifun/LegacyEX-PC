#include "game/ui/UIScene_HUD.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwchar>
#include <list>
#include <utility>
#include <vector>

#include "game/core/BossEvent.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/FoodData.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/GhostController.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModViews.hpp"
#include "platform/pc/PcNativeMod.h"
#include "platform/pc/PcNativeMods.hpp"
#include "platform/pc/PcGlide.hpp"
#include "platform/pc/PcSettings.hpp"
#endif
#include "game/core/Gui.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/L10N.hpp"
#include "game/core/LerpingBossEvent.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/Slot.hpp"
#include "game/core/System.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/FJ_Hud.hpp"
#include "game/ui/InventoryMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/GhostPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "platform/wiiu/CProfile.hpp"

UIScene_HUD::UIScene_HUD(int sceneType, void*, UILayer* layer) : UIScene(sceneType, layer)
{
    m_splitScreen = false;
    initialiseMovie();
    SetSelectedLabel(L"");
    for (int i = 0; i < 10; ++i)
        m_messageLabels[i].init(UIString(L""));
    m_selectedLabel.init(UIString(L""));
    for (int i = 0; i < 4; ++i)
        m_armorIcons[i] = m_shownArmorIcons[i] = !HideMainHUD();
    m_previousHealth = 0;
    m_air = 10;
    m_animatedSplashIcon = -1;
    m_hudSize = -1;
    m_healthVisible = !HideMainHUD();
    m_foodVisible = !HideMainHUD();
    m_airVisible = !HideMainHUD();
    m_armourVisible = !HideMainHUD();
    m_selectedLabelTicks = 0;
    m_displayNameText = L"";
    m_displayNameVisible = true;
    m_ridingHorse = true;
    m_quickSelectVisible = !HideMainHUD();
    m_lastCheckpointIds[3] = 0;
    m_wrongWayVisible = false;
    m_personalBestTime = 0;
    m_scoreIcon = 0;
    m_timedSplashTicks = 0;
    m_lastCheckpointIds[1] = 0;
    m_wrongWayTicks = 0;
    m_resultSplashShown = false;
    m_animatedSplashVisible = true;
    m_levelGenerationId = -1;
    m_displayFlag = true;
    m_hudGameMode = -1;
    m_timedSplashLocked = false;
    m_lastCheckpointIds[0] = 0;
    m_checkpointsInitialised = false;
    m_comparisonScore = 0;
    m_lastCheckpointIds[2] = 0;
    addTimer(0, 100);
}

FJ_Hud* UIScene_HUD::getFJ_Hud()
{
    if (m_movie != 0)
        return m_movie->getRootNode()->getFuiNode()->asFJ_Hud();
}

void UIScene_HUD::SetSelectedLabel(const std::wstring& label)
{
    if (!label.empty())
        m_selectedLabelTicks = 60;
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetSelectedLabel(label, true);
}

bool UIScene_HUD::HideMainHUD()
{
    return Minecraft::InMiniGame(eMiniGameId_Any, true) && Minecraft::GetInstance()->GetMiniGame()->IsHUDDisabled();
}

unsigned int UIScene_HUD::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_messageLabels[0].setupControl(this, currentNode, std::string("Label1"));
        m_controls.push_back(&m_messageLabels[0]);
    }
    {
        m_messageLabels[1].setupControl(this, currentNode, std::string("Label2"));
        m_controls.push_back(&m_messageLabels[1]);
    }
    {
        m_messageLabels[2].setupControl(this, currentNode, std::string("Label3"));
        m_controls.push_back(&m_messageLabels[2]);
    }
    {
        m_messageLabels[3].setupControl(this, currentNode, std::string("Label4"));
        m_controls.push_back(&m_messageLabels[3]);
    }
    {
        m_messageLabels[4].setupControl(this, currentNode, std::string("Label5"));
        m_controls.push_back(&m_messageLabels[4]);
    }
    {
        m_messageLabels[5].setupControl(this, currentNode, std::string("Label6"));
        m_controls.push_back(&m_messageLabels[5]);
    }
    {
        m_messageLabels[6].setupControl(this, currentNode, std::string("Label7"));
        m_controls.push_back(&m_messageLabels[6]);
    }
    {
        m_messageLabels[7].setupControl(this, currentNode, std::string("Label8"));
        m_controls.push_back(&m_messageLabels[7]);
    }
    {
        m_messageLabels[8].setupControl(this, currentNode, std::string("Label9"));
        m_controls.push_back(&m_messageLabels[8]);
    }
    {
        m_messageLabels[9].setupControl(this, currentNode, std::string("Label10"));
        m_controls.push_back(&m_messageLabels[9]);
    }
    {
        m_messageControls[0].setupControl(this, currentNode, std::string("Label1Background"));
        m_controls.push_back(&m_messageControls[0]);
    }
    {
        m_messageControls[1].setupControl(this, currentNode, std::string("Label2Background"));
        m_controls.push_back(&m_messageControls[1]);
    }
    {
        m_messageControls[2].setupControl(this, currentNode, std::string("Label3Background"));
        m_controls.push_back(&m_messageControls[2]);
    }
    {
        m_messageControls[3].setupControl(this, currentNode, std::string("Label4Background"));
        m_controls.push_back(&m_messageControls[3]);
    }
    {
        m_messageControls[4].setupControl(this, currentNode, std::string("Label5Background"));
        m_controls.push_back(&m_messageControls[4]);
    }
    {
        m_messageControls[5].setupControl(this, currentNode, std::string("Label6Background"));
        m_controls.push_back(&m_messageControls[5]);
    }
    {
        m_messageControls[6].setupControl(this, currentNode, std::string("Label7Background"));
        m_controls.push_back(&m_messageControls[6]);
    }
    {
        m_messageControls[7].setupControl(this, currentNode, std::string("Label8Background"));
        m_controls.push_back(&m_messageControls[7]);
    }
    {
        m_messageControls[8].setupControl(this, currentNode, std::string("Label9Background"));
        m_controls.push_back(&m_messageControls[8]);
    }
    {
        m_messageControls[9].setupControl(this, currentNode, std::string("Label10Background"));
        m_controls.push_back(&m_messageControls[9]);
    }
    {
        m_selectedLabel.setupControl(this, currentNode, std::string("Jukebox"));
        m_controls.push_back(&m_selectedLabel);
    }
    if (m_splitScreen)
    {
        m_displayName.setupControl(this, currentNode, std::string("LabelGamertag"));
        m_controls.push_back(&m_displayName);
    }
    {
        m_healthLabel.setupControl(this, currentNode, std::string("ScoreLabel"));
        m_controls.push_back(&m_healthLabel);
    }
    {
        m_hudComponent.setupControl(this, currentNode, std::string("ScoreIcon"));
        m_controls.push_back(&m_hudComponent);
    }
    {
        m_foodLabel.setupControl(this, currentNode, std::string("SpeedLabel"));
        m_controls.push_back(&m_foodLabel);
    }
    {
        m_armourLabel.setupControl(this, currentNode, std::string("TimeLabel"));
        m_controls.push_back(&m_armourLabel);
    }
    {
        m_positionLabels[0].setupControl(this, currentNode, std::string("PositionOne"));
        m_controls.push_back(&m_positionLabels[0]);
    }
    {
        m_positionLabels[1].setupControl(this, currentNode, std::string("PositionTwo"));
        m_controls.push_back(&m_positionLabels[1]);
    }
    {
        m_positionLabels[2].setupControl(this, currentNode, std::string("PositionThree"));
        m_controls.push_back(&m_positionLabels[2]);
    }
    {
        m_positionControls[0].setupControl(this, currentNode, std::string("Gold"));
        m_controls.push_back(&m_positionControls[0]);
    }
    {
        m_positionControls[1].setupControl(this, currentNode, std::string("Silver"));
        m_controls.push_back(&m_positionControls[1]);
    }
    {
        m_positionControls[2].setupControl(this, currentNode, std::string("Bronze"));
        m_controls.push_back(&m_positionControls[2]);
    }
    {
        m_effectLabel.setupControl(this, currentNode, std::string("WrongWayLabel"));
        m_controls.push_back(&m_effectLabel);
    }
    {
        m_splashLabel.setupControl(this, currentNode, std::string("RoundNumber"));
        m_controls.push_back(&m_splashLabel);
    }
    return 1;
}

void UIScene_HUD::ShowArmorIcons(int armour, bool visible)
{
    if (armour < 4 && m_shownArmorIcons[armour] != visible)
    {
        m_shownArmorIcons[armour] = visible;
        if (getFJ_Hud() != 0)
            getFJ_Hud()->ShowArmorIcons(armour, visible);
    }
}

bool UIScene_HUD::SORTBYTIMELEFT(MobEffectInstance* first, MobEffectInstance* second)
{
    if (first->getDuration() > second->getDuration())
        return true;
    if (first->getDuration() < second->getDuration())
        return false;
    return false;
}

void UIScene_HUD::updateEffectsDisplay()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[m_playerId];
    if (player == 0)
        return;

    std::vector<MobEffectInstance*>* effects = player->getActiveEffects();
    if (effects->empty())
    {
        getFJ_Hud()->UpdateStatusEffectTile(0, 0, 0, false, 0.0f);
    }
    else
    {
        std::sort(effects->begin(), effects->end(), SORTBYTIMELEFT);
        int slot = 0;
        for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end(); ++it)
        {
            MobEffectInstance* instance = *it;
            MobEffect* effect = instance->getEffect();
            if (!effect->hasIcon())
                continue;

            float alpha = 1.0f;
            if (instance->getDuration() < 200)
            {
                const int duration = instance->getDuration();
                const float fadeIn = Mth::clamp(static_cast<float>(instance->getDuration()) / 10.0f / 5.0f * 0.5f, 0.0f,
                                                0.5f);
                const float pulse = Mth::cos(static_cast<float>(instance->getDuration()) * 3.1415927f / 5.0f);
                const float blink = Mth::clamp(static_cast<float>(10 - duration / 20) / 10.0f * 0.25f, 0.0f, 0.25f);
                alpha = pulse * blink + fadeIn;
            }

            getFJ_Hud()->UpdateStatusEffectTile(static_cast<int>(effects->size()), slot, effect->getIcon(),
                                                effect->isBeneficial(), alpha);
            ++slot;
        }
    }

    delete effects;
}

void UIScene_HUD::SetNumBossBars(int count)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetNumBossBars(count);
}

void UIScene_HUD::SetBossBar(int index, float progress, const std::wstring& name)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetBossBar(index, progress, name);
}

void UIScene_HUD::HideSelectedLabel()
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->HideSelectedLabel();
}

void UIScene_HUD::tick()
{
    if (getFJ_Hud() != 0)
        UIScene::tick();

    if (m_movie == 0)
        return;

    if (gApp.GetGameStarted())
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        if (minecraft->mLocalPlayers[m_playerId] == 0 || minecraft->mGameModes[m_playerId] == 0)
            return;

        if (m_armorIcons[0] != m_shownArmorIcons[0])
            ShowArmorIcons(0, m_armorIcons[0]);
        if (m_armorIcons[1] != m_shownArmorIcons[1])
            ShowArmorIcons(1, m_armorIcons[1]);
        if (m_armorIcons[2] != m_shownArmorIcons[2])
            ShowArmorIcons(2, m_armorIcons[2]);
        if (m_armorIcons[3] != m_shownArmorIcons[3])
            ShowArmorIcons(3, m_armorIcons[3]);

        updateEffectsDisplay();

        int index = 0;
        boost::unordered_map<std::wstring, LerpingBossEvent*> bossEvents =
            minecraft->mGui->getBossOverlay(m_playerId)->mBossEvents;
        SetNumBossBars(bossEvents.size() > 5 ? 5 : static_cast<int>(bossEvents.size()));

        for (boost::unordered_map<std::wstring, LerpingBossEvent*>::iterator it = bossEvents.begin();
             it != bossEvents.end(); ++it)
        {
            if (index < 5)
            {
                BossEvent* bossEvent = it->second;
                std::wstring name = L10N::GetString(bossEvent->getNameId());
                if (!bossEvent->getName().empty())
                    name = bossEvent->getName();
                SetBossBar(index, bossEvent->getPercent(), name);
            }
            ++index;
        }
    }
    else
    {
        SetNumBossBars(0);
        if (m_selectedLabelTicks != 0)
        {
            m_selectedLabelTicks = 0;
            HideSelectedLabel();
        }
    }
}

void UIScene_HUD::updateSafeZone()
{
    double safeTop = 0.0;
    double safeBottom = 0.0;
    double safeLeft = 0.0;
    double safeRight = 0.0;
    switch (m_layer->getViewport())
    {
    case C4JRender::eViewportType_TopHalf:
        safeTop = getSafeZoneHalfHeight();
        safeLeft = getSafeZoneHalfWidth();
        safeRight = getSafeZoneHalfWidth();
        break;
    case C4JRender::eViewportType_BottomHalf:
        safeBottom = getSafeZoneHalfHeight();
        safeLeft = getSafeZoneHalfWidth();
        safeRight = getSafeZoneHalfWidth();
        break;
    case C4JRender::eViewportType_LeftHalf:
        safeLeft = getSafeZoneHalfWidth();
        safeTop = getSafeZoneHalfHeight();
        safeBottom = getSafeZoneHalfHeight();
        break;
    case C4JRender::eViewportType_RightHalf:
        safeRight = getSafeZoneHalfWidth();
        safeTop = getSafeZoneHalfHeight();
        safeBottom = getSafeZoneHalfHeight();
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        safeTop = getSafeZoneHalfHeight();
        safeLeft = getSafeZoneHalfWidth();
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        safeTop = getSafeZoneHalfHeight();
        safeRight = getSafeZoneHalfWidth();
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        safeBottom = getSafeZoneHalfHeight();
        safeLeft = getSafeZoneHalfWidth();
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        safeBottom = getSafeZoneHalfHeight();
        safeRight = getSafeZoneHalfWidth();
        break;
    case C4JRender::eViewportType_FullScreen:
    default:
        safeTop = getSafeZoneHalfHeight();
        safeBottom = getSafeZoneHalfHeight();
        safeLeft = getSafeZoneHalfWidth();
        safeRight = getSafeZoneHalfWidth();
        break;
    }
    setSafeZone(safeTop, safeBottom, safeLeft, safeRight);
}

std::wstring UIScene_HUD::getMoviePath()
{
    const unsigned int viewport = m_layer->getViewport();
    if (viewport == 0 || viewport > 8)
    {
        m_splitScreen = false;
        return std::wstring(L"HUD");
    }
    m_splitScreen = true;
    return std::wstring(L"HUDSplit");
}

void UIScene_HUD::customDraw(const char* name, fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[m_playerId] == NULL || minecraft->mGameModes[m_playerId] == NULL)
        return;

    int slot = -1;
    bool armorSlot = false;
    std::sscanf(name, "slot_%d", &slot);
#ifdef MINECRAFT_PC
    if (slot >= 0 && slot <= 9 && !m_splitScreen)
    {
        const float* modelView = gRenderer->MatrixGet(0);
        const float* projection = gRenderer->MatrixGet(1);
        float corners[2][2];
        const float points[2][2] = {{rect->xMin, rect->yMin}, {rect->xMax, rect->yMax}};
        for (int corner = 0; corner < 2; ++corner)
        {
            float view[4];
            for (int row = 0; row < 4; ++row)
                view[row] = modelView[row] * points[corner][0] + modelView[4 + row] * points[corner][1]
                            + modelView[12 + row];
            float clip[4];
            for (int row = 0; row < 4; ++row)
                clip[row] = projection[row] * view[0] + projection[4 + row] * view[1] + projection[8 + row] * view[2]
                            + projection[12 + row] * view[3];
            corners[corner][0] = (clip[0] / clip[3] + 1.0f) * 0.5f;
            corners[corner][1] = (1.0f - clip[1] / clip[3]) * 0.5f;
        }
        PcModViewsNoteHotbarSlot(slot, std::min(corners[0][0], corners[1][0]),
                               std::min(corners[0][1], corners[1][1]), std::max(corners[0][0], corners[1][0]),
                               std::max(corners[0][1], corners[1][1]));
    }
#endif
    if (slot == -1)
    {
        std::sscanf(name, "armor_slot_%d", &slot);
        if (slot != -1)
        {
            armorSlot = true;
        }
        else
        {
            std::sscanf(name, "pickup_slot_%d", &slot);
            if (slot == -1)
                return;
        }
    }

    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    if (armorSlot)
    {
        item = minecraft->mLocalPlayers[m_playerId]
                   ->mInventoryMenu->getSlot(InventoryMenu::ARMOR_SLOT_START + slot)
                   ->getItem();
        bool hidden = false;
        if (!item->isEmpty() && HideMainHUD())
            hidden = true;
        m_armorIcons[slot] = hidden;
    }
    else
    {
        item = minecraft->mLocalPlayers[m_playerId]
                   ->mInventoryMenu->getSlot(InventoryMenu::USE_ROW_SLOT_START + slot)
                   ->getItem();
    }

    if (item->isEmpty())
        return;

    unsigned int opacity = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_InterfaceOpacity);
    float alpha;
    if (opacity < 80)
    {
        unsigned int timer = gApp.GetOpacityTimer(m_playerId);
        if (timer == 0)
            alpha = 0.01f * (float)opacity;
        else if (timer < 10)
            alpha = (80.0f - (10.0f - (float)timer) * ((80.0f - (float)opacity) / 10.0f)) * 0.01f;
        else
            alpha = 80.0f * 0.01f;
    }
    else
    {
        alpha = 0.01f * (float)opacity;
    }

    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mPlayer;
    if (static_cast<unsigned int>(m_playerId) < 4)
        minecraft->mPlayer = minecraft->mLocalPlayers[m_playerId];
    customDrawSlotControlFui(rect, m_playerId, item, alpha, item->getItem()->isFoil(item), true, false);
    minecraft->mPlayer = player;
}

void UIScene_HUD::SetHudSize(int size)
{
    if (size != m_hudSize)
    {
        m_hudLoading = true;
        m_hudSize = size;
        if (getFJ_Hud() != 0)
            getFJ_Hud()->LoadHud(size);
    }
}

void UIScene_HUD::SetDisplayName(const std::wstring& name)
{
    if (name.compare(m_displayNameText) != 0 && m_splitScreen)
    {
        m_displayNameText.assign(name);
        if (getFJ_Hud() != 0)
            getFJ_Hud()->SetGamertag(name);
        if (m_splitScreen)
            m_displayName.setVisible(m_displayNameVisible);
    }
}

void UIScene_HUD::repositionHud()
{
    if (!m_splitScreen)
        return;

    int width = 0;
    int height = 0;
    m_layer->getRenderDimensions(width, height);
    const unsigned int viewport = m_layer->getViewport();
    switch (viewport)
    {
    case C4JRender::eViewportType_LeftHalf:
    case C4JRender::eViewportType_RightHalf:
        height = static_cast<int>(gConsoleUIController.m_screenHeight);
        break;
    case C4JRender::eViewportType_TopHalf:
    case C4JRender::eViewportType_BottomHalf:
        width = static_cast<int>(gConsoleUIController.m_screenWidth);
        break;
    }

    if (getFJ_Hud() != 0)
        getFJ_Hud()->RepositionHud(width, height);
}

void UIScene_HUD::ShowGameModePlayers(bool visible)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowGameModePlayers(visible);
}

bool UIScene_HUD::ShowHudComponent(bool wasVisible, bool visible)
{
    if (!wasVisible && visible && Minecraft::InMiniGame(eMiniGameId_Lobby, true)
        && Minecraft::GetInstance()->GetClientMasterGameMode()->IsRoundEnding())
        visible = false;
    return visible;
}

void UIScene_HUD::ShowHealth(bool visible)
{
    if (HideMainHUD())
        visible = false;
    visible = ShowHudComponent(m_healthVisible, visible);
    m_healthVisible = visible;
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowHealth(visible);
}
void UIScene_HUD::ShowFood(bool visible)
{
    if (HideMainHUD())
        visible = false;
    visible = ShowHudComponent(m_foodVisible, visible);
    m_foodVisible = visible;
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowFood(visible);
}
void UIScene_HUD::ShowArmour(bool visible)
{
    if (HideMainHUD())
        visible = false;
    visible = ShowHudComponent(m_armourVisible, visible);
    m_armourVisible = visible;
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowArmour(visible);
}
void UIScene_HUD::ShowArmorBar(bool visible)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowArmorBar(visible);
}
void UIScene_HUD::ShowQuickSelect(bool visible)
{
    if (HideMainHUD())
        visible = false;
    visible = ShowHudComponent(m_quickSelectVisible, visible);
    bool showArmorBar = false;
    if (Minecraft::InMiniGame(eMiniGameId_Battle, false))
        showArmorBar = visible;
    ShowArmorBar(showArmorBar);
    m_quickSelectVisible = visible;
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowQuickSelect(visible);
}

void UIScene_HUD::ShowExpBar(bool visible)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowExpBar(visible);
}
void UIScene_HUD::ShowHorseHealth(bool visible)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowHorseHealth(visible);
}
void UIScene_HUD::ShowAir(bool visible)
{
    if (HideMainHUD())
        visible = false;
    visible = ShowHudComponent(m_airVisible, visible);
    m_airVisible = visible;
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowAir(visible);
}
void UIScene_HUD::SetRidingHorse(bool riding, bool showJump, int health)
{
    if (HideMainHUD())
        showJump = false;
    m_ridingHorse = riding;
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetRidingHorse(riding, showJump, health);
}
void UIScene_HUD::SetRegenerationEffect(bool enabled)
{
    if (HideMainHUD())
        enabled = false;
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetRegenerationEffect(enabled);
}
void UIScene_HUD::SetHealthAbsorb(int health)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetHealthAbsorb(health > 0, health);
}
void UIScene_HUD::InitMainHUD()
{
    const bool visible = !HideMainHUD();
    ShowGameModePlayers(true);
    ShowHealth(visible);
    ShowFood(visible);
    ShowArmour(visible);
    ShowQuickSelect(visible);
    ShowExpBar(visible);
    ShowHorseHealth(visible);
    ShowAir(false);
    SetRidingHorse(false, false, 0);
    SetRegenerationEffect(false);
    SetHealthAbsorb(0);
    m_shownArmorIcons[0] = visible;
    m_armorIcons[0] = visible;
    m_shownArmorIcons[1] = visible;
    m_armorIcons[1] = visible;
    m_shownArmorIcons[2] = visible;
    m_armorIcons[2] = visible;
    m_shownArmorIcons[3] = visible;
    m_armorIcons[3] = visible;
}
void UIScene_HUD::SetTooltipsEnabled(bool enabled)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetTooltipsEnabled(enabled);
}
void UIScene_HUD::handleReload()
{
    m_hudSize = -1;
    m_air = 10;
    m_airVisible = true;
    m_armourVisible = true;
    m_foodVisible = true;
    m_selectedLabelTicks = 0;
    m_healthVisible = true;
    m_displayNameText = L"";
    m_displayFlag = true;
    m_hudGameMode = -1;
    m_quickSelectVisible = true;
    m_ridingHorse = true;
    m_animatedSplashVisible = true;
    m_cachedLabel = L"";
    m_animatedSplashText = L"";
    m_displayNameVisible = true;
    m_scoreIcon = 0;
    m_animatedSplashIcon = -1;
    m_checkpointsInitialised = false;

    if (m_splitScreen)
        m_displayName.setVisible(true);

    Minecraft* minecraft = Minecraft::GetInstance();

    SetSelectedLabel(L"");

    for (int i = 0; i < 10; ++i)
        m_messageLabels[i].init(UIString(L""));
    m_selectedLabel.init(UIString(L""));

    if (minecraft->mLocalPlayers[m_playerId] != 0 && minecraft->mLocalPlayers[m_playerId]->mViewport != 0)
        SetHudSize(GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen));
    else
        SetHudSize(GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSize));

    SetDisplayName(gProfile->GetMiiName(m_playerId));

    repositionHud();
    minecraft->mGui->clearSingleMessages();

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
        InitMainHUD();

    bool tooltipsEnabled = true;
    if (!gConsoleUIController.GetMenuDisplayed(m_playerId)
        && GameSettings::GetGameSettings(m_playerId, eGameSetting_Tooltips) == 0)
        tooltipsEnabled = false;
    SetTooltipsEnabled(tooltipsEnabled);
}

unsigned int UIScene_HUD::getPad()
{
    return static_cast<unsigned int>(m_playerId);
}
void UIScene_HUD::SetOpacity(float opacity)
{
    setOpacity(opacity);
}
void UIScene_HUD::SetVisible(bool visible)
{
    setVisible(visible);
}
void UIScene_HUD::GetHudLoading()
{
    if (m_hudLoading)
        m_hudLoading = getFJ_Hud()->GetHudLoading();
}
void UIScene_HUD::SetExpBarProgress(float progress, int)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetExpBarProgress(progress);
}
void UIScene_HUD::SetExpLevel(int level)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetPlayerLevel(level);
}
void UIScene_HUD::SetActiveSlot(int slot)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetActiveSlot(slot);
}
void UIScene_HUD::SetGlideHealth(int health)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetGlideHealth(health);
}
void UIScene_HUD::SetHealth(int health, int minimumHealth, bool highlight, bool hardcore, bool hide)
{
    if (minimumHealth > health)
        health = minimumHealth;

    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetHealth(health, highlight, hardcore, hide);
}
void UIScene_HUD::SetFood(int food, int, bool highlight)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetFood(food, highlight);
}
void UIScene_HUD::SetAir(int air, int)
{
    if (m_air != air)
    {
        m_air = air;
        if (getFJ_Hud() != 0)
            getFJ_Hud()->SetAir(air);
    }
}
void UIScene_HUD::SetArmour(int armour)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetArmour(armour);
}
void UIScene_HUD::SetRoundNumber(unsigned int round, unsigned int total, bool visible)
{
    if (visible)
        m_splashLabel.setLabel(UIString(formatwstr(L"%d/%d", round, total)), false, false);
    else
        m_splashLabel.setLabel(UIString(L""), false, false);
}
void UIScene_HUD::ShowCrosshair(bool visible)
{
#ifdef MINECRAFT_PC
    if (PcNativeModsHudHidden(PC_NATIVE_HUD_CROSSHAIR))
        visible = false;
#endif
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowCrosshair(visible);
}
void UIScene_HUD::SetFoodSaturationLevel(int level)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetFoodSaturationLevel(level);
}
void UIScene_HUD::SetHorseHealth(int health, bool highlight)
{
    if (m_ridingHorse && getFJ_Hud() != 0)
        getFJ_Hud()->SetHorseHealth(health, highlight);
}
void UIScene_HUD::SetHorseJumpBarProgress(float progress)
{
    if (m_ridingHorse && getFJ_Hud() != 0)
        getFJ_Hud()->SetHorseJumpBarProgress(progress);
}
void UIScene_HUD::ShowDisplayName(bool visible)
{
    m_displayNameVisible = visible;
    if (m_splitScreen)
        m_displayName.setVisible(visible);
}
void UIScene_HUD::SetHudGameMode(int gameMode)
{
    if (m_hudGameMode == gameMode)
        return;
    m_hudGameMode = gameMode;
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetHudGameMode(gameMode);
}
void UIScene_HUD::SetGameModePlayerState(int player, eGameModePlayerState state, bool visible)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetGameModePlayerState(player, (int)state, visible);
}
void UIScene_HUD::SetGameModeNumPlayers(int count, bool visible)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetGameModeNumPlayers(count, visible);
}
int UIScene_HUD::GetHotBarX()
{
    return getFJ_Hud() == 0 ? 0 : getFJ_Hud()->GetHotBarX();
}
int UIScene_HUD::GetHotBarY()
{
    return getFJ_Hud() == 0 ? 0 : getFJ_Hud()->GetHotBarY();
}
int UIScene_HUD::GetHotBarWidth()
{
    return getFJ_Hud() == 0 ? 0 : getFJ_Hud()->GetHotBarWidth();
}
int UIScene_HUD::GetHotBarHeight()
{
    return getFJ_Hud() == 0 ? 0 : getFJ_Hud()->GetHotBarHeight();
}
void UIScene_HUD::ShowDirectionalDamage(int direction, int ticks, float alpha)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowDirectionalDamage(direction, ticks, alpha);
}
void UIScene_HUD::SetAnimatedSplashTexture(const std::wstring& texture)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetAnimatedSplashTexture(texture);
}
void UIScene_HUD::SetAnimatedSplashIcon(int icon)
{
    std::wstring texture = L"";
    bool found;
    if (icon == 0)
    {
        texture = Minecraft::GetInstance()->GetMiniGame()->getIcon();
        found = gApp.hasArchiveFile(texture, false);
    }
    else if (icon == 2)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Glide, true)
            && Minecraft::GetInstance()->mLocalPlayers[getPad()]->GetGameType() != GameType::LOBBY)
        {
            if (Minecraft::GetInstance()->GetClientMasterGameMode()->GetMiniGame()->ArePointsActive())
            {
                texture = L"Graphics\\GameModeGraphics\\ScoreIcon.png";
                found = gApp.hasArchiveFile(texture, false);
            }
            else
            {
                texture = L"Graphics\\GameModeGraphics\\TimeIcon.png";
                found = gApp.hasArchiveFile(texture, false);
            }
        }
        else
        {
            texture = L"Graphics\\GameModeGraphics\\AwardIcons_0029.png";
            found = gApp.hasArchiveFile(texture, false);
        }
    }
    else
    {
        texture = L"Graphics\\GameModeGraphics\\AwardIcons_0029.png";
        found = gApp.hasArchiveFile(texture, false);
    }

    if (found)
    {
        arrayWithLength<unsigned char> file = gApp.getArchiveFile(texture, false);
        if (file.length != 0 && file.data != NULL)
            registerSubstitutionTexture(texture, file.data, file.length, true);
        SetAnimatedSplashTexture(texture);
    }
    else
    {
        SetAnimatedSplashTexture(L"");
    }
}
void UIScene_HUD::ShowAnimatedSplash(const std::wstring& label, bool visible, int icon)
{
    if (m_animatedSplashText != label || m_animatedSplashVisible != visible)
    {
        m_animatedSplashText = label;
        m_animatedSplashVisible = visible;
        if (icon >= 0 && m_animatedSplashIcon != icon)
        {
            SetAnimatedSplashIcon(icon);
            m_animatedSplashIcon = icon;
        }
        if (getFJ_Hud() != 0)
            getFJ_Hud()->ShowAnimatedSplash(label, visible);
    }
}
void UIScene_HUD::SetIsRightHanded(bool rightHanded)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetIsRightHanded(rightHanded);
}
void UIScene_HUD::ShowOffhandSlot(bool visible)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowOffhandSlot(visible);
}
void UIScene_HUD::SetScore(const std::wstring& score)
{
    m_healthLabel.setLabel(UIString(score), false, false);
}
void UIScene_HUD::ShowScore(bool visible)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ShowScore(visible);
}
void UIScene_HUD::SetSpeed(const std::wstring& speed)
{
    m_foodLabel.setLabel(UIString(speed), false, false);
}
void UIScene_HUD::SetTime(const std::wstring& time)
{
    m_armourLabel.setLabel(UIString(time), false, false);
}
void UIScene_HUD::SetTimerIcon(int icon, int time)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetTimerIcon(icon, time);
}
void UIScene_HUD::SetWrongWay(const std::wstring& label)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetWrongWayLabel(label);
}
void UIScene_HUD::SetPercentageMapTraversed(int position, double percentage)
{
    percentage = Mth::clamp(percentage, 0.0, 100.0);
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetPercentageMapTraversed(position, (float)percentage);
}
void UIScene_HUD::AddCheckpointAtPercentage(double percentage)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->AddCheckpointAtPercentage((float)percentage);
}
void UIScene_HUD::ClearCheckpointPositions()
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->ClearCheckpointPositions();
}
void UIScene_HUD::AnimateCheckpoint(unsigned int index)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->AnimateCheckpoint((int)index);
}
void UIScene_HUD::SetPositions(unsigned int position, const std::wstring& label, bool visible)
{
    if (position < 3 && getFJ_Hud() != 0)
        getFJ_Hud()->SetPosition((int)position, label, visible);
}
void UIScene_HUD::SetSpeedometer(unsigned int value)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetSpeedometer((int)value);
}
void UIScene_HUD::SetScoreIcon(unsigned int value)
{
    if (getFJ_Hud() != 0)
        getFJ_Hud()->SetScoreIcon((int)value);
}
unsigned int UIScene_HUD::GetStatusEffectsYOffset()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
    if (player != 0 && !player->getActiveEffects()->empty())
        return getFJ_Hud()->GetStatusEffectsYOffset();
    return 0;
}
void UIScene_HUD::renderPlayerHealth()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    const unsigned int pad = getPad();
    ShowHealth(true);

    SetRegenerationEffect(minecraft->mLocalPlayers[pad]->hasEffect(MobEffects::REGENERATION));

    int blinkCycleTime = minecraft->mLocalPlayers[pad]->mInvulnerableTime;
    int blinkActiveTime = minecraft->mLocalPlayers[pad]->mInvulnerableTime;
    const int health = Mth::ceil(minecraft->mLocalPlayers[pad]->getHealth());
    const bool poisoned = minecraft->mLocalPlayers[pad]->hasEffect(MobEffects::POISON);
    const bool withered = minecraft->mLocalPlayers[pad]->hasEffect(MobEffects::WITHER);
    const int absorption = (int)minecraft->mLocalPlayers[pad]->getAbsorptionAmount();
    const int armour = minecraft->mLocalPlayers[pad]->getArmorValue();
    SetHealth(health, m_previousHealth, blinkActiveTime > 9 && (blinkCycleTime / 3) % 2 == 1, poisoned || withered,
              withered);
    m_previousHealth = health;
    SetHealthAbsorb(absorption);
    if (armour > 0)
    {
        ShowArmour(true);
        SetArmour(armour);
    }
    else
    {
        ShowArmour(false);
    }

    boost::shared_ptr<Entity> vehicle = minecraft->mLocalPlayers[pad]->getVehicle();
    if (vehicle == 0 || (vehicle->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
    {
        SetRidingHorse(false, false, 0);
        ShowFood(true);
        ShowHorseHealth(false);
        FoodData* food = minecraft->mLocalPlayers[pad]->getFoodData();
        const int foodLevel = food->getFoodLevel();
        const int lastFoodLevel = food->getLastFoodLevel();
        const bool hungry = minecraft->mLocalPlayers[pad]->hasEffect(MobEffects::HUNGER);
        const int saturation = (int)minecraft->mLocalPlayers[pad]->getFoodData()->getSaturationLevel();
        SetFood(foodLevel, lastFoodLevel, hungry);
        SetFoodSaturationLevel(saturation);
    }

    boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[pad];
    if (!player->isUnderLiquid(Material::WATER) && !player->isUnderLiquid(Material::LAVA)
        && player->getAirSupply() >= player->getMaxAirSupply())
    {
        ShowAir(false);
        return;
    }

    ShowAir(true);
    const int air = (int)std::ceil((float)(minecraft->mLocalPlayers[pad]->getAirSupply() - 2) * 10.0f / 300.0f);
    const int popping = (int)std::ceil((float)minecraft->mLocalPlayers[pad]->getAirSupply() * 10.0f / 300.0f) - air;
    SetAir(air, popping);
}
void UIScene_HUD::renderVehicleHealth()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer>& player = minecraft->mLocalPlayers[getPad()];
    boost::shared_ptr<Entity> vehicle = player->getVehicle();
    if (vehicle == 0 || (vehicle->GetType() & EntityType::LIVING_ENTITY) != EntityType::LIVING_ENTITY)
    {
        SetRidingHorse(false, false, 0);
        ShowHorseHealth(false);
    }
    else
    {
        boost::shared_ptr<LivingEntity> livingVehicle = boost::static_pointer_cast<LivingEntity>(vehicle);
        const int health = (int)std::ceil(livingVehicle->getHealth());
        SetRidingHorse(true, player->isRidingJumpable(), (int)livingVehicle->getMaxHealth());
        SetHorseHealth(health, false);
        ShowHorseHealth(true);
    }
}
std::wstring UIScene_HUD::formatTime(long long time, bool hundredths)
{
    unsigned int seconds = (unsigned int)(time / 1000) % 60;
    int minutes = (int)((time / 60000) % 60);
    std::wstring text = L"";
    if (hundredths)
    {
        int centiseconds = (int)((time % 1000) / 10);
        if (minutes != 0)
            text = formatwstr(L"%d:%.2d.%.2d", minutes, seconds, centiseconds);
        else
            text = formatwstr(L"%d.%.2d", seconds, centiseconds);
    }
    else
    {
        int milliseconds = (int)(time % 1000);
        if (minutes != 0)
            text = formatwstr(L"%d:%.2d.%.3d", minutes, seconds, milliseconds);
        else
            text = formatwstr(L"%d.%.3d", seconds, milliseconds);
    }
    return text;
}
void UIScene_HUD::updateGameModePlayers()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    const int pad = static_cast<int>(getPad());
    const int networkPlayerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    int slotStates[8];
    for (int i = 0; i < 8; ++i)
        slotStates[i] = 8;

    boost::shared_ptr<MultiplayerLocalPlayer>& localPlayer = minecraft->mLocalPlayers[pad];
    bool showNumPlayers = true;
    if (localPlayer != 0 && localPlayer->GetGameType() != GameType::LOBBY)
        showNumPlayers = false;

    bool glideLayout = false;
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && !showNumPlayers)
        glideLayout = true;

    bool showGhost = false;
    if (minecraft->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
    {
        GhostController* ghostController = Minecraft::GetInstance()->GetGhostController();
        if (ghostController != 0 && !showNumPlayers && ghostController->mLoaded[pad] != 0)
        {
#ifdef MINECRAFT_PC
            if (GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_ShowGlideGhost) != 0
                && !PcSettingsGlideGhostHidden())
                showGhost = true;
#else
            if (GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_ShowGlideGhost) != 0)
                showGhost = true;
#endif
        }
    }

    for (int index = 0; index < 8; ++index)
    {
        boost::shared_ptr<Player> player = Minecraft::GetInstance()->GetPlayerByPlayerIndex(index);
        if (player != 0 && !player->isSpectator())
        {
            bool alive = player->isAlive();
            if (Minecraft::InMiniGame(eMiniGameId_Any, true) && !alive)
            {
                if (Minecraft::GetInstance()->GetMiniGame()->GetLivesPerRound() != 0)
                    alive = !player->isSpectator();
            }

            if (!alive)
                slotStates[player->mPlayerIndex] = 9;
            else
                slotStates[player->mPlayerIndex] = player->mPlayerIndex;
        }
    }

    ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
    SetGameModeNumPlayers(showGhost ? networkPlayerCount + 1 : networkPlayerCount, showNumPlayers);

    const int localIndex = localPlayer != 0 ? localPlayer->mPlayerIndex : 0;
    int displaySlot = 0;

    if (glideLayout)
    {
        bool visible = true;
        if (!gameMode->IsPlayerReady(static_cast<unsigned int>(localIndex)) && gameMode->IsInLobbyState())
            visible = false;
        SetGameModePlayerState(0, static_cast<eGameModePlayerState>(slotStates[localIndex]), visible);
        m_gameModePlayerSlots[localIndex] = 0;
        displaySlot = 1;
    }

    for (int group = 0; group < 4; ++group)
    {
        for (int pair = 0; pair < 2; ++pair)
        {
            const int playerIndex = group * 2 + pair;
            bool visible = true;
            if (!gameMode->IsPlayerReady(static_cast<unsigned int>(playerIndex)) && gameMode->IsInLobbyState())
                visible = false;

            boost::shared_ptr<Player> player = Minecraft::GetInstance()->GetPlayerByPlayerIndex(playerIndex);
            if (glideLayout && player != 0 && player->isSpectator())
                visible = false;

            const int state = slotStates[playerIndex];
            if (state != 9 && state != 8 && (localIndex != playerIndex || !glideLayout))
            {
                SetGameModePlayerState(displaySlot, static_cast<eGameModePlayerState>(state), visible);
                m_gameModePlayerSlots[playerIndex] = displaySlot;
                ++displaySlot;
            }
        }
    }

    for (int group = 0; group < 4; ++group)
    {
        for (int pair = 0; pair < 2; ++pair)
        {
            const int playerIndex = group * 2 + pair;
            bool visible = true;
            if (!gameMode->IsPlayerReady(static_cast<unsigned int>(playerIndex)) && gameMode->IsInLobbyState())
                visible = false;

            boost::shared_ptr<Player> player = Minecraft::GetInstance()->GetPlayerByPlayerIndex(playerIndex);
            if (glideLayout && player != 0 && player->isSpectator())
                visible = false;

            const int state = slotStates[playerIndex];
            if ((state == 9 || state == 8) && (localIndex != playerIndex || !glideLayout))
            {
                SetGameModePlayerState(displaySlot, static_cast<eGameModePlayerState>(state), visible);
                m_gameModePlayerSlots[playerIndex] = displaySlot;
                ++displaySlot;
            }
        }
    }

    if (!showGhost)
    {
        m_ghostPlayerSlot = -1;
        return;
    }

    m_ghostPlayerSlot = networkPlayerCount;
    SetGameModePlayerState(networkPlayerCount, static_cast<eGameModePlayerState>(9), true);
}
void UIScene_HUD::ClearGlideHUD(int player)
{
    ShowScore(false);
    m_lastCheckpointIds[player] = 0;
    m_levelGenerationId = 0xffffffff;
    ShowExpBar(false);
    SetHudGameMode(1);
    ShowGameModePlayers(true);
    updateGameModePlayers();
    SetRoundNumber(0, 0, false);
    if (m_checkpointsInitialised)
    {
        ClearCheckpointPositions();
        m_lastCheckpointIds[0] = 0;
        m_checkpointsInitialised = false;
        m_lastCheckpointIds[2] = 0;
        m_lastCheckpointIds[1] = 0;
        m_lastCheckpointIds[3] = 0;
    }
}
void UIScene_HUD::showTimedSplash(const std::wstring& label, unsigned int duration, int icon)
{
    if (!m_timedSplashLocked)
    {
        m_timedSplashTicks = duration;
        m_timedSplashText.assign(label);
        m_timedSplashIcon = icon;
    }
}
void UIScene_HUD::updateFrameTick()
{
    const int pad = static_cast<int>(getPad());
    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<MultiplayerLocalPlayer>& player = minecraft->mLocalPlayers[pad];
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[pad];
    if (player->mViewport == 0)
        SetHudSize(static_cast<int>(GameSettings::GetGameSettings(pad, eGameSetting_HUDSize)));
    else
        SetHudSize(static_cast<int>(GameSettings::GetGameSettings(pad, eGameSetting_HUDSizeSplitscreen)));

    SetDisplayName(gProfile->GetMiiName(pad));

    bool tooltipsEnabled = true;
    if (gConsoleUIController.GetMenuDisplayed(pad) == 0
        && GameSettings::GetGameSettings(pad, eGameSetting_Tooltips) == 0)
        tooltipsEnabled = false;
    SetTooltipsEnabled(tooltipsEnabled);

    SetActiveSlot(player->mInventory->m_selected);

    not_null_ptr<ItemInstance> offhandItem = player->getOffhandItem();
    const HumanoidArm* oppositeArm = player->getMainArm()->getOpposite();
    if (!offhandItem->isEmpty())
    {
        SetIsRightHanded(oppositeArm == HumanoidArm::LEFT);
        ShowOffhandSlot(true);
    }
    else
    {
        ShowOffhandSlot(false);
    }

    if (player->isSpectator())
    {
        ShowGameModePlayers(true);
        ShowQuickSelect(false);
        bool showCrosshair = false;
        const MiniGameDef* miniGame = minecraft->GetMiniGame();
        if (miniGame->HasSpectatorParticipation())
        {
            if (!player->IsSpectatingOtherPlayer())
                showCrosshair = player->GetThirdPersonView(false) == 0;
        }
        ShowCrosshair(showCrosshair);
        ShowHealth(false);
        ShowFood(false);
        ShowHorseHealth(false);
        ShowAir(false);
        ShowArmour(false);
        ShowExpBar(false);
        SetRidingHorse(false, false, 0);
        SetRegenerationEffect(false);
        SetHealthAbsorb(0);
    }
    else
    {
        ShowCrosshair(player->GetThirdPersonView(false) == 0);
        const bool canHurt = gameMode->canHurtPlayer();
        ShowQuickSelect(true);
        if (canHurt)
        {
            renderPlayerHealth();
            renderVehicleHealth();
        }
        else
        {
            ShowHealth(false);
            ShowFood(false);
            ShowAir(false);
            ShowArmour(false);
            ShowExpBar(false);
            SetHealthAbsorb(0);
            renderVehicleHealth();
        }
    }

    if (player->isRidingJumpable())
    {
        SetHorseJumpBarProgress(static_cast<float>(player->getJumpRidingScale()));
    }
    else
    {
        if (gameMode->hasExperience())
        {
            if (gApp.GetGameHostOption(eGameHostOption_MiniGame) == 0)
                ShowExpBar(true);
            SetExpBarProgress(player->mExperienceProgress, player->getXpNeededForNextLevel());
            SetExpLevel(player->mExperienceLevel);
        }
        else
        {
            ShowExpBar(false);
            SetExpLevel(0);
        }
    }

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        if (player->mDamageIndicatorAllDirections)
        {
            const double delay = Minecraft::GetInstance()->GetMiniGame()->GetDamageIndicatorDelay();
            ShowDirectionalDamage(8, static_cast<int>(player->getDamageTaken()), static_cast<float>(delay));
        }
        else
        {
            for (int segment = 0; segment < 8; ++segment)
            {
                if (player->mDamageIndicatorSegments[segment])
                {
                    const double delay = Minecraft::GetInstance()->GetMiniGame()->GetDamageIndicatorDelay();
                    ShowDirectionalDamage(segment, static_cast<int>(player->getDamageTaken()),
                                          static_cast<float>(delay));
                }
            }
        }
    }

    if (m_selectedLabelTicks != 0)
    {
        --m_selectedLabelTicks;
        if (m_selectedLabelTicks < 0x14)
        {
            HideSelectedLabel();
            m_selectedLabelTicks = 0;
        }
    }

    unsigned int hudOpacity = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_InterfaceOpacity);
    if (hudOpacity < 0x50)
    {
        if (gConsoleUIController.GetMenuDisplayed(pad) != 0 && hudOpacity < 0xf)
            hudOpacity = 0xf;
        const unsigned int opacityTimer = gApp.GetOpacityTimer(pad);
        if (opacityTimer == 0)
        {
            SetOpacity(static_cast<float>(hudOpacity) * 0.01f);
        }
        else if (opacityTimer < 10)
        {
            const float step = (80.0f - static_cast<float>(hudOpacity)) / 10.0f;
            SetOpacity(0.01f * (80.0f - (10.0f - static_cast<float>(opacityTimer)) * step));
        }
        else
        {
            SetOpacity(0.01f * 80.0f);
        }
    }
    else
    {
        gConsoleUIController.GetMenuDisplayed(pad);
        SetOpacity(static_cast<float>(hudOpacity) * 0.01f);
    }

    if (gApp.GetGameStarted() == 0 || gConsoleUIController.GetMenuDisplayed(pad) != 0
        || gConsoleUIController.IsSceneInStack(pad, EUIScene_ConnectingProgress) != 0
        || gApp.GetXuiAction(pad) == eXuiAction_AutoSave
        || GameSettings::GetGameSettings(pad, eGameSetting_DisplayHUD) == 0 || player == 0)
    {
        SetVisible(false);
    }
    else
    {
        SetVisible(true);
    }

    if (gApp.GetGameHostOption(eGameHostOption_MiniGame) == 0)
    {
        m_personalBestTime = 0;
        SetHudGameMode(0);
        ShowGameModePlayers(false);
        ShowAnimatedSplash(L"", false, -1);
    }
    else
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = player;
        if (!Minecraft::InMiniGame(eMiniGameId_Glide, true) || player->GetGameType() == GameType::LOBBY)
        {
            ClearGlideHUD(pad);
        }
        else
        {
            ClientScoreboard* scoreboard = localPlayer->GetLevel()->GetClientScoreboard();
            ClientMasterGameMode* clientGameMode = minecraft->GetClientMasterGameMode();
            const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
            boost::shared_ptr<Player> glidePlayer = localPlayer;
            if (localPlayer->isSpectator()
                && eTYPE_DERIVED_FROM(EntityType::PLAYER, localPlayer->GetCamera()->GetType()))
                glidePlayer = boost::static_pointer_cast<Player>(localPlayer->GetCamera());

            SetHudGameMode(2);
            SetRoundNumber(static_cast<unsigned int>(scoreboard->mCurrentRound + 1),
                           Minecraft::GetInstance()->GetMiniGame()->GetRoundCount(), true);

            double speed = 0.0;
            if (glidePlayer->isFallFlying())
            {
                speed = std::sqrt(glidePlayer->mXd * glidePlayer->mXd + glidePlayer->mYd * glidePlayer->mYd
                                  + glidePlayer->mZd * glidePlayer->mZd);
            }
            speed *= 20.0;

            SetSpeed(replaceAll(L10N::GetString(0xc6565405), L"%d", formatwstr(L"%.2f", speed)));
            SetSpeedometer(static_cast<int>((static_cast<float>(speed) / 50.0f) * 12.0f + 0.5f));

            if (miniGame->ArePointsActive())
            {
                const int points = scoreboard->GetPoints(glidePlayer->getSUID(), -1);
                const int lastPoints = scoreboard->GetLastPointsAdded(glidePlayer->getSUID(), -1);
                SetScore(formatwstr(L"%d", points));
                ShowScore(true);
                if (lastPoints == 7)
                {
                    SetScoreIcon(3);
                    m_scoreIcon = 3;
                }
                else if (lastPoints == 5)
                {
                    SetScoreIcon(2);
                    m_scoreIcon = 2;
                }
                else
                {
                    const unsigned int icon = lastPoints == 3 ? 1U : 0U;
                    SetScoreIcon(icon);
                    m_scoreIcon = static_cast<int>(icon);
                }
            }
            else
            {
                ShowScore(false);
            }

            SetGlideHealth(Mth::ceil(glidePlayer->getHealth()));

            unsigned int elapsedMs = 0;
            bool showTimer = true;
            if (scoreboard->GetEndZoneEntered(glidePlayer->getSUID(), -1))
            {
                elapsedMs = scoreboard->GetLifeSpan(glidePlayer->getSUID(), -1);
                SetTime(formatTime(static_cast<long long>(elapsedMs), true));
            }
            else if (!clientGameMode->IsRoundEnding())
            {
                if (!clientGameMode->IsWaitingToStart() && !clientGameMode->IsRoundStarting())
                {
                    const unsigned long long now = System::processTimeInMilliSecs();
                    const unsigned long long start = clientGameMode->GetLocalRoundTimeStart();
                    elapsedMs = static_cast<unsigned int>(now - start);
                    SetTime(formatTime(static_cast<long long>(elapsedMs), true));
                }
                else
                {
                    SetTime(formatTime(0, true));
                }
            }
            else
            {
                showTimer = false;
                SetTime(L10N::GetString(IDS_DNF));
                SetTimerIcon(0, 0);
            }
            if (showTimer)
                SetTimerIcon(static_cast<int>((elapsedMs / 60000) % 60), static_cast<int>((elapsedMs / 1000) % 60));

            std::list<std::pair<std::wstring, int> > sortedPlayers;
            if (!miniGame->ArePointsActive())
            {
                if (!clientGameMode->IsWaitingToStart() && !clientGameMode->IsRoundStarting())
                    scoreboard->GetPlayersSortedByPosition(sortedPlayers, -1);
                else
                    scoreboard->GetPlayersSortedByPosition(sortedPlayers, scoreboard->mCurrentRound - 1);
            }
            else
            {
                scoreboard->GetPlayersSortedByPoints(sortedPlayers, -1);
            }

            unsigned int positionSlot = 0;
            if (player->mViewport == 0)
            {
                if (sortedPlayers.size() < 2)
                {
#ifdef MINECRAFT_PC
                    const bool timeTrial =
                        Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial)
                        != 0;
                    const bool practice = timeTrial && PcGlidePractice(pad) && clientGameMode->IsRoundPlaying();
                    if (practice)
                    {
                        SetPositions(0, L"Practice", true);
                        positionSlot = 1;
                    }
                    if (timeTrial && !practice && !PcSettingsGlideGhostHidden())
#else
                    if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial)
                        != 0)
#endif
                    {
                        GhostController* ghostController = Minecraft::GetInstance()->GetGhostController();
                        const bool endZoneEntered = scoreboard->GetEndZoneEntered(localPlayer->getSUID(), -1);
                        if (ghostController != 0)
                        {
                            GhostController::GhostRecord* ghostRecord = ghostController->mLoaded[pad];
                            if (!miniGame->ArePointsActive())
                            {
                                boost::shared_ptr<GhostPlayer> ghostPlayer = ghostController->getGhostPlayer(pad);
                                if ((ghostPlayer != 0 || endZoneEntered) && ghostRecord != 0
                                    && (ghostController->mPlaybackActive[pad]
                                        || ghostController->HasFinished(ghostRecord)))
                                {
                                    int comparisonDelta;
                                    if (sortedPlayers.empty() || !endZoneEntered)
                                    {
                                        comparisonDelta = static_cast<int>(elapsedMs - ghostRecord->mElapsed);
                                        m_comparisonScore = static_cast<unsigned int>(ghostRecord->mElapsed);
                                    }
                                    else
                                    {
                                        comparisonDelta = sortedPlayers.front().second
                                                          - static_cast<int>(m_comparisonScore);
                                    }

                                    bool ghostAhead = false;
                                    if (ghostPlayer != 0)
                                    {
                                        const double ghostProgress = ghostPlayer->getProgress();
                                        ghostAhead =
                                            Minecraft::GetInstance()->GetClientMasterGameMode()->GetPlayerMapProgress(
                                                localPlayer->mXuid)
                                            < ghostProgress;
                                    }
                                    if (comparisonDelta > 0)
                                        ghostAhead = true;

                                    std::wstring positionLabel = L"";
                                    if (ghostAhead)
                                    {
                                        positionLabel = formatwstr(L"%ls : %ls",
                                                                   formatTime(m_comparisonScore, true).c_str(),
                                                                   L10N::GetString(IDS_MG03_GHOST));
                                    }
                                    else if (comparisonDelta < 0)
                                    {
                                        positionLabel = formatwstr(L"-%ls : %ls",
                                                                   formatTime(-comparisonDelta, true).c_str(),
                                                                   player->getDisplayName().c_str());
                                    }
                                    else
                                    {
                                        positionLabel = formatwstr(L"+%ls : %ls",
                                                                   formatTime(comparisonDelta, true).c_str(),
                                                                   player->getDisplayName().c_str());
                                    }
                                    SetPositions(0, positionLabel, true);
                                    if (ghostAhead)
                                    {
                                        if (comparisonDelta < 0)
                                        {
                                            positionLabel = formatwstr(L"-%ls : %ls",
                                                                       formatTime(-comparisonDelta, true).c_str(),
                                                                       player->getDisplayName().c_str());
                                        }
                                        else
                                        {
                                            positionLabel = formatwstr(L"+%ls : %ls",
                                                                       formatTime(comparisonDelta, true).c_str(),
                                                                       player->getDisplayName().c_str());
                                        }
                                    }
                                    else
                                    {
                                        positionLabel = formatwstr(L"%ls : %ls",
                                                                   formatTime(m_comparisonScore, true).c_str(),
                                                                   L10N::GetString(IDS_MG03_GHOST));
                                    }
                                    SetPositions(1, positionLabel, true);
                                    positionSlot = 2;
                                }
                            }
                            else
                            {
                                unsigned int bestScore;
                                if (!endZoneEntered)
                                {
                                    bestScore = ghostController->GetBestScore(pad);
                                    m_comparisonScore = bestScore;
                                }
                                else
                                {
                                    bestScore = m_comparisonScore;
                                }
                                if (bestScore != 0)
                                {
                                    unsigned int playerScore = 0;
                                    if (!sortedPlayers.empty())
                                        playerScore = static_cast<unsigned int>(sortedPlayers.front().second);
                                    if (playerScore > bestScore)
                                    {
                                        std::wstring positionLabel = formatwstr(L"%d : %ls", playerScore,
                                                                                player->getDisplayName().c_str());
                                        SetPositions(0, positionLabel, true);
                                        positionLabel = formatwstr(L"%d : %ls", m_comparisonScore,
                                                                   L10N::GetString(IDS_MG03_GHOST));
                                        SetPositions(1, positionLabel, true);
                                    }
                                    else
                                    {
                                        std::wstring positionLabel = formatwstr(L"%d : %ls", bestScore,
                                                                                L10N::GetString(IDS_MG03_GHOST));
                                        SetPositions(0, positionLabel, true);
                                        positionLabel = formatwstr(L"%d : %ls", playerScore,
                                                                   player->getDisplayName().c_str());
                                        SetPositions(1, positionLabel, true);
                                    }
                                    positionSlot = 2;
                                }
                            }
                        }
                    }
                }
                else
                {
                    const int leaderValue = sortedPlayers.front().second;
                    for (std::list<std::pair<std::wstring, int> >::iterator it = sortedPlayers.begin();
                         it != sortedPlayers.end() && positionSlot < 3; ++it, ++positionSlot)
                    {
                        std::wstring label = L"";
                        if (!miniGame->ArePointsActive())
                        {
                            if (it->second == 0)
                                label = formatwstr(L"%ls", it->first.c_str());
                            else if (positionSlot == 0)
                                label = formatwstr(L"%ls : %ls", formatTime(it->second, true).c_str(),
                                                   it->first.c_str());
                            else
                                label = formatwstr(L"+%ls : %ls", formatTime(it->second - leaderValue, true).c_str(),
                                                   it->first.c_str());
                        }
                        else
                        {
                            label = formatwstr(L"%d : %ls", it->second, it->first.c_str());
                        }
                        SetPositions(positionSlot, label, true);
                    }
                }
            }

            for (; positionSlot < 3; ++positionSlot)
                SetPositions(positionSlot, L"", false);

            LevelGenerationOptions* levelGenerationOptions = gApp.getLevelGenerationOptions();
            if (levelGenerationOptions == 0)
            {
                ClearGlideHUD(pad);
                m_levelGenerationId = static_cast<unsigned int>(-1);
            }
            else if (!m_checkpointsInitialised)
            {
                const double totalDistance = clientGameMode->getTotalDistance();
                std::vector<double>* checkpointDistances = clientGameMode->getCheckpointDistances();
                double accumulated = 0.0;
                for (std::vector<double>::iterator it = checkpointDistances->begin(); it != checkpointDistances->end();
                     ++it)
                {
                    AddCheckpointAtPercentage(((accumulated + *it) / totalDistance) * 100.0);
                    accumulated += *it;
                }
                m_checkpointsInitialised = true;
                m_levelGenerationId = levelGenerationOptions->getUniqueId();
            }
            else if (levelGenerationOptions->getUniqueId() != m_levelGenerationId)
            {
                ClearGlideHUD(pad);
            }

            if (clientGameMode->IsInActiveRoundState())
            {
                const int checkpointId = scoreboard->GetLastBankedCheckpointID(glidePlayer->getSUID(), -1);
                if (checkpointId > m_lastCheckpointIds[pad])
                {
                    m_lastCheckpointIds[pad] = checkpointId;
                    AnimateCheckpoint(
                        static_cast<unsigned int>(clientGameMode->convertCheckpointIDToSpawnableID(checkpointId)));
                }
                else if (checkpointId == 0)
                {
                    m_lastCheckpointIds[pad] = 0;
                }
#ifdef MINECRAFT_PC
                else if (checkpointId < m_lastCheckpointIds[pad])
                {
                    m_lastCheckpointIds[pad] = checkpointId;
                }
#endif
            }
            updateGameModePlayers();

            if (!gConsoleUIController.IsPauseMenuDisplayed(pad) && !gConsoleUIController.GetMenuDisplayed(pad))
            {
                boost::shared_ptr<MultiplayerLocalPlayer> traversingPlayer = minecraft->mLocalPlayers[pad];
                if (traversingPlayer != 0 && !traversingPlayer->isSpectator())
                {
                    const double progress = Minecraft::GetInstance()->GetClientMasterGameMode()->GetPlayerMapProgress(
                        localPlayer->mXuid);
                    if (showTimer && !clientGameMode->IsWaitingToStart() && !clientGameMode->IsRoundStarting()
                        && !scoreboard->GetEndZoneEntered(localPlayer->getSUID(), -1))
                    {
                        if (traversingPlayer->GetPercentageTraversed() <= progress || !localPlayer->isAlive())
                        {
                            m_wrongWayTicks = 0;
                            SetWrongWay(L"");
                            m_wrongWayVisible = false;
                        }
                        else if (++m_wrongWayTicks < 61)
                        {
                            SetWrongWay(L"");
                            m_wrongWayVisible = false;
                        }
                        else if (!m_wrongWayVisible && !localPlayer->isSpectator())
                        {
                            SetWrongWay(L10N::GetString(IDS_MG03_WRONG_WAY));
                            m_wrongWayVisible = true;
                        }
                    }
                    else
                    {
                        m_wrongWayTicks = 0;
                        SetWrongWay(L"");
                        m_wrongWayVisible = false;
                    }
                    traversingPlayer->SetPercentageTraversed(progress);
                }
            }

            for (int index = 0; index < 8; ++index)
            {
                boost::shared_ptr<Player> trackedPlayer = Minecraft::GetInstance()->GetPlayerByPlayerIndex(index);
                if (trackedPlayer == localPlayer && localPlayer->isSpectator()
                    && eTYPE_DERIVED_FROM(EntityType::PLAYER, localPlayer->GetCamera()->GetType()))
                    trackedPlayer = boost::static_pointer_cast<Player>(localPlayer->GetCamera());
                if (trackedPlayer != 0)
                {
                    SetPercentageMapTraversed(m_gameModePlayerSlots[index],
                                              Minecraft::GetInstance()->GetClientMasterGameMode()->GetPlayerMapProgress(
                                                  trackedPlayer->mXuid));
                }
            }

            GhostController* ghostController = Minecraft::GetInstance()->GetGhostController();
            if (m_ghostPlayerSlot >= 0)
            {
                if (ghostController == 0)
                {
                    SetPercentageMapTraversed(m_ghostPlayerSlot, 0.0);
                }
                else
                {
                    boost::shared_ptr<GhostPlayer> ghostPlayer = ghostController->getGhostPlayer(pad);
                    if (ghostPlayer != 0)
                        SetPercentageMapTraversed(m_ghostPlayerSlot, ghostPlayer->getProgress());
                }
            }
        }

        ClientScoreboard* scoreboard = localPlayer->GetLevel()->GetClientScoreboard();
        ClientMasterGameMode* clientGameMode = minecraft->GetClientMasterGameMode();
        if (localPlayer->GetGameType() == GameType::LOBBY)
        {
            m_personalBestTime = 0;
            ShowAnimatedSplash(L"", false, -1);
            m_timedSplashLocked = false;
        }
        else
        {
            GhostController* ghostController = Minecraft::GetInstance()->GetGhostController();
            if (clientGameMode->IsRoundStarting())
            {
                if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && scoreboard != 0)
                {
                    std::wstring splash;
                    if (CommonMasterGameMode::GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
                    {
                        if (scoreboard->mCurrentRound
                            == static_cast<int>(Minecraft::GetInstance()->GetMiniGame()->GetFinalRoundIndex()))
                        {
                            splash = L10N::GetString(IDS_MG03_ROUND_FINAL);
                            ShowAnimatedSplash(splash, true, 0);
                        }
                        else
                        {
                            splash = L10N::GetString(IDS_MG03_ROUND_NUMBER);
                            splash.replace(splash.find(L"%d"), 2, _toString<int>(scoreboard->mCurrentRound + 1));
                            ShowAnimatedSplash(splash, true, 0);
                        }
                    }
                    else
                    {
                        splash = L10N::GetString(IDS_MG03_ROUND_SOLO);
                        ShowAnimatedSplash(splash, true, 0);
                    }
                }
                m_resultSplashShown = false;
            }
            else if (clientGameMode->IsRoundEnding())
            {
                if (!Minecraft::InMiniGame(eMiniGameId_Glide, true))
                {
                    if (scoreboard == 0 || !scoreboard->HasWinner(-1))
                    {
                        ShowAnimatedSplash(L10N::GetString(0xd2622bfa), true, 0);
                    }
                    else
                    {
                        std::wstring splash = L10N::GetString(IDS_MG_PLAYER_WON);
                        splash = replaceAll(splash, L"{*PLAYER*}", scoreboard->GetWinnerName(-1));
                        ShowAnimatedSplash(splash, true, 0);
                    }
                }
                else if (scoreboard != 0)
                {
                    StatsUID uid = localPlayer->getSUID();
                    std::wstring splash;
                    if (minecraft->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0
                        && scoreboard->mCurrentRound
                               == static_cast<int>(Minecraft::GetInstance()->GetMiniGame()->GetFinalRoundIndex()))
                    {
                        if (!scoreboard->GlideHasOverallWinner())
                        {
                            splash = L10N::GetString(0xd2622bfa);
                            ShowAnimatedSplash(splash, true, 1);
                        }
                        else
                        {
                            splash = L10N::GetString(IDS_MG_PLAYER_WON);
                            splash = replaceAll(splash, L"{*PLAYER*}",
                                                scoreboard->GetGlideOverallWinner(
                                                    CommonMasterGameMode::GetMiniGame()->ArePointsActive()));
                            ShowAnimatedSplash(splash, true, 0);
                        }
                    }
                    else if (!CommonMasterGameMode::GetMiniGame()->ArePointsActive())
                    {
                        if (!scoreboard->GetEndZoneEntered(uid, -1))
                        {
                            splash = L10N::GetString(IDS_DNF);
                            ShowAnimatedSplash(splash, true, 1);
                        }
                        else
                        {
                            const unsigned int lifeSpan = scoreboard->GetLifeSpan(uid, -1);
                            splash = formatwstr(L"%.2d:%.2d:%.3d", lifeSpan / 60000, (lifeSpan / 1000) % 60,
                                                lifeSpan % 1000);
                            ShowAnimatedSplash(splash, true, 2);
                        }
                    }
                    else
                    {
                        splash = formatwstr(L"%d", scoreboard->GetPoints(uid, -1));
                        ShowAnimatedSplash(splash, true, 2);
                    }
                }
                m_resultSplashShown = true;
            }
            else if (!m_resultSplashShown && clientGameMode->IsRoundPlaying()
                     && Minecraft::InMiniGame(eMiniGameId_Glide, true) && !localPlayer->isSpectator() && scoreboard != 0
                     && scoreboard->GetEndZoneEntered(localPlayer->getSUID(), -1)
                     && (ghostController == 0 || !ghostController->mRecordingActive[m_playerId]))
            {
                StatsUID uid = localPlayer->getSUID();
                std::wstring splash;
                if (!CommonMasterGameMode::GetMiniGame()->ArePointsActive())
                {
                    if (!scoreboard->IsPersonalBest(uid, -1)
                        || (m_personalBestTime != 0 && System::processTimeInMilliSecs() >= m_personalBestTime + 3000))
                    {
                        const unsigned int lifeSpan = scoreboard->GetLifeSpan(uid, -1);
                        splash = formatwstr(L"%.2d:%.2d:%.3d", lifeSpan / 60000, (lifeSpan / 1000) % 60,
                                            lifeSpan % 1000);
                    }
                    else
                    {
                        if (m_personalBestTime == 0)
                            m_personalBestTime = System::processTimeInMilliSecs();
                        splash = L10N::GetString(IDS_MG03_PERSONAL_BEST);
                    }
                }
                else if (!scoreboard->IsPersonalBest(uid, -1)
                         || (m_personalBestTime != 0 && System::processTimeInMilliSecs() >= m_personalBestTime + 3000))
                {
                    splash = formatwstr(L"%d", scoreboard->GetPoints(uid, -1));
                }
                else
                {
                    if (m_personalBestTime == 0)
                        m_personalBestTime = System::processTimeInMilliSecs();
                    splash = L10N::GetString(IDS_MG03_PERSONAL_BEST);
                }
                showTimedSplash(splash, 60, 2);
                m_resultSplashShown = true;
                m_timedSplashLocked = true;
            }
            else
            {
                m_personalBestTime = 0;
                if (m_timedSplashTicks == 0)
                {
                    ShowAnimatedSplash(L"", false, -1);
                    m_timedSplashLocked = false;
                }
                else
                {
                    ShowAnimatedSplash(m_timedSplashText, true, m_timedSplashIcon);
                    --m_timedSplashTicks;
                }
            }
        }
    }
}
void UIScene_HUD::handleGameTick()
{
    if (m_movie == 0 || gApp.GetGameStarted() == 0)
        return;

    Minecraft* minecraft = Minecraft::GetInstance();
    const int playerId = m_playerId;
    if (minecraft->mLocalPlayers[playerId] == 0 || minecraft->mGameModes[playerId] == 0)
    {
        m_layer->showComponent(playerId, EUIScene_HUD, false);
        return;
    }

    m_layer->showComponent(playerId, EUIScene_HUD, true);
    updateFrameTick();
    GetHudLoading();
}
void UIScene_HUD::render(int width, int height, C4JRender::eViewportType viewport)
{
    if (!m_splitScreen)
    {
        UIScene::render(width, height, viewport);
        return;
    }

    int screenWidth;
    int screenHeight;
    gConsoleUIController.getRenderDimensions(C4JRender::eViewportType_FullScreen, screenWidth, screenHeight);

    int renderWidth = width;
    int renderHeight = height;
    int movieY = 0;
    switch (viewport)
    {
    case C4JRender::eViewportType_TopHalf:
        gConsoleUIController.setupRenderPosition(0, 0);
        renderWidth = screenWidth;
        movieY = m_stageHeight / 2;
        break;
    case C4JRender::eViewportType_BottomHalf:
        gConsoleUIController.setupRenderPosition(0, screenHeight / 2);
        renderWidth = screenWidth;
        movieY = m_stageHeight / 2;
        break;
    case C4JRender::eViewportType_LeftHalf:
        gConsoleUIController.setupRenderPosition(0, 0);
        renderHeight = screenHeight;
        break;
    case C4JRender::eViewportType_RightHalf:
        gConsoleUIController.setupRenderPosition(screenWidth / 2, 0);
        renderHeight = screenHeight;
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        gConsoleUIController.setupRenderPosition(0, 0);
        movieY = m_stageHeight / 2;
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        gConsoleUIController.setupRenderPosition(screenWidth / 2, 0);
        movieY = m_stageHeight / 2;
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        gConsoleUIController.setupRenderPosition(0, screenHeight / 2);
        movieY = m_stageHeight / 2;
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        gConsoleUIController.setupRenderPosition(screenWidth / 2, screenHeight / 2);
        movieY = m_stageHeight / 2;
        break;
    default:
        gConsoleUIController.setupRenderPosition(0, 0);
        break;
    }

    gFui->render(m_movie, 0.0f, static_cast<float>(movieY), static_cast<float>(renderWidth),
                 static_cast<float>(renderHeight));
}
void UIScene_HUD::updateViewportTouchOffset()
{
    const float half = 0.5f;
    switch (m_layer->getViewport())
    {
    case C4JRender::eViewportType_TopHalf:
    case C4JRender::eViewportType_TopLeftQuarter:
        gConsoleUIController.updateViewportTouchOffset(0, -(int)(gConsoleUIController.m_screenHeight * half));
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        gConsoleUIController.updateViewportTouchOffset((int)(gConsoleUIController.m_screenWidth * half),
                                                       -(int)(gConsoleUIController.m_screenHeight * half));
        break;
    case C4JRender::eViewportType_RightHalf:
    case C4JRender::eViewportType_BottomRightQuarter:
        gConsoleUIController.updateViewportTouchOffset((int)(gConsoleUIController.m_screenWidth * half), 0);
        break;
    default:
        gConsoleUIController.updateViewportTouchOffset(0, 0);
        break;
    }
}
void UIScene_HUD::handleTimerComplete(int)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    const int playerId = m_playerId;
    Gui* gui = minecraft->mGui;
    if (minecraft->mLocalPlayers[playerId] == 0 || gui == 0)
    {
        for (int i = 0; i < 10; ++i)
        {
            m_messageControls[i].setOpacity(0.0f);
            m_messageLabels[i].setOpacity(0.0f);
            m_messageLabels[i].setLabel(UIString(L""), false, false);
        }
        m_selectedLabel.setOpacity(0.0f);
    }
    else
    {
        for (int i = 0; i < 10; ++i)
        {
            float opacity = static_cast<float>(gui->getOpacity(playerId, static_cast<unsigned int>(i)));
#ifdef MINECRAFT_PC
            if (minecraft->mLocalPlayers[playerId]->mViewport == C4JRender::eViewportType_FullScreen)
                opacity = 0.0f;
#endif
            if (opacity > 0.0f)
            {
                m_messageControls[i].setOpacity(opacity);
                m_messageLabels[i].setOpacity(opacity);
                if (gui->mMessages[m_playerId].empty())
                {
                    m_messageLabels[i].setLabel(UIString(std::wstring(L"")), false, false);
                }
                else
                {
                    std::wstring message = gui->mMessages[m_playerId].at(i).message;
                    std::wstring escaped = escapeXML(message);
                    m_messageLabels[i].setLabel(UIString(escaped), false, false);
                }
            }
            else
            {
                m_messageControls[i].setOpacity(0.0f);
                m_messageLabels[i].setOpacity(0.0f);
                m_messageLabels[i].setLabel(UIString(L""), false, false);
            }
        }

        m_selectedLabel.setOpacity(static_cast<float>(gui->getJukeboxOpacity(m_playerId)));
        std::wstring selected = gui->mSingleMessages[m_playerId];
        m_selectedLabel.setLabel(UIString(selected), false, false);
    }
}
int UIScene_HUD::getSceneType()
{
    return EUIScene_HUD;
}
bool UIScene_HUD::stealsFocus()
{
    return false;
}
bool UIScene_HUD::hasFocus(int)
{
    return false;
}
bool UIScene_HUD::hidesLowerScenes()
{
    return false;
}
