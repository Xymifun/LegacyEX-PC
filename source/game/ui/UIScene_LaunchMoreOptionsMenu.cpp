#include "game/ui/UIScene_LaunchMoreOptionsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/L10N.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_LaunchMoreOptionsMenu.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/SuperflatMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/GameSettings.hpp"

extern ConsoleUIController gConsoleUIController;

static int descriptionIds[] = {IDS_GAMEOPTION_ONLINE,
#ifdef MINECRAFT_PC
                               ePcString_FriendsOnlyDescription,
#else
                               IDS_GAMEOPTION_INVITEONLY,
#endif
                               IDS_GAMEOPTION_ALLOWFOF,
                               IDS_GAMEOPTION_PVP,
                               IDS_GAMEOPTION_TRUST,
                               IDS_GAMEOPTION_FIRE_SPREADS,
                               IDS_GAMEOPTION_TNT_EXPLODES,
                               IDS_GAMEOPTION_HOST_PRIVILEGES,
                               IDS_GAMEOPTION_RESET_NETHER,
                               0xcb6d8de1,
                               IDS_GAMEOPTION_STRUCTURES,
                               IDS_GAMEOPTION_BONUS_CHEST,
                               IDS_GAMEOPTION_KEEP_INVENTORY,
                               IDS_GAMEOPTION_MOB_SPAWNING,
                               IDS_GAMEOPTION_MOB_LOOT,
                               IDS_GAMEOPTION_MOB_GRIEFING,
                               IDS_GAMEOPTION_TILE_DROPS,
                               IDS_GAMEOPTION_NATURAL_REGEN,
                               IDS_GAMEOPTION_DAYLIGHT_CYCLE,
                               IDS_GAMEOPTION_WEATHER_CYCLE,
                               IDS_GAMEOPTION_SUPERFLAT,
                               IDS_SUPERFLAT_CUSTOMISE_DESCRIPTION,
                               IDS_GAMEOPTION_SEED,
                               0x8f88ae9a,
                               IDS_DESC_BIOME_SCALE,
                               0x814065bc,
                               IDS_MG_OPTION_DESC_CENTRE_SPAWN,
                               IDS_MG_OPTION_DESC_MAP_SELECT,
                               IDS_MG_OPTION_DESC_ROUND_LENGTH,
                               IDS_MASHUP_CHOOSE_BETWEEN_SEVERAL_VARIANTS_SAME_CORE_GAME_TEXT,
                               IDS_MG_OPTION_DESC_SPECTATE_MODE,
                               0xbe68971f,
                               0,
                               0,
                               IDS_MG_LIVES_PER_ROUND_DESC,
                               IDS_MG01_ROUND_COUNT_DESC,
                               IDS_MG01_MAP_SIZE_DESC,
                               IDS_MG01_NATURAL_REGENERATION_DESC,
                               0xf7c16b2c,
                               IDS_MG01_SMALL_INVENTORY_DESC,
                               IDS_MG01_TAKE_EVERYTHING_DESC,
                               IDS_MG01_CHEST_REFILL_SETTING_DESC,
                               IDS_MG02_SPECTATOR_PARTICIPATION_DESC,
                               IDS_MG02_LAYER_SCALE_DESC,
                               IDS_MG02_LAYER_COUNT_DESC};

#ifdef MINECRAFT_PC
static const int WORLD_SIZE_ITEM = 0x36;
static const int WORLD_SIZE_DESC = 0x7bd1534e;
static const int PUBLIC_TITLE_ITEM = 0x37;
static const int worldSizeLabels[] = {static_cast<int>(0xa978e0a8), static_cast<int>(IDS_SMALL),
                                       static_cast<int>(IDS_MEDIUM), static_cast<int>(IDS_LARGE),
                                       static_cast<int>(ePcString_Massive)};

static int WorldSizeSliderValue(int hostWorldSize)
{
    return hostWorldSize < 1 || hostWorldSize > 5 ? 0 : hostWorldSize - 1;
}
#endif

static float arenaSizeTable[] = {0.5f, 0.625f, 0.75f, 0.875f, 1.0f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f};

UIScene_LaunchMoreOptionsMenu::UIScene_LaunchMoreOptionsMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer),
      m_worldSize(),
      m_worldType(),
      m_gameMode(),
      m_lootSet(),
      m_foodBalance(),
      m_mapSize(),
      m_roundLength(),
      m_worldTab(),
      m_gameTab(),
      m_unmappedControlA(),
      m_unmappedControlB(),
      m_worldOptions(),
      m_gameOptions(),
      m_worldOptionsList(),
      m_gameOptionsList(),
      m_worldLabel(),
      m_gameLabel(),
      m_unmappedLabelA(),
      m_gameDescription(),
      m_worldDescription(),
      m_unmappedLabelB(),
      m_unmappedLabelC(),
      m_worldSizeLabel(),
      m_resizeLabel(),
      m_seed()
{
    initialiseMovie();
    m_initData = static_cast<LaunchMoreOptionsMenuInitData*>(data);
    bool online = false;
    if (gProfile->IsSignedInLive(m_initData->mPad) && gProfile->AllowedToPlayMultiplayer(m_initData->mPad))
        online = true;
    m_online = online;
    m_updateCheats = false;
    m_menuType = static_cast<int>(m_initData->mNewWorld);
    _init();
    addTimer(0, 100);
    m_requestKeyboard = false;
    m_keyboardActive = false;
}

UIScene_LaunchMoreOptionsMenu::~UIScene_LaunchMoreOptionsMenu()
{
}

