#include "game/ui/UIScene_InGameSettingsListMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ClientboundMGPlayerSettingsUpdatePacket.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerSettingsChangedPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_MultilistMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "platform/wiiu/CProfile.hpp"

extern ConsoleUIController gConsoleUIController;

template <typename T> std::wstring _toString(T value);

namespace
{
float arenaSizes[] = {0.5f, 0.625f, 0.75f, 0.875f, 1.0f};
float arenaSizeLabels[] = {1.0f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f};
}

UIScene_InGameSettingsListMenu::UIScene_InGameSettingsListMenu(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_miniGameId = Minecraft::GetInstance()->GetFirstMiniGame()->GetId();
    m_customGame = Minecraft::GetInstance()->GetMiniGame()->GetId() == eMiniGameId_Lobby;
    m_launchData.mMiniGameId = m_miniGameId;
    m_launchData.mMatchmaking = CGameNetworkManager::sInstance.IsInPublicJoinableGame();
    m_launchData.mTimeTrial =
        Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0;
    m_mapSelection = 0;
    m_launchData.mPad = sceneType;
    switch (m_miniGameId)
    {
    case eMiniGameId_Battle:
        m_mapSelection = GameSettings::GetBattleMiniGameMapSelection(sceneType);
        break;
    case eMiniGameId_Glide:
        m_mapSelection = GameSettings::GetGlideMiniGameMapSelection(sceneType);
        break;
    }
    Init();
}

UIScene_InGameSettingsListMenu::~UIScene_InGameSettingsListMenu()
{
}

void UIScene_InGameSettingsListMenu::EnableCustomOptions(bool enabled)
{
    if (m_miniGameId != eMiniGameId_Battle && m_miniGameId != eMiniGameId_Tumble)
        return;

    m_multiList.EnableItem(2, enabled, true);
    m_multiList.EnableItem(8, enabled, true);
    m_multiList.EnableItem(4, enabled, true);
    if (m_miniGameId == eMiniGameId_Battle)
    {
        unsigned int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
        int mapSize = Minecraft::GetInstance()->GetMiniGame()->GetMapSizeOverride();
        bool largeSession = (mapSize == 1 || mapSize == 2) && playerCount > 8;
        bool enableCentreSpawn = enabled && !largeSession;
        m_multiList.EnableItem(0, enableCentreSpawn, true);
        m_multiList.EnableItem(5, enabled, true);
        m_multiList.EnableItem(6, enabled, true);
        m_multiList.EnableItem(7, enabled, true);
        m_multiList.EnableItem(9, enabled, true);
        m_multiList.EnableItem(10, enabled, true);
        m_multiList.EnableItem(11, enabled, true);
        m_multiList.EnableItem(15, enabled, true);
        m_multiList.EnableItem(13, enabled, true);
        m_multiList.EnableItem(14, enabled, true);
        if (!largeSession)
            m_multiList.SetCheckboxValue(
                0, m_playerSettings.mHostOptions.Get(eCustomGameModeHostOption_CentreSpawn) != 0, true);
        m_multiList.SetCheckboxValue(5, m_playerSettings.mHostOptions.Get(eCustomGameModeHostOption_NoSmallSkins) == 0,
                                     true);
        m_multiList.SetCheckboxValue(11, m_playerSettings.mNaturalRegeneration, true);
        m_multiList.SetCheckboxValue(13, m_playerSettings.mSmallInventoryEnabled, true);
        m_multiList.SetCheckboxValue(14, m_playerSettings.mTakeEverythingEnabled, true);
        m_multiList.SetCheckboxValue(15, m_playerSettings.mChestRefillEnabled, true);
    }
    else if (m_miniGameId == eMiniGameId_Tumble)
    {
        m_multiList.EnableItem(6, enabled, true);
        m_multiList.EnableItem(16, enabled, true);
        m_multiList.EnableItem(17, enabled, true);
        m_multiList.EnableItem(18, enabled, true);
        m_multiList.SetCheckboxValue(18, m_playerSettings.mSpectatorParticipation, true);
    }
}

void UIScene_InGameSettingsListMenu::doVerticalResizeCheck()
{
    FJ_MultilistMenu* menu = static_cast<FJ_MultilistMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->DoVerticalResizeCheck();
}

