#include "game/ui/UIScene_SettingsListMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_MultilistMenu.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "platform/wiiu/CProfile.hpp"
#ifdef MINECRAFT_PC
#include "game/network/ClientPacketListener.hpp"
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#include "platform/pc/PcPanorama.hpp"
#include "platform/pc/PcTitleLogo.hpp"
#include "platform/pc/PcTutorials.hpp"
#include "platform/pc/PcVoice.hpp"
#include "platform/pc/PcKeyBindings.hpp"

static int s_pendingRenderDistance = -1;
static const int TITLE_LOGO_ITEM = 44;
static const int TUTORIAL_VERSION_ITEM = 45;
static const int PANORAMA_ITEM = 46;
static const int MICROPHONE_ITEM = 49;
static const int MIC_VOLUME_ITEM = 50;
static const int MIC_TEST_ITEM = 51;
static const int VOICE_MODE_ITEM = 52;
static const int PROXIMITY_CHAT_ITEM = 53;
static const int PROXIMITY_RANGE_ITEM = 54;
static const int VOICE_VOLUME_ITEM = 55;
static const size_t MIC_NAME_LENGTH = 40;
static const int GOLDEN_AGE_GROUP_COUNT = 6;
static const wchar_t* const goldenAgeLabels[GOLDEN_AGE_GROUP_COUNT] = {
    L"Old Block Textures", L"Old Water", L"Old Lava", L"Old Item Visuals", L"Old Models", L"Old Sound Effects"};
static const int goldenAgeGroups[GOLDEN_AGE_GROUP_COUNT] = {kPcGoldenAgeBlocks, kPcGoldenAgeWater,
                                                                kPcGoldenAgeLava,   kPcGoldenAgeItems,
                                                                kPcGoldenAgeModels, kPcGoldenAgeSounds};
static const int goldenAgeItemIds[GOLDEN_AGE_GROUP_COUNT] = {40, 47, 48, 41, 42, 43};

static void RenderDistanceLabel(wchar_t* text)
{
    const int distance = s_pendingRenderDistance >= 0 ? s_pendingRenderDistance : PcSettingsRenderDistance();
    std::swprintf(text, 256, L"Render Distance: %d chunks", distance);
}

static void ApplyPendingRenderDistance()
{
    const int distance = s_pendingRenderDistance;
    s_pendingRenderDistance = -1;
    if (distance < 0 || distance == PcSettingsRenderDistance())
        return;
    PcSettingsSetRenderDistance(distance);
    PcSettingsSave();
    ClientPacketListener::SendViewDistanceToAll();
}

static void FramerateLabel(int step, wchar_t* text)
{
    const int limit = PcSettingsFramerateLimit(step);
    if (limit < 0)
        std::swprintf(text, 256, L"Max Framerate: VSync");
    else if (limit == 0)
        std::swprintf(text, 256, L"Max Framerate: Unlimited");
    else
        std::swprintf(text, 256, L"Max Framerate: %d", limit);
}

static void RenderScaleLabel(int step, wchar_t* text)
{
    std::swprintf(text, 256, L"Render Scale: %d%%", PcSettingsRenderScale(step));
}

static void TitleLogoLabel(wchar_t* text)
{
    std::swprintf(text, 256, L"Title Logo: %ls", PcTitleLogoName(PcSettingsTitleLogo()));
}

static void TutorialVersionLabel(wchar_t* text)
{
    const int version = PcSettingsTutorialVersion();
    std::swprintf(text, 256, L"Tutorial Version: %ls%ls", PcTutorialVersionName(version),
                  PcTutorialVersionAvailable(version) ? L"" : L" (files missing)");
}

static void MicrophoneLabel(int device, wchar_t* text)
{
    std::wstring name = PcVoiceDeviceName(device);
    if (name.size() > MIC_NAME_LENGTH)
        name = name.substr(0, MIC_NAME_LENGTH - 3) + L"...";
    std::swprintf(text, 256, L"Input: %ls", name.c_str());
}

static void MicVolumeLabel(wchar_t* text)
{
    std::swprintf(text, 256, L"Input Volume: %d%%", PcSettingsMicVolume());
}

static void VoiceVolumeLabel(wchar_t* text)
{
    std::swprintf(text, 256, L"Voice Output Volume: %d%%", PcSettingsVoiceVolume());
}