void UIScene_LaunchMoreOptionsMenu::EnableCustomOptions(bool enabled)
{
    if (m_initData->mMiniGameId != eMiniGameId_Battle && m_initData->mMiniGameId != eMiniGameId_Tumble)
        return;
    if (m_initData->mHostMiniGame)
    {
        m_gameOptionsList.EnableItem(0x22, enabled, true);
        m_gameOptionsList.EnableItem(0x1e, enabled, true);
    }
    if (m_initData->mMiniGameId == eMiniGameId_Battle)
    {
        if (!m_initData->mJoinMiniGame && m_initData->mHostMiniGame)
        {
            m_gameOptionsList.EnableItem(0x1f, enabled, true);
            m_gameOptionsList.EnableItem(0x20, enabled, true);
            m_gameOptionsList.EnableItem(0x21, enabled, true);
            m_gameOptionsList.EnableItem(0x23, enabled, true);
            m_gameOptionsList.EnableItem(0x24, enabled, true);
            m_gameOptionsList.EnableItem(0x25, enabled, true);
            m_gameOptionsList.EnableItem(0x27, enabled, true);
            m_gameOptionsList.EnableItem(0x28, enabled, true);
            m_gameOptionsList.EnableItem(0x29, enabled, true);
            m_gameOptionsList.SetCheckboxValue(
                0x1f, m_initData->mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_NoSmallSkins) == 0, true);
            m_gameOptionsList.SetCheckboxValue(0x25, m_initData->mPlayerSettings.mNaturalRegeneration, true);
            m_gameOptionsList.SetCheckboxValue(0x27, m_initData->mPlayerSettings.mSmallInventoryEnabled, true);
            m_gameOptionsList.SetCheckboxValue(0x28, m_initData->mPlayerSettings.mTakeEverythingEnabled, true);
            m_gameOptionsList.SetCheckboxValue(0x29, m_initData->mPlayerSettings.mChestRefillEnabled, true);
        }
    }
    else if (m_initData->mMiniGameId == eMiniGameId_Tumble && !m_initData->mJoinMiniGame && m_initData->mHostMiniGame)
    {
        m_gameOptionsList.EnableItem(0x20, enabled, true);
        m_gameOptionsList.EnableItem(0x2b, enabled, true);
        m_gameOptionsList.EnableItem(0x2c, enabled, true);
        m_gameOptionsList.EnableItem(0x2a, enabled, true);
        m_gameOptionsList.SetCheckboxValue(0x2a, m_initData->mPlayerSettings.mSpectatorParticipation, true);
    }
}

void UIScene_LaunchMoreOptionsMenu::UpdateCheats()
{
    bool cheatsEnabled = m_initData->mHostPrivileges;
    if (!m_initData->mHostMiniGame && !m_initData->mJoinMiniGame)
    {
        m_gameOptionsList.EnableItem(0xc, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0xd, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0xf, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0x12, cheatsEnabled, true);
        m_gameOptionsList.EnableItem(0x13, cheatsEnabled, true);
    }
    if (!cheatsEnabled)
    {
        m_initData->mMobGriefing = true;
        m_initData->mDoMobSpawning = true;
        m_initData->mKeepInventory = false;
        m_initData->mDoDaylightCycle = true;
        m_initData->mDoWeatherCycle = true;
        if (!m_initData->mHostMiniGame && !m_initData->mJoinMiniGame)
        {
            m_gameOptionsList.SetCheckboxValue(0xc, false, true);
            m_gameOptionsList.SetCheckboxValue(0xd, m_initData->mDoMobSpawning, true);
            m_gameOptionsList.SetCheckboxValue(0xf, m_initData->mMobGriefing, true);
            m_gameOptionsList.SetCheckboxValue(0x12, m_initData->mDoDaylightCycle, true);
            m_gameOptionsList.SetCheckboxValue(0x13, m_initData->mDoWeatherCycle, true);
        }
    }
}