void UIScene_InGameSettingsListMenu::Init()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    m_gameStarted = !(minecraft->mLevel == 0);
    m_primaryPlayer = gProfile->GetPrimaryPad() == m_playerId;
    m_host = CGameNetworkManager::sInstance.IsHost();
    bool canEdit = m_host && m_primaryPlayer;
    wchar_t text[256];

    MiniGameDef::GetPlayerSettings(m_playerSettings);
    MiniGameDef::GetPlayerSettings(m_originalPlayerSettings);

    if (MiniGameDef::GetCustomGameModeById(m_miniGameId, true)->HasMultipleMaps() && canEdit && m_customGame)
    {
        if (m_miniGameId == eMiniGameId_Battle)
        {
            m_multiList.AddNewButton(std::wstring(L10N::GetString(IDS_MG_SELECT_MAPS)), 1, true);
            m_multiList.EnableItem(1, true, true);
        }
    }

    if (MiniGameDef::GetCustomGameModeById(m_miniGameId, true)->HasRoundLengthOption())
    {
        m_roundLengthSlider = MultiListSliderLogic(2, IDS_MG_OPTION_ROUND_LENGTH, eMiniGameId_Battle,
                                                   eCustomGameModeHostOption_RoundLength);
        m_roundLengthSlider.AddTo(m_multiList,
                                  m_playerSettings.mHostOptions.Get(eCustomGameModeHostOption_RoundLength));
    }

    if (MiniGameDef::GetCustomGameModeById(m_miniGameId, true)->HasCentralSpawnOption())
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG_OPTION_CENTRE_SPAWN)), 0,
                                   m_playerSettings.mHostOptions.Get(eCustomGameModeHostOption_CentreSpawn) != 0);

    bool supportsInfinite;
    if (m_playerSettings.mLivesPerRound == 0)
    {
        std::swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG_LIVES_PER_ROUND),
                      L10N::GetString(IDS_MG_INFINITE));
        supportsInfinite = MiniGameDef::GetCustomGameModeById(m_miniGameId, true)->HasFeatureSupportForInfiniteLives();
    }
    else
    {
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG_LIVES_PER_ROUND), m_playerSettings.mLivesPerRound);
        supportsInfinite = MiniGameDef::GetCustomGameModeById(m_miniGameId, true)->HasFeatureSupportForInfiniteLives();
    }
    m_multiList.AddNewSlider(std::wstring(text), 8, !supportsInfinite, 10, m_playerSettings.mLivesPerRound, false, 7,
                             3);

    m_gameVariantSlider = MultiListSliderLogic(4, IDS_MG_OPTION_SPECTATE_MODE, m_miniGameId,
                                               eCustomGameModeHostOption_SpectateMode);
    m_gameVariantSlider.AddTo(m_multiList, m_playerSettings.mHostOptions.Get(eCustomGameModeHostOption_SpectateMode));

    if (m_miniGameId == eMiniGameId_Battle)
    {
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG_OPTION_ALLOW_ALL_SKINS)), 5,
                                   m_playerSettings.mHostOptions.Get(eCustomGameModeHostOption_NoSmallSkins) == 0);

        int lootSet = m_playerSettings.mLootSetId;
        if (m_customGame && m_playerSettings.mRandomLootSet)
            lootSet = 5;
        std::wstring label = std::wstring(L10N::GetString(IDS_MG01_ITEMSET)) + L": "
                             + MiniGameDef::GetCustomGameModeById(m_miniGameId, true)->GetLootSetText(lootSet);
        m_multiList.AddNewSlider(label, 6, 0, 5, lootSet, false, 7, 3);

        label = std::wstring(L10N::GetString(0x1cbfab4a)) + L": "
                + MiniGameDef::GetCustomGameModeById(m_miniGameId, true)
                      ->GetFoodBalanceIndexText(m_playerSettings.mFoodBalance);
        m_multiList.AddNewSlider(label, 7, 0, 7, m_playerSettings.mFoodBalance, false, 7, 3);
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG01_ROUND_COUNT), m_playerSettings.mRoundCount);
        m_multiList.AddNewSlider(std::wstring(text), 9, 1, 10, m_playerSettings.mRoundCount, false, 7, 3);

        label = std::wstring(L10N::GetString(IDS_MG01_MAP_SIZE)) + L": "
                + MiniGameDef::GetCustomGameModeById(m_miniGameId, true)
                      ->GetMapSizeOverrideText(
                          m_playerSettings.mMapSizeOverride.isSet() ? m_playerSettings.mMapSizeOverride.get() : 0);
        m_multiList.AddNewSlider(
            label, 10, 0, 2, m_playerSettings.mMapSizeOverride.isSet() ? m_playerSettings.mMapSizeOverride.get() : 0,
            false, 7, 3);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG01_NATURAL_REGENERATION)), 11,
                                   m_playerSettings.mNaturalRegeneration);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG01_SMALL_INVENTORY)), 13,
                                   m_playerSettings.mSmallInventoryEnabled);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG01_TAKE_EVERYTHING)), 14,
                                   m_playerSettings.mTakeEverythingEnabled);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG01_CHEST_REFILL_SETTING)), 15,
                                   m_playerSettings.mChestRefillEnabled);
    }
    else if (m_miniGameId == eMiniGameId_Tumble)
    {
        std::wstring lootLabel =
            std::wstring(L10N::GetString(IDS_MG01_ITEMSET)) + L": "
            + MiniGameDef::GetCustomGameModeById(m_miniGameId, true)->GetLootSetText(m_playerSettings.mLootSetId);
        m_multiList.AddNewSlider(lootLabel, 6, 0, 3, m_playerSettings.mLootSetId, false, 7, 3);

        int arenaSize = 0;
        if (m_playerSettings.mArenaSizeMultiplier.isSet())
        {
            float multiplier = m_playerSettings.mArenaSizeMultiplier.get();
            if (multiplier == arenaSizes[0])
                arenaSize = 1;
            else if (multiplier == arenaSizes[1])
                arenaSize = 2;
            else if (multiplier == arenaSizes[2])
                arenaSize = 3;
            else if (multiplier == arenaSizes[3])
                arenaSize = 4;
            else if (multiplier == arenaSizes[4])
                arenaSize = 5;
            std::swprintf(text, 256, L"%ls: x%ls", L10N::GetString(IDS_MG02_LAYER_SCALE),
                          _toString<float>(arenaSizeLabels[arenaSize]).c_str());
        }
        else
        {
            std::swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG02_LAYER_SCALE), L10N::GetString(IDS_MG_AUTO));
        }
        m_multiList.AddNewSlider(std::wstring(text), 16, 0, 5, arenaSize, true, 7, 3);

        int layerCount;
        if (m_playerSettings.mLayerCount.isSet())
            layerCount = m_playerSettings.mLayerCount.get();
        else
            layerCount = 0;
        if (layerCount > 0)
            std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG02_LAYER_COUNT), layerCount);
        else
            std::swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG02_LAYER_COUNT), L10N::GetString(IDS_MG_AUTO));
        m_multiList.AddNewSlider(std::wstring(text), 17, 0, 8, layerCount, false, 7, 3);
        m_multiList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG02_SPECTATOR_PARTICIPATION)), 18,
                                   m_playerSettings.mSpectatorParticipation);
    }

    m_description.init(UIString(L""));
    EnableCustomOptions(canEdit && m_customGame);
    doVerticalResizeCheck();
    doHorizontalResizeCheck();
    gApp.GetLocalPlayerCount();
    gConsoleUIController.TouchBoxRebuild(this);
}