static bool ProximityForced(int* forcedRange)
{
    int mode = kPcHostProximityUsersChoose;
    *forcedRange = 0;
    PcVoiceHostOverride(&mode, forcedRange);
    return mode != kPcHostProximityUsersChoose;
}

static void ProximityChatLabel(wchar_t* text)
{
    int range = 0;
    std::swprintf(text, 256, ProximityForced(&range) ? L"Proximity Chat (set by host)" : L"Proximity Chat");
}

static void ProximityRangeLabel(int blocks, wchar_t* text)
{
    int range = 0;
    ProximityForced(&range);
    if (range != 0)
        std::swprintf(text, 256, L"Proximity Range: %d blocks (set by host)", range);
    else
        std::swprintf(text, 256, L"Proximity Range: %d blocks", blocks);
}

static void VoiceModeLabel(wchar_t* text)
{
    if (PcSettingsVoiceMode() == kPcVoiceModePushToTalk)
        std::swprintf(text, 256, L"Voice: Push to Talk (%ls)", PcKeyCodeName(PcKeyBinding(ePcKey_PushToTalk)).c_str());
    else
        std::swprintf(text, 256, L"Voice: Voice Activated");
}

static void MicTestLabel(wchar_t* text)
{
    if (!PcVoiceTesting())
        std::swprintf(text, 256, L"Test Microphone");
    else if (!PcVoiceInputOpen())
        std::swprintf(text, 256, L"Test Microphone (no microphone)");
    else
        std::swprintf(text, 256, L"Test Microphone (level %d%%)", PcVoiceInputLevel());
}

static void PanoramaLabel(wchar_t* text)
{
    const int panorama = PcSettingsPanorama();
    std::swprintf(text, 256, L"Panorama: %ls%ls", PcPanoramaName(panorama),
                  PcPanoramaAvailable(panorama) ? L"" : L" (files missing)");
}
#endif

extern ConsoleUIController gConsoleUIController;

UIScene_SettingsListMenu::UIScene_SettingsListMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    int* settingsData = static_cast<int*>(data);
    m_settingsMode = static_cast<unsigned int>(settingsData[1]);
    delete settingsData;
    Init();
}

UIScene_SettingsListMenu::~UIScene_SettingsListMenu()
{
#ifdef MINECRAFT_PC
    ApplyPendingRenderDistance();
    if (m_settingsMode == 1)
        PcVoiceSetTest(false);
#endif
}

unsigned int UIScene_SettingsListMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_multiList.setupControl(this, currentNode, std::string("MultiList"));
        m_controls.push_back(&m_multiList);
    }
    {
        m_description.setupControl(this, currentNode, std::string("Description"));
        m_controls.push_back(&m_description);
    }
    return 1;
}

void UIScene_SettingsListMenu::doVerticalResizeCheck()
{
    FJ_MultilistMenu* menu = static_cast<FJ_MultilistMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->DoVerticalResizeCheck();
}

void UIScene_SettingsListMenu::HideDescription()
{
    FJ_MultilistMenu* menu = static_cast<FJ_MultilistMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->HideDescription();
}

inline void UIScene_SettingsListMenu::addSettingCheckbox(int playerId, const wchar_t* label, int id,
                                                         eGameSetting setting)
{
    m_multiList.AddNewCheckbox(std::wstring(label), id, GameSettings::GetGameSettings(playerId, setting) == 1);
}

inline void UIScene_SettingsListMenu::addInvertedSettingCheckbox(int playerId, const wchar_t* label, int id,
                                                                 eGameSetting setting)
{
    m_multiList.AddNewCheckbox(std::wstring(label), id, GameSettings::GetGameSettings(playerId, setting) != 1);
}