void UIScene_LaunchMoreOptionsMenu::UpdateDescriptionText()
{
    if (m_focusedItemId == -1)
        return;

#ifdef MINECRAFT_PC
    int descriptionId;
    if (m_focusedItemId == WORLD_SIZE_ITEM)
        descriptionId = WORLD_SIZE_DESC;
    else if (m_focusedItemId == PUBLIC_TITLE_ITEM)
        descriptionId = ePcString_PublicWorldTitleDescription;
    else
        descriptionId = descriptionIds[m_focusedItemId];
#else
    int descriptionId = descriptionIds[m_focusedItemId];
#endif
    std::wstring description(L"");
    bool hiDef;
    if (descriptionId == -1 || descriptionId == -2)
    {
        hiDef = gRenderer->IsHiDef();
    }
    else
    {
        description = gApp.FormatHTMLString(m_playerId, std::wstring(L10N::GetString(descriptionId)), -1);
        if (descriptionId == static_cast<int>(IDS_GAMEOPTION_SUPERFLAT))
            description = replaceAll(description, std::wstring(L"{*DISABLES_ACHIEVEMENTS*}"),
                                     std::wstring(L10N::GetString(IDS_HOST_OPTION_DISABLES_ACHIEVEMENTS)));
        hiDef = gRenderer->IsHiDef();
    }
    if (!hiDef)
        gRenderer->IsWidescreen();
    if (m_focusedItemId == 0x15)
    {
        if (!m_initData->mFlatWorld)
            description = L10N::GetString(0xaa2e57af);
        else
            description += L"\n\n" + FlatGeneratorInfo(&m_initData->mSuperflatConfig).generateDescription();
    }
    if (m_focusedItemId == 0x20)
        description = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)
                          ->GetLootSetDescription(m_gameOptionsList.GetSliderValue(0x20));
    else if (m_focusedItemId == 0x21)
        description = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)
                          ->GetFoodBalanceIndexDescription(m_gameOptionsList.GetSliderValue(0x21));
    wchar_t prefix[64];
    swprintf(prefix, 64, L"<font color=\"#%08x\">", gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White));
    description = prefix + description;
    if (!m_initData->mHostMiniGame && !m_initData->mJoinMiniGame && m_menuType == 1)
        m_worldDescription.setLabel(UIString(description), false, false);
    else
        m_gameDescription.setLabel(UIString(description), false, false);
    if (m_initData->mNewWorld)
        m_worldOptionsList.SetItemLabel(0x16, UIString(m_initData->mSeed), false);
}
void UIScene_LaunchMoreOptionsMenu::_init()
{
    m_worldLabel.init(UIString(L10N::GetString(IDS_WORLD_OPTIONS)));
    m_worldOptions.setId(0x32);
    m_focusedControlId = -1;
    m_gameOptions.setId(0x33);

    const bool onlineGame = m_initData->mOnlineGame;
    const bool allowFriendsOfFriends = m_initData->mAllowFriendsOfFriends;

    if (!m_initData->mJoinMiniGame)
    {
        if (m_initData->mHostMiniGame)
        {
            m_focusedItemId = 0;
            m_gameOptionsList.AddNewLabel(std::wstring(L10N::GetString(IDS_GAME_OPTIONS)), 0x34);

            if (MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)->HasGameVariants())
            {
                int variant = m_initData->mMatchmakingFilters[1];
                m_foodBalance = SliderLogic(0x1d, IDS_MG_OPTION_GAME_VARIANT, eCustomGameModeHostOption_ModeVariant,
                                            m_initData->mMiniGameId, true);
                m_foodBalance.AddTo(m_gameOptionsList, variant);
                variant = variant - 1;
                if (variant < 0)
                    variant = 0;
                m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, variant);
            }
            if (MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)->HasMultipleMaps()
                && m_initData->mMiniGameId != eMiniGameId_Tumble)
            {
                m_gameOptionsList.AddNewButton(std::wstring(L10N::GetString(IDS_MG_SELECT_MAPS)), 0x1b, true);
                m_gameOptionsList.EnableItem(0x1b, true, true);
            }
            if (MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)->HasRoundLengthOption())
            {
                m_lootSet = SliderLogic(0x1c, IDS_MG_OPTION_ROUND_LENGTH, eCustomGameModeHostOption_MapVariant,
                                        m_initData->mMiniGameId, true);
                m_lootSet.AddTo(m_gameOptionsList, m_initData->mMatchmakingFilters[0]);
            }
            if (MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)->HasCentralSpawnOption())
                m_gameOptionsList.AddNewCheckbox(
                    std::wstring(L10N::GetString(IDS_MG_OPTION_CENTRE_SPAWN)), 0x1a,
                    m_initData->mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_CentreSpawn) != 0);

            if (m_initData->mMiniGameId == eMiniGameId_Battle || m_initData->mMiniGameId == eMiniGameId_Tumble)
            {
                wchar_t text[256];
                m_gameOptionsList.AddNewLabel(std::wstring(L" "), 0x34);
                m_gameOptionsList.AddNewLabel(std::wstring(L10N::GetString(IDS_MG_CUSTOM_OPTIONS)), 0x34);
                if (m_initData->mPlayerSettings.mLivesPerRound == 0)
                    swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG_LIVES_PER_ROUND),
                             L10N::GetString(IDS_MG_INFINITE));
                else
                    swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG_LIVES_PER_ROUND),
                             m_initData->mPlayerSettings.mLivesPerRound);
                m_gameOptionsList.AddNewSlider(std::wstring(text), 0x22,
                                               !MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)
                                                    ->HasFeatureSupportForInfiniteLives(),
                                               10, m_initData->mPlayerSettings.mLivesPerRound, false, 7, 3);
                m_worldType = SliderLogic(0x1e, IDS_MG_OPTION_SPECTATE_MODE, eCustomGameModeHostOption_SpectateMode,
                                          m_initData->mMiniGameId, false);
                m_worldType.AddTo(m_gameOptionsList,
                                  m_initData->mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_SpectateMode));
            }

            if (m_initData->mMiniGameId == eMiniGameId_Battle)
            {
                wchar_t text[256];
                m_gameOptionsList.AddNewCheckbox(
                    std::wstring(L10N::GetString(IDS_MG_OPTION_ALLOW_ALL_SKINS)), 0x1f,
                    m_initData->mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_NoSmallSkins) == 0);
                std::wstring label = std::wstring(L10N::GetString(IDS_MG01_ITEMSET)) + L": "
                                     + MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)
                                           ->GetLootSetText(m_initData->mPlayerSettings.mLootSetId);
                m_gameOptionsList.AddNewSlider(label, 0x20, 0, 5, m_initData->mPlayerSettings.mLootSetId, false, 7, 3);
                label = std::wstring(L10N::GetString(0x1cbfab4a)) + L": "
                        + MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)
                              ->GetFoodBalanceIndexText(m_initData->mPlayerSettings.mFoodBalance);
                m_gameOptionsList.AddNewSlider(label, 0x21, 0, 7, m_initData->mPlayerSettings.mFoodBalance, false, 7,
                                               3);
                swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG01_ROUND_COUNT),
                         m_initData->mPlayerSettings.mRoundCount);
                m_gameOptionsList.AddNewSlider(std::wstring(text), 0x23, 1, 10, m_initData->mPlayerSettings.mRoundCount,
                                               false, 7, 3);
                int mapSizeText = 0;
                if (m_initData->mPlayerSettings.mMapSizeOverride.isSet())
                    mapSizeText = m_initData->mPlayerSettings.mMapSizeOverride.get();
                label = std::wstring(L10N::GetString(IDS_MG01_MAP_SIZE)) + L": "
                        + MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)
                              ->GetMapSizeOverrideText(mapSizeText);
                int mapSizeValue = 0;
                if (m_initData->mPlayerSettings.mMapSizeOverride.isSet())
                    mapSizeValue = m_initData->mPlayerSettings.mMapSizeOverride.get();
                m_gameOptionsList.AddNewSlider(label, 0x24, 0, 2, mapSizeValue, false, 7, 3);
                m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG01_NATURAL_REGENERATION)), 0x25,
                                                 m_initData->mPlayerSettings.mNaturalRegeneration);
                m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG01_SMALL_INVENTORY)), 0x27,
                                                 m_initData->mPlayerSettings.mSmallInventoryEnabled);
                m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG01_TAKE_EVERYTHING)), 0x28,
                                                 m_initData->mPlayerSettings.mTakeEverythingEnabled);
                m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG01_CHEST_REFILL_SETTING)), 0x29,
                                                 m_initData->mPlayerSettings.mChestRefillEnabled);
            }
            else if (m_initData->mMiniGameId == eMiniGameId_Tumble)
            {
                wchar_t text[256];
                std::wstring label = std::wstring(L10N::GetString(IDS_MG01_ITEMSET)) + L": "
                                     + MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)
                                           ->GetLootSetText(m_initData->mPlayerSettings.mLootSetId);
                m_gameOptionsList.AddNewSlider(label, 0x20, 0, 3, m_initData->mPlayerSettings.mLootSetId, false, 7, 3);
                int arenaSize = 0;
                if (!m_initData->mPlayerSettings.mArenaSizeMultiplier.isSet())
                {
                    swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG02_LAYER_SCALE),
                             L10N::GetString(IDS_MG_AUTO));
                    m_gameOptionsList.AddNewSlider(std::wstring(text), 0x2b, 0, 5, 0, true, 7, 3);
                }
                else
                {
                    float multiplier = m_initData->mPlayerSettings.mArenaSizeMultiplier.get();
                    for (int index = 0; index < 5; ++index)
                    {
                        if (multiplier == arenaSizeTable[index])
                        {
                            arenaSize = index + 1;
                            break;
                        }
                    }
                    std::wstring display = _toString<float>(arenaSizeTable[arenaSize + 4]);
                    swprintf(text, 256, L"%ls: x%ls", L10N::GetString(IDS_MG02_LAYER_SCALE), display.c_str());
                    m_gameOptionsList.AddNewSlider(std::wstring(text), 0x2b, 0, 5, arenaSize, true, 7, 3);
                }
                int layerCount = 0;
                if (m_initData->mPlayerSettings.mLayerCount.isSet())
                    layerCount = m_initData->mPlayerSettings.mLayerCount.get();
                if (layerCount < 1)
                    swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG02_LAYER_COUNT),
                             L10N::GetString(IDS_MG_AUTO));
                else
                    swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG02_LAYER_COUNT), layerCount);
                m_gameOptionsList.AddNewSlider(std::wstring(text), 0x2c, 0, 8, layerCount, false, 7, 3);
                m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MG02_SPECTATOR_PARTICIPATION)), 0x2a,
                                                 m_initData->mPlayerSettings.mSpectatorParticipation);
            }
            EnableCustomOptions(MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings));
        }
        else
        {
            if (!m_initData->mNewWorld)
            {
                m_focusedItemId = 0;
                m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_RESET_NETHER)), 8,
                                                  m_initData->mResetNether);
                m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_RESET_THE_END)), 9,
                                                  m_initData->mResetEnd);
            }
            else
            {
                static int biomeLabels[] = {IDS_SMALL, IDS_MEDIUM, IDS_LARGE};
                m_focusedItemId = 0x16;
                m_worldOptionsList.AddNewLabel(std::wstring(L10N::GetString(IDS_CREATE_NEW_WORLD_SEED)), 0x34);
                m_worldOptionsList.AddNewTextInput(m_initData->mSeed, 0x16, 0x3c);
                m_worldOptionsList.AddNewLabel(std::wstring(L10N::GetString(0xae4164b2)), 0x34);
                m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_FIND_BALANCED)), 0x17,
                                                  m_initData->mFindBalancedSeed);
                m_worldOptionsList.AddNewLabel(std::wstring(L" "), 0x34);