unsigned int UIScene_InGameSettingsListMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    {
        m_multiList.setupControl(this, root, std::string("MultiList"));
        m_controls.push_back(&m_multiList);
    }
    {
        m_description.setupControl(this, root, std::string("Description"));
        m_controls.push_back(&m_description);
    }
    return 1;
}

void UIScene_InGameSettingsListMenu::UpdateSettings(bool unavailable)
{
    int activeItem = m_multiList.GetActiveItem();
    m_multiList.clearList();
    Init();
    m_multiList.setCurrentSelection(activeItem, false, true);
    int parameters[1];
    parameters[0] = IDS_CONFIRM_OK;
    if (unavailable)
    {
        gConsoleUIController.RequestAlertMessage(IDS_MG_SETTINGS_UPDATED_TITLE, 0xd5232b99, parameters, 1, m_playerId,
                                                 NULL, NULL, NULL, true);
        m_multiList.EnableItem(0, false, true);
        m_multiList.SetCheckboxValue(0, false, true);
    }
    else
    {
        gConsoleUIController.RequestAlertMessage(IDS_MG_SETTINGS_UPDATED_TITLE, IDS_MG_SETTINGS_UPDATED_MSG, parameters,
                                                 1, m_playerId, NULL, NULL, NULL, true);
    }
}