void UIScene_SettingsListMenu::Init()
{
    wchar_t text[256];
    Minecraft* minecraft = Minecraft::GetInstance();
    m_gameStarted = minecraft->mLevel != 0;
    m_primaryPlayer = gProfile->GetPrimaryPad() == m_playerId;
    m_host = CGameNetworkManager::sInstance.IsHost();
    m_saveRequested = false;

    switch (m_settingsMode)
    {
    case 0:
    {
        bool canChangeDifficulty = true;
        bool canChangeAutosave = true;
        bool showSaveButton = true;
        if (!m_primaryPlayer)
        {
            canChangeDifficulty = false;
            canChangeAutosave = false;
        }
        if (m_gameStarted)
        {
            canChangeDifficulty = false;
            if (!CGameNetworkManager::sInstance.IsHost())
                canChangeAutosave = false;
        }
        if (gApp.GetGameStarted())
            showSaveButton = false;
        if (m_primaryPlayer)
            addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_VERTICAL_SPLIT_SCREEN), 0,
                               eGameSetting_SplitScreenVertical);
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_AUTOJUMP), 9, eGameSetting_AutoJump);
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_VIEW_BOBBING), 1, eGameSetting_ViewBob);
        addInvertedSettingCheckbox(m_playerId, L10N::GetString(IDS_FLYING_VIEW_ROLLING), 2,
                                   eGameSetting_FlyingViewRolling);
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_HINTS), 3, eGameSetting_Hints);
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_DEATH_MESSAGES), 4, eGameSetting_DeathMessages);
        if (showSaveButton)
            m_multiList.AddNewButton(std::wstring(L10N::GetString(IDS_LANGUAGE_SELECTOR)), 5, false);
        if (canChangeAutosave)
        {
            int skin = static_cast<int>(GameSettings::GetGameSettings(m_playerId, eGameSetting_Autosave));
            for (unsigned int index = 0; index < 9; ++index)
            {
                if (index == 0)
                    std::swprintf(m_skinNames[index], 256, L"%ls", L10N::GetString(IDS_SLIDER_AUTOSAVE_OFF));
                else
                    std::swprintf(m_skinNames[index], 256, L"%ls: %d %ls", L10N::GetString(IDS_SLIDER_AUTOSAVE),
                                  index * 15, L10N::GetString(IDS_MINUTES));
            }
            m_multiList.AddNewSlider(std::wstring(m_skinNames[skin]), 6, 0, 8, skin, false, 7, 3);
        }
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_SENSITIVITY_INGAME),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_GameSensitivity));
        m_multiList.AddNewSlider(std::wstring(text), 7, 1, 200,
                                 GameSettings::GetGameSettings(m_playerId, eGameSetting_GameSensitivity), true, 7, 3);
        if (canChangeDifficulty)
        {
            int difficulty = static_cast<int>(GameSettings::GetGameSettings(m_playerId, eGameSetting_Difficulty));
            for (int index = 0; index < 4; ++index)
                std::swprintf(m_difficultyNames[index], 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                              L10N::GetString(Difficulty::byId(index)->getKey()));
            m_multiList.AddNewSlider(std::wstring(m_difficultyNames[difficulty]), 8, 0, 3, difficulty, false, 7, 3);
        }
    }
    break;
    case 1:
    {
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_MUSIC),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_MusicVolume));
        m_multiList.AddNewSlider(std::wstring(text), 10, 0, 100,
                                 GameSettings::GetGameSettings(m_playerId, eGameSetting_MusicVolume), true, 7, 3);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_SOUND),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_SoundVolume));
        m_multiList.AddNewSlider(std::wstring(text), 11, 0, 100,
                                 GameSettings::GetGameSettings(m_playerId, eGameSetting_SoundVolume), true, 7, 3);
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_CAVE_SOUNDS), 12, eGameSetting_CaveSounds);
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_MINECART_SOUNDS), 13, eGameSetting_MinecartSounds);
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_VOICE_CHAT), 14, eGameSetting_GameChat);
        m_multiList.EnableItem(14, true, true);