#ifdef MINECRAFT_PC
                m_worldOptionsList.AddNewLabel(std::wstring(L10N::GetString(IDS_WORLD_SIZE)), 0x34);
                m_worldOptionsList.AddNewSlider(
                    std::wstring(L10N::GetString(worldSizeLabels[WorldSizeSliderValue(m_initData->mWorldSize)])),
                    WORLD_SIZE_ITEM, 0, 4, WorldSizeSliderValue(m_initData->mWorldSize), false, 7, 3);
#endif
                m_worldOptionsList.AddNewLabel(std::wstring(L10N::GetString(IDS_BIOME_SCALE)), 0x34);
                m_worldOptionsList.AddNewSlider(std::wstring(L10N::GetString(biomeLabels[m_initData->mBiomeScale])),
                                                0x18, 0, 2, m_initData->mBiomeScale, false, 7, 3);
                m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_GENERATE_STRUCTURES)), 10,
                                                  m_initData->mStructures);
                m_worldOptionsList.EnableItem(10, !m_initData->mFlatWorld, true);
                m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_BONUS_CHEST)), 0xb,
                                                  m_initData->mBonusChest);
                m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_SUPERFLAT_WORLD)), 0x14,
                                                  m_initData->mFlatWorld);
                m_worldOptionsList.EnableItem(0x14, !m_initData->mWorldTypeLocked, true);
                m_worldOptionsList.AddNewButton(std::wstring(L10N::GetString(IDS_SUPERFLAT_CUSTOMISE)), 0x15, true);
                m_worldOptionsList.EnableItem(0x15, m_initData->mFlatWorld && !m_initData->mWorldTypeLocked, true);
                m_worldOptionsList.AddNewLabel(std::wstring(L" "), 0x34);
            }
            m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TRUST_PLAYERS)), 4,
                                              m_initData->mTrustPlayers);
            m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_FIRE_SPREADS)), 5,
                                              m_initData->mFireSpreads);
            m_worldOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TNT_EXPLODES)), 6, m_initData->mTNT);

            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_ONLINE_GAME)), 0, onlineGame);
#ifdef MINECRAFT_PC
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(ePcString_FriendsOnly)), 1,
                                             m_initData->mInviteOnly);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(ePcString_PublicWorldTitle)),
                                             PUBLIC_TITLE_ITEM, m_initData->mPublicWorldTitle);