void UIScene_InGameSettingsListMenu::tick()
{
    UIScene::tick();
}

void UIScene_InGameSettingsListMenu::updateTooltips()
{
    bool canEdit = m_host && m_primaryPlayer;
    int accept = -1;
    if (canEdit)
        accept = IDS_TOOLTIPS_SELECT;
    gConsoleUIController.SetTooltips(m_playerId, accept, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, false, false);
}

void UIScene_InGameSettingsListMenu::handleReload()
{
    Init();
}

void UIScene_InGameSettingsListMenu::updateComponents()
{
    bool inMenu = Minecraft::GetInstance()->mLevel == 0;
    bool showLogo;
    if (inMenu)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        showLogo = true;
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        showLogo = false;
        if (gApp.GetLocalPlayerCount() == 1)
            showLogo = true;
    }
    m_layer->showComponent(m_playerId, EUIScene_Logo, showLogo);
}

std::wstring UIScene_InGameSettingsListMenu::getMoviePath()
{
    if (gApp.GetLocalPlayerCount() > 1)
        return std::wstring(L"MultilistMenuSplit");
    return std::wstring(L"MultilistMenu");
}

void UIScene_InGameSettingsListMenu::setGameSettings()
{
    if (!m_host || !m_primaryPlayer)
        return;

    if (m_miniGameId != eMiniGameId_Battle && m_miniGameId != eMiniGameId_Tumble)
        return;

    unsigned int hostOptions = gApp.GetGameHostOption(eGameHostOption_All);
    if (m_miniGameId == eMiniGameId_Battle)
    {
        m_playerSettings.mHostOptions.Set(eCustomGameModeHostOption_CentreSpawn, m_multiList.GetCheckboxValue(0));
        m_playerSettings.mHostOptions.Set(eCustomGameModeHostOption_NoSmallSkins, !m_multiList.GetCheckboxValue(5));
        m_playerSettings.mNaturalRegeneration = m_multiList.GetCheckboxValue(11);
        m_playerSettings.mSmallInventoryEnabled = m_multiList.GetCheckboxValue(13);
        m_playerSettings.mTakeEverythingEnabled = m_multiList.GetCheckboxValue(14);
        m_playerSettings.mChestRefillEnabled = m_multiList.GetCheckboxValue(15);
        gApp.SetGameHostOption(hostOptions, eGameHostOption_NaturalRegeneration, m_playerSettings.mNaturalRegeneration);
    }
    else if (m_miniGameId == eMiniGameId_Tumble)
    {
        m_playerSettings.mSpectatorParticipation = m_multiList.GetCheckboxValue(18);
    }

    MiniGameDef::SetPlayerSettings(m_playerSettings);

    bool broadcast = false;
    if (!(m_playerSettings == m_originalPlayerSettings))
        broadcast = true;

    unsigned int mapSelection = 0;
    switch (m_miniGameId)
    {
    case eMiniGameId_Battle:
        mapSelection = GameSettings::GetBattleMiniGameMapSelection(m_playerId);
        break;
    case eMiniGameId_Glide:
        mapSelection = GameSettings::GetGlideMiniGameMapSelection(m_playerId);
        break;
    }

    if (mapSelection != m_mapSelection)
    {
        (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)
            ->broadcastPlaylistInfo(&m_launchData.mLevelRulesetGroups);
        broadcast = true;
    }

    if (broadcast)
    {
        (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)
            ->broadcastAll(Minecraft::GetInstance()->GetMiniGame()->GetPlayerSettingsPacket());
    }

    if (hostOptions != gApp.GetGameHostOption(eGameHostOption_All))
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[m_playerId];
        if (player->mPacketListener != NULL)
        {
            player->mPacketListener->send(boost::shared_ptr<Packet>(new ServerSettingsChangedPacket(1, hostOptions)));
        }
    }
}