#ifdef MINECRAFT_PC
        {
            int forcedRange = 0;
            const bool forced = ProximityForced(&forcedRange);
            int effectiveRange = 0;
            const bool effective = PcVoiceEffectiveProximity(&effectiveRange);
            ProximityChatLabel(text);
            m_multiList.AddNewCheckbox(std::wstring(text), PROXIMITY_CHAT_ITEM,
                                       forced ? effective : PcSettingsProximityChat());
            if (forced)
                m_multiList.EnableItem(PROXIMITY_CHAT_ITEM, false, true);
            const int blocks = forcedRange != 0 ? forcedRange : PcSettingsProximityRange();
            ProximityRangeLabel(blocks, text);
            m_multiList.AddNewSlider(std::wstring(text), PROXIMITY_RANGE_ITEM,
                                     kPcProximityRangeMinimum / kPcProximityRangeStep,
                                     kPcProximityRangeMaximum / kPcProximityRangeStep, blocks / kPcProximityRangeStep,
                                     false, 7, 3);
            if (forcedRange != 0)
                m_multiList.EnableItem(PROXIMITY_RANGE_ITEM, false, true);
        }
        PcVoiceRefreshDevices();
        MicrophoneLabel(PcVoiceSelectedDevice(), text);
        m_multiList.AddNewSlider(std::wstring(text), MICROPHONE_ITEM, 0, PcVoiceDeviceCount() - 1,
                                 PcVoiceSelectedDevice(), false, 7, 3);
        MicVolumeLabel(text);
        m_multiList.AddNewSlider(std::wstring(text), MIC_VOLUME_ITEM, 0, kPcMicVolumeMaximum, PcSettingsMicVolume(),
                                 true, 7, 3);
        VoiceVolumeLabel(text);
        m_multiList.AddNewSlider(std::wstring(text), VOICE_VOLUME_ITEM, 0, 200, PcSettingsVoiceVolume(), true, 7, 3);
        VoiceModeLabel(text);
        m_multiList.AddNewSlider(std::wstring(text), VOICE_MODE_ITEM, 0, kPcVoiceModeCount - 1,
                                 PcSettingsVoiceMode(), false, 7, 3);
        MicTestLabel(text);
        m_multiList.AddNewCheckbox(std::wstring(text), MIC_TEST_ITEM, PcVoiceTesting());
#endif
    }
    break;
    case 2:
    {
        if (gProfile->GetPrimaryPad() == m_playerId)
        {
            addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_RENDER_CLOUDS), 15, eGameSetting_Clouds);
            if (!m_gameStarted || m_primaryPlayer)
                addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_CUSTOM_SKIN_ANIM), 16,
                                   eGameSetting_CustomSkinAnim);
            std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_GAMMA),
                          GameSettings::GetGameSettings(m_playerId, eGameSetting_Gamma));
            m_multiList.AddNewSlider(std::wstring(text), 17, 0, 100,
                                     GameSettings::GetGameSettings(m_playerId, eGameSetting_Gamma), true, 7, 3);
#ifdef MINECRAFT_PC
            std::swprintf(text, 256, L"FOV: %d", PcSettingsFov());
            m_multiList.AddNewSlider(std::wstring(text), 33, kPcFovMinimum, kPcFovMaximum, PcSettingsFov(), true, 7,
                                     3);
            FramerateLabel(PcSettingsFramerateStep(), text);
            m_multiList.AddNewSlider(std::wstring(text), 34, 0, kPcFramerateSteps - 1, PcSettingsFramerateStep(),
                                     false, 7, 3);
            RenderDistanceLabel(text);
            m_multiList.AddNewSlider(std::wstring(text), 36, kPcRenderDistanceMinimum, kPcRenderDistanceMaximum,
                                     s_pendingRenderDistance >= 0 ? s_pendingRenderDistance
                                                                    : PcSettingsRenderDistance(),
                                     true, 7, 1);
            std::swprintf(text, 256, L"Simulation Distance: %d", PcSettingsSimulationDistance());
            m_multiList.AddNewSlider(std::wstring(text), 37, kPcSimulationDistanceMinimum,
                                     kPcSimulationDistanceMaximum, PcSettingsSimulationDistance(), true, 7, 1);
            RenderScaleLabel(PcSettingsRenderScaleStep(), text);
            m_multiList.AddNewSlider(std::wstring(text), 35, 0, kPcRenderScaleSteps - 1, PcSettingsRenderScaleStep(),
                                     false, 7, 3);