#endif
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_ALLOWFRIENDSOFFRIENDS)), 2,
                                             allowFriendsOfFriends);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_PLAYER_VS_PLAYER)), 3, m_initData->mPvP);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_HOST_PRIVILEGES)), 7,
                                             m_initData->mHostPrivileges);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_DAYLIGHT_CYCLE)), 0x12,
                                             m_initData->mDoDaylightCycle);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_WEATHER_CYCLE)), 0x13,
                                             m_initData->mDoWeatherCycle);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_KEEP_INVENTORY)), 0xc,
                                             m_initData->mKeepInventory);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_SPAWNING)), 0xd,
                                             m_initData->mDoMobSpawning);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_GRIEFING)), 0xf,
                                             m_initData->mMobGriefing);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_MOB_LOOT)), 0xe, m_initData->mDoMobLoot);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_TILE_DROPS)), 0x10,
                                             m_initData->mDoTileDrops);
            m_gameOptionsList.AddNewCheckbox(std::wstring(L10N::GetString(IDS_NATURAL_REGEN)), 0x11,
                                             m_initData->mNaturalRegeneration);
        }
    }
    else
    {
        std::wstring label(L10N::GetString(IDS_GAME_OPTIONS));
        m_focusedItemId = 0;
        m_gameOptionsList.AddNewLabel(label, 0x34);
        label = L" ";
        m_gameOptionsList.AddNewLabel(label, 0x34);
        label = L" ";
        m_gameOptionsList.AddNewLabel(label, 0x34);
        if (MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)->HasGameVariants())
        {
            int variant = m_initData->mMatchmakingFilters[1];
            m_foodBalance = SliderLogic(0x1d, IDS_MG_OPTION_GAME_VARIANT, eCustomGameModeHostOption_ModeVariant,
                                        m_initData->mMiniGameId, false);
            m_foodBalance.AddTo(m_gameOptionsList, variant);
            variant = variant - 1;
            if (variant < 0)
                variant = 0;
            m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, variant);
        }
        if (MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true)->HasRoundLengthOption())
        {
            m_lootSet = SliderLogic(0x1c, IDS_MG_OPTION_ROUND_LENGTH, eCustomGameModeHostOption_MapVariant,
                                    m_initData->mMiniGameId, false);
            m_lootSet.AddTo(m_gameOptionsList, m_initData->mMatchmakingFilters[0]);
        }
    }

    if (!m_initData->mJoinMiniGame && (!m_initData->mHostMiniGame || !m_initData->mMatchmaking))
    {
        if (!gProfile->IsSignedInLive(m_initData->mPad))
            m_gameOptionsList.EnableItem(0, false, true);
        if (!m_initData->mOnlineSettingChangedBySystem || m_online)
        {
#ifdef MINECRAFT_PC
            if (!m_initData->mOnlineGame)
            {
                m_gameOptionsList.EnableItem(1, false, true);
                m_gameOptionsList.EnableItem(PUBLIC_TITLE_ITEM, false, true);
            }
            if (!m_initData->mOnlineGame || !m_initData->mInviteOnly)
                m_gameOptionsList.EnableItem(2, false, true);
#else
            if (!m_initData->mOnlineGame)
                m_gameOptionsList.EnableItem(2, false, true);
#endif
        }
        else
        {
            m_gameOptionsList.EnableItem(0, false, true);
#ifdef MINECRAFT_PC
            m_gameOptionsList.EnableItem(1, false, true);
            m_gameOptionsList.EnableItem(PUBLIC_TITLE_ITEM, false, true);
#endif
            m_gameOptionsList.EnableItem(2, false, true);
        }
    }

    m_gameLabel.init(UIString(L10N::GetString(IDS_GAME_OPTIONS)));
    FJ_LaunchMoreOptionsMenu* menu = static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode());
    menu->SetMenuType(!m_initData->mNewWorld);
    if (m_initData->mHostMiniGame || m_initData->mJoinMiniGame)
        static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->HideWorldTab();

    UpdateCheats();
    UpdateDescriptionText();
    m_worldTab.init(0x34);
    m_gameTab.init(0x35);
    gConsoleUIController.TouchBoxRebuild(this);
}

unsigned int UIScene_LaunchMoreOptionsMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_gameLabel.setupControl(this, currentNode, std::string("LabelGame"));
        m_controls.push_back(&m_gameLabel);
    }
    {
        m_worldLabel.setupControl(this, currentNode, std::string("LabelWorld"));
        m_controls.push_back(&m_worldLabel);
    }
    {
        m_worldOptions.setupControl(this, currentNode, std::string("WorldOptions"));
        m_controls.push_back(&m_worldOptions);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_worldOptions.getFuiRenderNode();
        {
            m_worldOptionsList.setupControl(this, currentNode, std::string("WorldOptionsList"));
            m_controls.push_back(&m_worldOptionsList);
        }
        {
            m_worldTab.setupControl(this, currentNode, std::string("TouchTabWorld"));
            m_controls.push_back(&m_worldTab);
        }
        {
            m_worldDescription.setupControl(this, currentNode, std::string("Description_WorldOptions"));
            m_controls.push_back(&m_worldDescription);
        }
        {
            m_worldSizeLabel.setupControl(this, currentNode, std::string("WorldSize"));
            m_controls.push_back(&m_worldSizeLabel);
        }
        {
            m_resizeLabel.setupControl(this, currentNode, std::string("ResizeLabel"));
            m_controls.push_back(&m_resizeLabel);
        }
        currentNode = parentNode;
    }
    {
        m_gameOptions.setupControl(this, currentNode, std::string("GameOptions"));
        m_controls.push_back(&m_gameOptions);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_gameOptions.getFuiRenderNode();
        {
            m_gameOptionsList.setupControl(this, currentNode, std::string("GameOptionsList"));
            m_controls.push_back(&m_gameOptionsList);
        }
        {
            m_gameTab.setupControl(this, currentNode, std::string("TouchTabGame"));
            m_controls.push_back(&m_gameTab);
        }
        {
            m_gameDescription.setupControl(this, currentNode, std::string("Description_GameOptions"));
            m_controls.push_back(&m_gameDescription);
        }
        currentNode = parentNode;
    }
    return 1;
}

void UIScene_LaunchMoreOptionsMenu::updateTooltips()
{
    if (m_initData->mHostMiniGame || m_initData->mJoinMiniGame)
    {
        gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, -1, -1, false, false);
    }
    else
    {
        int tabLabel = m_menuType == 0 ? IDS_WORLD_OPTIONS : IDS_GAME_OPTIONS;
        gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -2, tabLabel, -1,
                                         -1, -1, -1, -1, -1, -1, -1, false, false);
    }
}

void UIScene_LaunchMoreOptionsMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

std::wstring UIScene_LaunchMoreOptionsMenu::getMoviePath()
{
    return std::wstring(L"LaunchMoreOptionsMenu");
}

int UIScene_LaunchMoreOptionsMenu::KeyboardCompleteSeedCallback(void* data, bool accepted)
{
    UIScene_LaunchMoreOptionsMenu* scene = static_cast<UIScene_LaunchMoreOptionsMenu*>(data);
    scene->m_keyboardActive = false;
    if (accepted)
    {
        wchar_t text[128];
        std::memset(text, 0, sizeof(text));
        g_pInput->GetText(text, 0x7f);
        scene->m_worldOptionsList.SetItemLabel(0x16, UIString(text), false);
        scene->m_initData->mSeed.assign(text);
        scene->m_initData->mSettingsChanged = true;
    }
    return 0;
}

void UIScene_LaunchMoreOptionsMenu::HandleRequestKeyboard()
{
    m_keyboardActive = true;
    m_seed.assign(m_initData->mSeed);
    if (m_seed == L" ")
        m_seed.assign(L"");
    g_pInput->RequestKeyboard(L10N::GetString(IDS_CREATE_NEW_WORLD_SEED), m_seed.c_str(), 0, 0x3c,
                              KeyboardCompleteSeedCallback, this, C_4JInput::EKeyboardMode_0);
}