void UIScene_InGameSettingsListMenu::UpdateDescriptionText()
{
    static int descriptionIds[] = {IDS_MG_OPTION_DESC_CENTRE_SPAWN,
                                   IDS_MG_OPTION_DESC_MAP_SELECT,
                                   IDS_MG_OPTION_DESC_ROUND_LENGTH,
                                   IDS_MASHUP_CHOOSE_BETWEEN_SEVERAL_VARIANTS_SAME_CORE_GAME_TEXT,
                                   static_cast<int>(IDS_MG_OPTION_DESC_SPECTATE_MODE),
                                   static_cast<int>(0xbe68971f),
                                   0,
                                   0,
                                   IDS_MG_LIVES_PER_ROUND_DESC,
                                   IDS_MG01_ROUND_COUNT_DESC,
                                   IDS_MG01_MAP_SIZE_DESC,
                                   static_cast<int>(IDS_MG01_NATURAL_REGENERATION_DESC),
                                   static_cast<int>(0xf7c16b2c),
                                   static_cast<int>(IDS_MG01_SMALL_INVENTORY_DESC),
                                   static_cast<int>(IDS_MG01_TAKE_EVERYTHING_DESC),
                                   static_cast<int>(IDS_MG01_CHEST_REFILL_SETTING_DESC),
                                   IDS_MG02_LAYER_SCALE_DESC,
                                   IDS_MG02_LAYER_COUNT_DESC,
                                   static_cast<int>(IDS_MG02_SPECTATOR_PARTICIPATION_DESC)};

    if (m_childFocus == -1.0)
        return;

    int descriptionId = descriptionIds[static_cast<int>(m_childFocus)];
    std::wstring description(L"");
    if (descriptionId != -1 && descriptionId != -2)
        description = gApp.FormatHTMLString(m_playerId, std::wstring(L10N::GetString(descriptionId)), -1);
    if (!gRenderer->IsHiDef())
        gRenderer->IsWidescreen();

    if (m_childFocus == 6.0)
        description = MiniGameDef::GetCustomGameModeById(m_miniGameId, true)
                          ->GetLootSetDescription(m_multiList.GetSliderValue(6));
    else if (m_childFocus == 7.0)
        description = MiniGameDef::GetCustomGameModeById(m_miniGameId, true)
                          ->GetFoodBalanceIndexDescription(m_multiList.GetSliderValue(7));

    wchar_t colourTag[64];
    std::swprintf(colourTag, 64, L"<font color=\"#%08x\">", gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White));
    description = colourTag + description;
    m_description.setLabel(UIString(description), false, false);
}

void UIScene_InGameSettingsListMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                                 bool& handled)
{
    gConsoleUIController.AnimateKeyPress(playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            setGameSettings();
            navigateBack();
            handled = true;
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    }
}

void UIScene_InGameSettingsListMenu::handleFocusChange(int focus, int childFocus)
{
    m_focus = static_cast<double>(focus);
    m_childFocus = static_cast<double>(childFocus);
    UpdateDescriptionText();
}

void UIScene_InGameSettingsListMenu::handlePress(int, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (childId == 1)
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_MapSelectMenu, &m_launchData, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
}