#endif
        }
        if (!m_gameStarted
            || (Minecraft::InMiniGame(eMiniGameId_Glide, true)
                && Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial)))
        {
            addSettingCheckbox(m_playerId, L10N::GetString(IDS_MG03_ENABLE_GHOST), 18, eGameSetting_ShowGlideGhost);
            addSettingCheckbox(m_playerId, L10N::GetString(IDS_MG03_SHOW_GHOST_PATH), 19,
                               eGameSetting_ShowGlideGhostPath);
        }
    }
    break;
    case 3:
    {
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_DISPLAY_HUD), 20, eGameSetting_DisplayHUD);
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_DISPLAY_HAND), 21, eGameSetting_DisplayHand);
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_DISPLAY_CHAT_MESSAGES), 22,
                           eGameSetting_DisplayGameMessages);
        if (m_primaryPlayer)
        {
            addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_DISPLAY_SAVE_ICON), 23,
                               eGameSetting_DisplaySaveIcon);
            std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_INTERFACEOPACITY),
                          GameSettings::GetGameSettings(m_playerId, eGameSetting_InterfaceOpacity));
            m_multiList.AddNewSlider(std::wstring(text), 24, 0, 100,
                                     GameSettings::GetGameSettings(m_playerId, eGameSetting_InterfaceOpacity), true, 7,
                                     3);
        }
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_IN_GAME_TOOLTIPS), 25, eGameSetting_Tooltips);
        addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_ANIMATED_CHARACTER), 26,
                           eGameSetting_AnimatedCharacter);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_SENSITIVITY_INMENU),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_InterfaceSensitivity));
        m_multiList.AddNewSlider(std::wstring(text), 27, 0, 200,
                                 GameSettings::GetGameSettings(m_playerId, eGameSetting_InterfaceSensitivity), true, 7,
                                 3);
        bool primaryPlayer = m_primaryPlayer;
        if ((!m_gameStarted
             || (CGameNetworkManager::sInstance.IsHost() && !Minecraft::InMiniGame(eMiniGameId_Any, true)))
            && primaryPlayer)
            addSettingCheckbox(m_playerId, L10N::GetString(IDS_IN_GAME_GAMERTAGS), 28, eGameSetting_DisplayMii);
        if (m_primaryPlayer)
            addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_DISPLAY_SPLITSCREENGAMERTAGS), 29,
                               eGameSetting_SplitscreenNicknames);
        if (gApp.GetGameHostOption(eGameHostOption_MiniGame) == 0)
            addSettingCheckbox(m_playerId, L10N::GetString(IDS_CHECKBOX_CLASSIC_CRAFTING), 30,
                               eGameSetting_ClassicCrafting);
        if (!m_primaryPlayer || gProfile->IsGuest(m_playerId))
        {
            std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_SLIDER_UISIZESPLITSCREEN),
                          GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen) + 1);
            m_multiList.AddNewSlider(std::wstring(text), 32, 1, 3,
                                     GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen) + 1,
                                     false, 7, 3);
        }
        else
        {
            std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_SLIDER_UISIZE),
                          GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSize) + 1);
            m_multiList.AddNewSlider(std::wstring(text), 31, 1, 3,
                                     GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSize) + 1, false, 7, 3);
            std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_SLIDER_UISIZESPLITSCREEN),
                          GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen) + 1);
            m_multiList.AddNewSlider(std::wstring(text), 32, 1, 3,
                                     GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen) + 1,
                                     false, 7, 3);
        }
    }
    break;
#ifdef MINECRAFT_PC
    case 4:
    {
        const bool installed = PcGoldenAgeInstalled();
        for (int group = 0; group < GOLDEN_AGE_GROUP_COUNT; ++group)
        {
            std::wstring label = goldenAgeLabels[group];
            if (!installed)
                label += L" (TU0 files missing)";
            m_multiList.AddNewCheckbox(label, goldenAgeItemIds[group],
                                       (PcSettingsGoldenAge() & goldenAgeGroups[group]) != 0);
            if (!installed)
                m_multiList.EnableItem(goldenAgeItemIds[group], false, true);
        }
        wchar_t text[256];
        TitleLogoLabel(text);
        m_multiList.AddNewSlider(std::wstring(text), TITLE_LOGO_ITEM, 0, kPcTitleLogoCount - 1, PcSettingsTitleLogo(),
                                 false, 7, 3);
        TutorialVersionLabel(text);
        m_multiList.AddNewSlider(std::wstring(text), TUTORIAL_VERSION_ITEM, 0, kPcTutorialVersionCount - 1,
                                 PcSettingsTutorialVersion(), false, 7, 3);
        PanoramaLabel(text);
        m_multiList.AddNewSlider(std::wstring(text), PANORAMA_ITEM, 0, kPcPanoramaCount - 1, PcSettingsPanorama(),
                                 false, 7, 3);
    }
    break;
#endif
    }
    doVerticalResizeCheck();
    doHorizontalResizeCheck();
    HideDescription();
    if (gApp.GetLocalPlayerCount() > 1)
    {
    }
    gConsoleUIController.TouchBoxRebuild(this);
}