void UIScene_LaunchMoreOptionsMenu::UpdateOnline()
{
    if (m_initData->mHostMiniGame && m_initData->mMatchmaking)
        return;
    if (m_initData->mJoinMiniGame)
        return;
#ifdef MINECRAFT_PC
    m_gameOptionsList.EnableItem(1, m_initData->mOnlineGame, true);
    m_gameOptionsList.EnableItem(PUBLIC_TITLE_ITEM, m_initData->mOnlineGame, true);
    m_gameOptionsList.EnableItem(2, m_initData->mOnlineGame && m_initData->mInviteOnly, true);
#else
    m_gameOptionsList.EnableItem(2, m_initData->mOnlineGame, true);
#endif
}

void UIScene_LaunchMoreOptionsMenu::tick()
{
    UIScene::tick();
    bool online = gProfile->IsSignedInLive(m_initData->mPad) && gProfile->AllowedToPlayMultiplayer(m_initData->mPad);
    if (online != m_online)
    {
        m_gameOptionsList.EnableItem(0, online, true);
        m_gameOptionsList.EnableItem(2, online, true);
        if (online)
        {
            m_gameOptionsList.SetCheckboxValue(0, true, true);
            m_gameOptionsList.SetCheckboxValue(2, true, true);
        }
        m_online = online;
    }
    if (m_requestKeyboard)
    {
        HandleRequestKeyboard();
        m_requestKeyboard = false;
    }
    if (m_updateCheats)
    {
        UpdateCheats();
        m_updateCheats = false;
    }
    if (m_updateOnline)
    {
        UpdateOnline();
        m_updateOnline = false;
    }
}

void UIScene_LaunchMoreOptionsMenu::handleDestroy()
{
    g_pInput->DestroyKeyboard();
}

void UIScene_LaunchMoreOptionsMenu::handleReload()
{
    int savedItemId = m_focusedItemId;
    _init();
    static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->SetActiveTab(m_menuType, false);
    m_focusedItemId = savedItemId;
    if (m_menuType == 0)
    {
        UIControl_List* list = &m_gameOptionsList;
        int selection = list->getListIndex(savedItemId);
        list->setCurrentSelection(selection, false, true);
    }
    else
    {
        UIControl_List* list = &m_worldOptionsList;
        int selection = list->getListIndex(savedItemId);
        list->setCurrentSelection(selection, false, true);
    }
}

void UIScene_LaunchMoreOptionsMenu::handleInput(int, int action, bool repeat, bool pressed, bool released,
                                                bool& handled)
{
    if (!m_keyboardActive)
    {
        gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
        switch (action)
        {
        case EControllerActions_MenuCancel:
            if (pressed)
            {
                if (m_initData->mHostMiniGame || m_initData->mJoinMiniGame)
                    MasterGameMode::SaveDefaults(*m_initData);
                navigateBack();
                handled = true;
                return;
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
            handled = true;
            return;
        case EControllerActions_MenuRightScroll:
        case EControllerActions_MenuLeftScroll:
            if (pressed)
            {
                gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
                m_menuType = m_menuType == 0;
                updateTooltips();
                static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->ChangeTab();
                gConsoleUIController.TouchBoxRebuild(this);
            }
            break;
        }
    }
}

void UIScene_LaunchMoreOptionsMenu::handleFocusChange(int focus, int childFocus)
{
    m_focusedControlId = focus;
    m_focusedItemId = childFocus;
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleTimerComplete(int)
{
}

void UIScene_LaunchMoreOptionsMenu::handlePress(int controlId, int childId)
{
    if (!m_keyboardActive)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        if (controlId == 0x16)
        {
            m_requestKeyboard = true;
            return;
        }
        switch (childId)
        {
        case 0x15:
            gConsoleUIController.NavigateToScene(m_playerId, EUIScene_SuperFlatMenu, m_initData, EUILayer_Scene,
                                                 EUIGroup_PlayerDefault);
            return;
        case 0x19:
            gConsoleUIController.NavigateToScene(m_playerId, EUIScene_BiomePreviewMenu, m_initData, EUILayer_Scene,
                                                 EUIGroup_PlayerDefault);
            return;
        case 0x1b:
            gConsoleUIController.NavigateToScene(m_playerId, EUIScene_MapSelectMenu, m_initData, EUILayer_Scene,
                                                 EUIGroup_PlayerDefault);
            return;
        case 0x2d:
            break;
        default:
            return;
        }
        if (m_initData->mJoinMiniGame)
        {
            MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true);
            miniGame->SetDefaultMatchmakingSearchData(true, m_initData);
            m_gameOptionsList.SetSliderValue(0x2e, m_initData->mMatchmakingFilters[0], false);
            m_lootSet.Update(m_gameOptionsList, m_initData->mMatchmakingFilters[0]);
            m_gameOptionsList.SetSliderValue(0x2f, m_initData->mMatchmakingFilters[1], false);
            m_foodBalance.Update(m_gameOptionsList, m_initData->mMatchmakingFilters[1]);
            UpdateDescriptionText();
            return;
        }
        if (m_initData->mHostMiniGame && m_initData->mMatchmaking)
        {
            MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true);
            miniGame->SetDefaultMatchmakingSearchData(false, m_initData);
            m_gameOptionsList.SetSliderValue(0x2e, m_initData->mMatchmakingFilters[0], false);
            m_lootSet.Update(m_gameOptionsList, m_initData->mMatchmakingFilters[0]);
            m_gameOptionsList.SetSliderValue(0x2f, m_initData->mMatchmakingFilters[1], false);
            m_foodBalance.Update(m_gameOptionsList, m_initData->mMatchmakingFilters[1]);
            UpdateDescriptionText();
        }
    }
}