void UIScene_InGameSettingsListMenu::handleCheckboxElementToggled(int, int, bool)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void UIScene_InGameSettingsListMenu::handleSliderElementMove(int, int element, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    wchar_t text[256];
    switch (element)
    {
    case 2:
        m_playerSettings.mHostOptions.Set(eCustomGameModeHostOption_RoundLength, value);
        m_roundLengthSlider.Update(m_multiList, value);
        return;
    case 4:
        m_playerSettings.mHostOptions.Set(eCustomGameModeHostOption_SpectateMode, value);
        m_gameVariantSlider.Update(m_multiList, value);
        return;
    case 6:
    {
        gMinecraftServer->mMasterGameMode->mRandomLootSetChosen = false;
        m_playerSettings.mLootSetId = static_cast<unsigned int>(m_multiList.GetSliderValue(6));
        if (m_miniGameId == eMiniGameId_Battle)
            m_playerSettings.mRandomLootSet = m_playerSettings.mLootSetId == 5;
        std::wstring label =
            std::wstring(L10N::GetString(IDS_MG01_ITEMSET)) + L": "
            + MiniGameDef::GetCustomGameModeById(m_miniGameId, true)->GetLootSetText(m_playerSettings.mLootSetId);
        m_multiList.SetItemLabel(6, UIString(label), false);
        UpdateDescriptionText();
        break;
    }
    case 7:
    {
        m_playerSettings.mFoodBalance = static_cast<int>(m_multiList.GetSliderValue(7));
        std::wstring label = std::wstring(L10N::GetString(0x1cbfab4a)) + L": "
                             + MiniGameDef::GetCustomGameModeById(m_miniGameId, true)
                                   ->GetFoodBalanceIndexText(m_playerSettings.mFoodBalance);
        m_multiList.SetItemLabel(7, UIString(label), false);
        UpdateDescriptionText();
        break;
    }
    case 8:
        m_playerSettings.mLivesPerRound = m_multiList.GetSliderValue(8);
        if (m_playerSettings.mLivesPerRound == 0)
            std::swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG_LIVES_PER_ROUND),
                          L10N::GetString(IDS_MG_INFINITE));
        else
            std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG_LIVES_PER_ROUND),
                          m_playerSettings.mLivesPerRound);
        m_multiList.SetItemLabel(8, UIString(text), false);
        UpdateDescriptionText();
        return;
    case 9:
        m_playerSettings.mRoundCount = static_cast<int>(m_multiList.GetSliderValue(9));
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG01_ROUND_COUNT), m_playerSettings.mRoundCount);
        m_multiList.SetItemLabel(9, UIString(text), false);
        UpdateDescriptionText();
        return;
    case 10:
    {
        int mapSize = static_cast<int>(m_multiList.GetSliderValue(10));
        if (mapSize == 0)
            m_playerSettings.mMapSizeOverride = maybe<int>();
        else
            m_playerSettings.mMapSizeOverride = maybe<int>(mapSize);
        std::wstring label = std::wstring(L10N::GetString(IDS_MG01_MAP_SIZE)) + L": "
                             + MiniGameDef::GetCustomGameModeById(m_miniGameId, true)
                                   ->GetMapSizeOverrideText(m_playerSettings.mMapSizeOverride.otherwise(0));
        m_multiList.SetItemLabel(10, UIString(label), false);
        UpdateDescriptionText();
        break;
    }
    case 16:
    {
        int sizeIndex = static_cast<int>(m_multiList.GetSliderValue(16));
        if (sizeIndex == 0)
            m_playerSettings.mArenaSizeMultiplier = maybe<float>();
        else
            m_playerSettings.mArenaSizeMultiplier = maybe<float>(arenaSizes[sizeIndex - 1]);
        if (sizeIndex > 0)
            std::swprintf(text, 256, L"%ls: x%ls", L10N::GetString(IDS_MG02_LAYER_SCALE),
                          _toString<float>(arenaSizeLabels[sizeIndex]).c_str());
        else
            std::swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG02_LAYER_SCALE), L10N::GetString(IDS_MG_AUTO));
        m_multiList.SetItemLabel(16, UIString(text), false);
        UpdateDescriptionText();
        return;
    }
    case 17:
    {
        int layerCount = static_cast<int>(m_multiList.GetSliderValue(17));
        if (layerCount == 0)
            m_playerSettings.mLayerCount = maybe<unsigned int>();
        else
            m_playerSettings.mLayerCount = maybe<unsigned int>(layerCount);
        if (layerCount > 0)
            std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG02_LAYER_COUNT), layerCount);
        else
            std::swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG02_LAYER_COUNT), L10N::GetString(IDS_MG_AUTO));
        m_multiList.SetItemLabel(17, UIString(text), false);
        UpdateDescriptionText();
        return;
    }
    case 3:
        m_playerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, value);
        m_maxPlayersSlider.Update(m_multiList, value);
        break;
    default:
        break;
    }
}

int UIScene_InGameSettingsListMenu::getSceneType()
{
    return EUIScene_InGameSettingsListMenu;
}