void UIScene_SettingsListMenu::setGameSettings()
{
    switch (m_settingsMode)
    {
    case 0:
    {
        int playerId = m_playerId;
        if (m_multiList.CheckElementExists(9))
            GameSettings::SetGameSettings(playerId, eGameSetting_AutoJump, m_multiList.GetCheckboxValue(9));
    }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(1))
                GameSettings::SetGameSettings(playerId, eGameSetting_ViewBob, m_multiList.GetCheckboxValue(1));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(2))
                GameSettings::SetGameSettings(playerId, eGameSetting_FlyingViewRolling,
                                              !m_multiList.GetCheckboxValue(2));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(3))
                GameSettings::SetGameSettings(playerId, eGameSetting_Hints, m_multiList.GetCheckboxValue(3));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(4))
                GameSettings::SetGameSettings(playerId, eGameSetting_DeathMessages, m_multiList.GetCheckboxValue(4));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(0))
                GameSettings::SetGameSettings(playerId, eGameSetting_SplitScreenVertical,
                                              m_multiList.GetCheckboxValue(0));
        }
        break;
    case 1:
    {
        int playerId = m_playerId;
        if (m_multiList.CheckElementExists(12))
            GameSettings::SetGameSettings(playerId, eGameSetting_CaveSounds, m_multiList.GetCheckboxValue(12));
    }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(13))
                GameSettings::SetGameSettings(playerId, eGameSetting_MinecartSounds, m_multiList.GetCheckboxValue(13));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(14))
                GameSettings::SetGameSettings(playerId, eGameSetting_GameChat, m_multiList.GetCheckboxValue(14));
        }
        break;
    case 2:
    {
        int playerId = m_playerId;
        if (m_multiList.CheckElementExists(15))
            GameSettings::SetGameSettings(playerId, eGameSetting_Clouds, m_multiList.GetCheckboxValue(15));
    }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(16))
                GameSettings::SetGameSettings(playerId, eGameSetting_CustomSkinAnim, m_multiList.GetCheckboxValue(16));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(18))
                GameSettings::SetGameSettings(playerId, eGameSetting_ShowGlideGhost, m_multiList.GetCheckboxValue(18));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(19))
                GameSettings::SetGameSettings(playerId, eGameSetting_ShowGlideGhostPath,
                                              m_multiList.GetCheckboxValue(19));
        }
        break;
    case 3:
    {
        int playerId = m_playerId;
        if (m_multiList.CheckElementExists(20))
            GameSettings::SetGameSettings(playerId, eGameSetting_DisplayHUD, m_multiList.GetCheckboxValue(20));
    }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(21))
                GameSettings::SetGameSettings(playerId, eGameSetting_DisplayHand, m_multiList.GetCheckboxValue(21));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(22))
                GameSettings::SetGameSettings(playerId, eGameSetting_DisplayGameMessages,
                                              m_multiList.GetCheckboxValue(22));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(23))
                GameSettings::SetGameSettings(playerId, eGameSetting_DisplaySaveIcon, m_multiList.GetCheckboxValue(23));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(29))
                GameSettings::SetGameSettings(playerId, eGameSetting_SplitscreenNicknames,
                                              m_multiList.GetCheckboxValue(29));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(28))
                GameSettings::SetGameSettings(playerId, eGameSetting_DisplayMii, m_multiList.GetCheckboxValue(28));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(26))
                GameSettings::SetGameSettings(playerId, eGameSetting_AnimatedCharacter,
                                              m_multiList.GetCheckboxValue(26));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(30))
                GameSettings::SetGameSettings(playerId, eGameSetting_ClassicCrafting, m_multiList.GetCheckboxValue(30));
        }
        {
            int playerId = m_playerId;
            if (m_multiList.CheckElementExists(25))
                GameSettings::SetGameSettings(playerId, eGameSetting_Tooltips, m_multiList.GetCheckboxValue(25));
        }
        break;
#ifdef MINECRAFT_PC
    case 4:
        if (PcGoldenAgeInstalled())
        {
            int groups = 0;
            for (int group = 0; group < GOLDEN_AGE_GROUP_COUNT; ++group)
            {
                if (m_multiList.CheckElementExists(goldenAgeItemIds[group])
                    && m_multiList.GetCheckboxValue(goldenAgeItemIds[group]))
                    groups |= goldenAgeGroups[group];
            }
            PcGoldenAgeSetGroups(groups);
        }
        break;
#endif
    }
}