void UIScene_LaunchMoreOptionsMenu::handleSliderElementMove(int, int second, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    wchar_t text[256];
    switch (second)
    {
#ifdef MINECRAFT_PC
    case WORLD_SIZE_ITEM:
        m_initData->mWorldSize = value + 1;
        m_worldOptionsList.SetItemLabel(WORLD_SIZE_ITEM, UIString(L10N::GetString(worldSizeLabels[value])), false);
        m_initData->mSettingsChanged = true;
        return;
#endif
    case 0x18:
    {
        static const int biomeLabels[] = {static_cast<int>(IDS_SMALL), static_cast<int>(IDS_MEDIUM),
                                          static_cast<int>(IDS_LARGE)};
        m_initData->mBiomeScale = static_cast<unsigned char>(value);
        m_worldOptionsList.SetItemLabel(0x18, UIString(L10N::GetString(biomeLabels[value])), false);
        m_initData->mSettingsChanged = true;
        return;
    }
    case 0x1c:
        m_initData->mMatchmakingFilters[0] = value;
        m_lootSet.Update(m_gameOptionsList, value);
        m_initData->mSettingsChanged = true;
        return;
    case 0x1d:
    {
        int modeVariant = value - 1;
        if (modeVariant < 0)
            modeVariant = 0;
        m_initData->mMatchmakingFilters[1] = value;
        m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, modeVariant);
        m_foodBalance.Update(m_gameOptionsList, value);
        EnableCustomOptions(MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings));
        m_initData->mSettingsChanged = true;
        return;
    }
    case 0x1e:
        m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_SpectateMode, value);
        m_worldType.Update(m_gameOptionsList, value);
        m_initData->mSettingsChanged = true;
        return;
    case 0x20:
    {
        const int lootSet = m_gameOptionsList.GetSliderValue(0x20);
        m_initData->mPlayerSettings.mLootSetId = static_cast<unsigned int>(lootSet);
        if (m_initData->mMiniGameId == eMiniGameId_Battle)
            m_initData->mPlayerSettings.mRandomLootSet = lootSet == 5;
        const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true);
        m_gameOptionsList.SetItemLabel(0x20,
                                       UIString(std::wstring(L10N::GetString(IDS_MG01_ITEMSET)) + L": "
                                                + miniGame->GetLootSetText(m_initData->mPlayerSettings.mLootSetId)),
                                       false);
        UpdateDescriptionText();
        break;
    }
    case 0x21:
    {
        m_initData->mPlayerSettings.mFoodBalance = m_gameOptionsList.GetSliderValue(0x21);
        const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true);
        m_gameOptionsList.SetItemLabel(
            0x21,
            UIString(std::wstring(L10N::GetString(0x1cbfab4a)) + L": "
                     + miniGame->GetFoodBalanceIndexText(m_initData->mPlayerSettings.mFoodBalance)),
            false);
        UpdateDescriptionText();
        break;
    }
    case 0x22:
    {
        const int lives = m_gameOptionsList.GetSliderValue(0x22);
        m_initData->mPlayerSettings.mLivesPerRound = static_cast<unsigned int>(lives);
        if (lives == 0)
            swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG_LIVES_PER_ROUND), L10N::GetString(IDS_MG_INFINITE));
        else
            swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG_LIVES_PER_ROUND), lives);
        m_gameOptionsList.SetItemLabel(0x22, UIString(text), false);
        UpdateDescriptionText();
        m_initData->mSettingsChanged = true;
        return;
    }
    case 0x23:
        m_initData->mPlayerSettings.mRoundCount = m_gameOptionsList.GetSliderValue(0x23);
        swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG01_ROUND_COUNT), m_initData->mPlayerSettings.mRoundCount);
        m_gameOptionsList.SetItemLabel(0x23, UIString(text), false);
        UpdateDescriptionText();
        m_initData->mSettingsChanged = true;
        return;
    case 0x24:
    {
        const int mapSize = m_gameOptionsList.GetSliderValue(0x24);
        if (mapSize == 0)
            m_initData->mPlayerSettings.mMapSizeOverride = maybe<int>();
        else
            m_initData->mPlayerSettings.mMapSizeOverride = maybe<int>(mapSize);
        const int overrideValue = m_initData->mPlayerSettings.mMapSizeOverride.isSet()
                                      ? m_initData->mPlayerSettings.mMapSizeOverride.get()
                                      : 0;
        const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(m_initData->mMiniGameId, true);
        m_gameOptionsList.SetItemLabel(0x24,
                                       UIString(std::wstring(L10N::GetString(IDS_MG01_MAP_SIZE)) + L": "
                                                + miniGame->GetMapSizeOverrideText(overrideValue)),
                                       false);
        UpdateDescriptionText();
        break;
    }
    case 0x2b:
    {
        const int arenaSize = m_gameOptionsList.GetSliderValue(0x2b);
        if (arenaSize == 0)
            m_initData->mPlayerSettings.mArenaSizeMultiplier = maybe<float>();
        else
            m_initData->mPlayerSettings.mArenaSizeMultiplier = maybe<float>(arenaSizeTable[arenaSize - 1]);
        if (arenaSize > 0)
        {
            std::wstring display = _toString<float>(arenaSizeTable[arenaSize + 4]);
            swprintf(text, 256, L"%ls: x%ls", L10N::GetString(IDS_MG02_LAYER_SCALE), display.c_str());
        }
        else
        {
            swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG02_LAYER_SCALE), L10N::GetString(IDS_MG_AUTO));
        }
        m_gameOptionsList.SetItemLabel(0x2b, UIString(text), false);
        UpdateDescriptionText();
        m_initData->mSettingsChanged = true;
        return;
    }
    case 0x2c:
    {
        const int layerCount = m_gameOptionsList.GetSliderValue(0x2c);
        if (layerCount == 0)
        {
            m_initData->mPlayerSettings.mLayerCount = maybe<unsigned int>();
            swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG02_LAYER_COUNT), L10N::GetString(IDS_MG_AUTO));
        }
        else
        {
            m_initData->mPlayerSettings.mLayerCount = maybe<unsigned int>(static_cast<unsigned int>(layerCount));
            if (layerCount > 0)
                swprintf(text, 256, L"%ls: %d", L10N::GetString(IDS_MG02_LAYER_COUNT), layerCount);
            else
                swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_MG02_LAYER_COUNT), L10N::GetString(IDS_MG_AUTO));
        }
        m_gameOptionsList.SetItemLabel(0x2c, UIString(text), false);
        UpdateDescriptionText();
        break;
    }
    case 0x2e:
        m_initData->mMatchmakingFilters[0] = value;
        m_lootSet.Update(m_gameOptionsList, value);
        UpdateDescriptionText();
        m_initData->mSettingsChanged = true;
        return;
    case 0x2f:
        m_initData->mMatchmakingFilters[1] = value;
        m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, value);
        m_foodBalance.Update(m_gameOptionsList, value);
        UpdateDescriptionText();
        m_initData->mSettingsChanged = true;
        return;
    default:
        break;
    }
    m_initData->mSettingsChanged = true;
}

void UIScene_LaunchMoreOptionsMenu::handleGainFocus(bool)
{
    UpdateDescriptionText();
}

void UIScene_LaunchMoreOptionsMenu::handleCheckboxElementToggled(int, int second, bool checked)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    switch (second)
    {
    case 0:
        m_initData->mOnlineGame = checked;
        m_updateOnline = true;
        m_initData->mSettingsChanged = true;
        break;
    case 1:
        m_initData->mSettingsChanged = true;
        m_initData->mInviteOnly = checked;
#ifdef MINECRAFT_PC
        m_updateOnline = true;
#endif
        break;
    case 2:
        m_initData->mSettingsChanged = true;
        m_initData->mAllowFriendsOfFriends = checked;
        break;
#ifdef MINECRAFT_PC
    case PUBLIC_TITLE_ITEM:
        m_initData->mSettingsChanged = true;
        m_initData->mPublicWorldTitle = checked;
        break;
#endif
    case 3:
        m_initData->mSettingsChanged = true;
        m_initData->mPvP = checked;
        break;
    case 4:
        m_initData->mSettingsChanged = true;
        m_initData->mTrustPlayers = checked;
        break;
    case 5:
        m_initData->mSettingsChanged = true;
        m_initData->mFireSpreads = checked;
        break;
    case 6:
        m_initData->mSettingsChanged = true;
        m_initData->mTNT = checked;
        break;
    case 7:
        m_initData->mHostPrivileges = checked;
        m_updateCheats = true;
        m_initData->mSettingsChanged = true;
        break;
    case 8:
        m_initData->mSettingsChanged = true;
        m_initData->mResetNether = checked;
        break;
    case 9:
        m_initData->mSettingsChanged = true;
        m_initData->mResetEnd = checked;
        break;
    case 10:
        m_initData->mStructures = m_initData->mFlatWorld || checked;
        m_initData->mSettingsChanged = true;
        break;
    case 11:
        m_initData->mSettingsChanged = true;
        m_initData->mBonusChest = checked;
        break;
    case 12:
        m_initData->mSettingsChanged = true;
        m_initData->mKeepInventory = checked;
        break;
    case 13:
        m_initData->mSettingsChanged = true;
        m_initData->mDoMobSpawning = checked;
        break;
    case 14:
        m_initData->mSettingsChanged = true;
        m_initData->mDoMobLoot = checked;
        break;
    case 15:
        m_initData->mSettingsChanged = true;
        m_initData->mMobGriefing = checked;
        break;
    case 16:
        m_initData->mSettingsChanged = true;
        m_initData->mDoTileDrops = checked;
        break;
    case 17:
        m_initData->mSettingsChanged = true;
        m_initData->mNaturalRegeneration = checked;
        break;
    case 18:
        m_initData->mSettingsChanged = true;
        m_initData->mDoDaylightCycle = checked;
        break;
    case 19:
        m_initData->mSettingsChanged = true;
        m_initData->mDoWeatherCycle = checked;
        break;
    case 20:
        m_initData->mStructures = true;
        m_initData->mFlatWorld = checked;
        if (!m_initData->mWorldTypeLocked)
        {
            m_worldOptionsList.EnableItem(0x15, checked, false);
            m_worldOptionsList.EnableItem(10, !m_initData->mFlatWorld, false);
            if (!m_initData->mFlatWorld)
            {
                m_worldOptionsList.SetCheckboxValue(10, true, false);
                m_initData->mSettingsChanged = true;
                return;
            }
            SuperflatMenu::PresetCache::GetCachedPreset(m_playerId, m_initData->mSuperflatConfig);
            m_initData->mSettingsChanged = true;
            return;
        }
        m_initData->mSettingsChanged = true;
        break;
    case 23:
        m_initData->mSettingsChanged = true;
        m_initData->mFindBalancedSeed = checked;
        break;
    case 26:
        m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_CentreSpawn, checked);
        m_initData->mSettingsChanged = true;
        break;
    case 0x1f:
        if (MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_NoSmallSkins, checked ? 0 : 1);
        m_initData->mSettingsChanged = true;
        break;
    case 0x25:
        if (MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mNaturalRegeneration = checked;
        m_initData->mSettingsChanged = true;
        break;
    case 0x27:
        if (MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mSmallInventoryEnabled = checked;
        m_initData->mSettingsChanged = true;
        break;
    case 0x28:
        if (MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mTakeEverythingEnabled = checked;
        m_initData->mSettingsChanged = true;
        break;
    case 0x29:
        if (MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mChestRefillEnabled = checked;
        m_initData->mSettingsChanged = true;
        break;
    case 0x2a:
        if (MiniGameDef::IsCustomGame(m_initData->mMiniGameId, m_initData->mPlayerSettings))
            m_initData->mPlayerSettings.mSpectatorParticipation = checked;
        m_initData->mSettingsChanged = true;
        break;
    default:
        m_initData->mSettingsChanged = true;
        break;
    }
}

void UIScene_LaunchMoreOptionsMenu::handleTouchInput(unsigned int, int, int, int touchBox, bool pressed, bool, bool)
{
    if (!pressed)
        return;
    switch (touchBox)
    {
    case 0x34:
    case 0x35:
        break;
    default:
        return;
    }
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    bool world = touchBox == 0x34;
    if (m_menuType != static_cast<int>(world))
    {
        m_menuType = world;
        updateTooltips();
        static_cast<FJ_LaunchMoreOptionsMenu*>(m_movie->getRootNode()->getFuiNode())->ChangeTab();
    }
    gConsoleUIController.TouchBoxRebuild(this);
}

UIControl* UIScene_LaunchMoreOptionsMenu::GetMainPanel()
{
    return m_menuType == 1 ? &m_worldOptions : &m_gameOptions;
}

int UIScene_LaunchMoreOptionsMenu::getSceneType()
{
    return EUIScene_LaunchMoreOptionsMenu;
}