void UIScene_SettingsListMenu::tick()
{
    UIScene::tick();
#ifdef MINECRAFT_PC
    if (m_settingsMode == 1 && PcVoiceTesting() && m_multiList.CheckElementExists(MIC_TEST_ITEM))
    {
        wchar_t text[256];
        MicTestLabel(text);
        m_multiList.SetItemLabel(MIC_TEST_ITEM, UIString(text), false);
    }
#endif
    if (m_saveRequested)
    {
        m_saveRequested = false;
        setGameSettings();
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_LanguageSelector, 0, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
    }
}

void UIScene_SettingsListMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(m_playerId, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void UIScene_SettingsListMenu::handlePreUnloadForReload()
{
    setGameSettings();
}

void UIScene_SettingsListMenu::handleReload()
{
    Init();
}

void UIScene_SettingsListMenu::updateComponents()
{
    bool notInGame = Minecraft::GetInstance()->mLevel == 0;
    if (notInGame)
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
        m_layer->showComponent(m_playerId, EUIScene_Logo, true);
    }
    else
    {
        m_layer->showComponent(m_playerId, EUIScene_Panorama, false);
        m_layer->showComponent(m_playerId, EUIScene_Logo, gApp.GetLocalPlayerCount() == 1);
    }
}

std::wstring UIScene_SettingsListMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"MultilistMenuSplit") : std::wstring(L"MultilistMenu");
}

void UIScene_SettingsListMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
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

void UIScene_SettingsListMenu::handlePress(int, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (childId == 5)
        m_saveRequested = true;
}

void UIScene_SettingsListMenu::handleCheckboxElementToggled(int, int id, bool checked)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
#ifdef MINECRAFT_PC
    int forcedRange = 0;
    if (id == PROXIMITY_CHAT_ITEM && !ProximityForced(&forcedRange) && checked != PcSettingsProximityChat())
    {
        PcSettingsSetProximityChat(checked);
        PcSettingsSave();
    }
    if (id == MIC_TEST_ITEM)
    {
        PcVoiceSetTest(checked);
        wchar_t text[256];
        MicTestLabel(text);
        m_multiList.SetItemLabel(MIC_TEST_ITEM, UIString(text), false);
    }
#else
    (void)id;
    (void)checked;
#endif
}

void UIScene_SettingsListMenu::handleSliderElementMove(int, int id, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    wchar_t text[256];
    switch (id)
    {
    case 6:
        GameSettings::SetGameSettings(m_playerId, eGameSetting_Autosave, value);
        m_multiList.SetItemLabel(6, UIString(m_skinNames[value]), false);
        gApp.SetAutosaveTimerTime();
        break;
    case 7:
        GameSettings::SetGameSettings(m_playerId, eGameSetting_GameSensitivity, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_SENSITIVITY_INGAME),
                      GameSettings::GetGameSettings(m_playerId, eGameSetting_GameSensitivity));
        m_multiList.SetItemLabel(7, UIString(text), false);
        break;
    case 8:
        GameSettings::SetGameSettings(m_playerId, eGameSetting_Difficulty, value);
        m_multiList.SetItemLabel(8, UIString(m_difficultyNames[value]), false);
        break;
    case 10:
        GameSettings::SetGameSettings(m_playerId, eGameSetting_MusicVolume, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_MUSIC), value);
        m_multiList.SetItemLabel(10, UIString(text), false);
        break;
    case 11:
        GameSettings::SetGameSettings(m_playerId, eGameSetting_SoundVolume, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_SOUND), value);
        m_multiList.SetItemLabel(11, UIString(text), false);
        break;
    case 17:
        GameSettings::SetGameSettings(m_playerId, eGameSetting_Gamma, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_GAMMA), value);
        m_multiList.SetItemLabel(17, UIString(text), false);
        break;
#ifdef MINECRAFT_PC
    case 33:
        PcSettingsSetFov(value);
        PcSettingsSave();
        std::swprintf(text, 256, L"FOV: %d", PcSettingsFov());
        m_multiList.SetItemLabel(33, UIString(text), false);
        break;
    case 34:
        PcSettingsSetFramerateStep(value);
        PcSettingsSave();
        FramerateLabel(PcSettingsFramerateStep(), text);
        m_multiList.SetItemLabel(34, UIString(text), false);
        break;
    case 36:
        s_pendingRenderDistance = value;
        RenderDistanceLabel(text);
        m_multiList.SetItemLabel(36, UIString(text), false);
        break;
    case 37:
        PcSettingsSetSimulationDistance(value);
        PcSettingsSave();
        std::swprintf(text, 256, L"Simulation Distance: %d", PcSettingsSimulationDistance());
        m_multiList.SetItemLabel(37, UIString(text), false);
        break;
    case TUTORIAL_VERSION_ITEM:
        PcSettingsSetTutorialVersion(value);
        PcSettingsSave();
        TutorialVersionLabel(text);
        m_multiList.SetItemLabel(TUTORIAL_VERSION_ITEM, UIString(text), false);
        break;
    case PANORAMA_ITEM:
        PcSettingsSetPanorama(value);
        PcSettingsSave();
        PanoramaLabel(text);
        m_multiList.SetItemLabel(PANORAMA_ITEM, UIString(text), false);
        break;
    case MICROPHONE_ITEM:
        PcVoiceSelectDevice(value);
        MicrophoneLabel(value, text);
        m_multiList.SetItemLabel(MICROPHONE_ITEM, UIString(text), false);
        break;
    case PROXIMITY_RANGE_ITEM:
    {
        int forcedRange = 0;
        ProximityForced(&forcedRange);
        if (forcedRange == 0)
        {
            PcSettingsSetProximityRange(value * kPcProximityRangeStep);
            PcSettingsSave();
        }
        ProximityRangeLabel(PcSettingsProximityRange(), text);
        m_multiList.SetItemLabel(PROXIMITY_RANGE_ITEM, UIString(text), false);
        break;
    }
    case VOICE_MODE_ITEM:
        PcSettingsSetVoiceMode(value);
        PcSettingsSave();
        VoiceModeLabel(text);
        m_multiList.SetItemLabel(VOICE_MODE_ITEM, UIString(text), false);
        break;
    case MIC_VOLUME_ITEM:
        PcSettingsSetMicVolume(value);
        PcSettingsSave();
        MicVolumeLabel(text);
        m_multiList.SetItemLabel(MIC_VOLUME_ITEM, UIString(text), false);
        break;
    case VOICE_VOLUME_ITEM:
        PcSettingsSetVoiceVolume(value);
        PcSettingsSave();
        VoiceVolumeLabel(text);
        m_multiList.SetItemLabel(VOICE_VOLUME_ITEM, UIString(text), false);
        break;
    case TITLE_LOGO_ITEM:
        PcSettingsSetTitleLogo(value);
        PcSettingsSave();
        TitleLogoLabel(text);
        m_multiList.SetItemLabel(TITLE_LOGO_ITEM, UIString(text), false);
        break;
    case 35:
        PcSettingsSetRenderScaleStep(value);
        PcSettingsSave();
        RenderScaleLabel(PcSettingsRenderScaleStep(), text);
        m_multiList.SetItemLabel(35, UIString(text), false);
        break;
#endif
    case 24:
        GameSettings::SetGameSettings(m_playerId, eGameSetting_InterfaceOpacity, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_INTERFACEOPACITY), value);
        m_multiList.SetItemLabel(24, UIString(text), false);
        break;
    case 31:
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_SLIDER_UISIZE), value);
        m_multiList.SetItemLabel(31, UIString(text), false);
        if (value != GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSize) + 1)
        {
            GameSettings::SetGameSettings(m_playerId, eGameSetting_HUDSize, value - 1);
            gConsoleUIController.UpdateSelectedItemPos(m_playerId);
        }
        break;
    case 32:
        std::swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_SLIDER_UISIZESPLITSCREEN), value);
        m_multiList.SetItemLabel(32, UIString(text), false);
        if (value != GameSettings::GetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen) + 1)
        {
            GameSettings::SetGameSettings(m_playerId, eGameSetting_HUDSizeSplitscreen, value - 1);
            gConsoleUIController.UpdateSelectedItemPos(m_playerId);
        }
        break;
    case 27:
        GameSettings::SetGameSettings(m_playerId, eGameSetting_InterfaceSensitivity, value);
        std::swprintf(text, 256, L"%ls: %d%%", L10N::GetString(IDS_SLIDER_SENSITIVITY_INMENU), value);
        m_multiList.SetItemLabel(27, UIString(text), false);
        break;
    }
}

int UIScene_SettingsListMenu::getSceneType()
{
    return EUIScene_SettingsListMenu;
}
