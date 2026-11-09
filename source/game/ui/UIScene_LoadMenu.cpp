#include "game/ui/UIScene_LoadMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringConversion.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "vendor/_SaveListDetails.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/ui/FJ_LoadMenu.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/core/System.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcNetworkSession.hpp"
#include "platform/pc/PcSettings.hpp"
#endif
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "platform/wiiu/CProfile.hpp"

extern ConsoleUIController gConsoleUIController;

namespace
{
struct LoadMenuSceneData
{
    int playerId;
    int saveIndex;
    LevelGenerationOptions* generationOptions;
    _SaveListDetails* saveDetails;
    EMiniGameId miniGameId;
};

const int sControlDescriptions[] = {-1,
                                    -1,
                                    -1,
                                    -1,
                                    -1,
                                    static_cast<int>(0x1a93e279),
                                    -1,
                                    -1,
                                    static_cast<int>(0x4f8bedac),
                                    static_cast<int>(0x4f8bedac),
                                    static_cast<int>(0xfb9d0b2b),
                                    static_cast<int>(0x7b994768),
                                    static_cast<int>(0x0f7151d3),
                                    static_cast<int>(0x75cf2428),
                                    static_cast<int>(0x4c17ef92),
                                    -1,
                                    static_cast<int>(IDS_MASHUP_CHOOSE_BETWEEN_SEVERAL_VARIANTS_SAME_CORE_GAME_TEXT),
                                    static_cast<int>(IDS_MG_JOIN_ANY_GAME_DESC),
                                    -1,
                                    -1,
                                    -1,
                                    static_cast<int>(0x29bfcbdb)};
}

UIScene_LoadMenu::UIScene_LoadMenu(int sceneType, void* data, UILayer* layer)
    : IUIScene_StartGame(sceneType, data, layer, true),
      m_seedLogic(),
      m_sizeLogic(),
      m_gameModeLogic(),
      m_difficultyLogic(),
      m_mainPanel(),
      m_gameName(),
      m_description(),
      m_createdMode(),
      m_playButton(),
      m_loadSettings(),
      m_levelIcon(),
      m_list(),
      m_thumbnailName()
{
#ifdef MINECRAFT_PC
    m_availableGames = -1;
    m_availableCheckedAt = 0;
#endif
    initialiseMovie();
    m_publicGamesAllowed = true;
    m_updateInProgress = true;
    m_resetNetherPending = true;
    m_updateEndPending = true;
    m_hasGameData = false;
    m_selectedSlot = -1;
    m_focus = -1.0;
    m_childFocus = -1.0;
    m_description.init(UIString(L""));
    m_createdMode.init(UIString(L10N::GetString(IDS_CREATED_IN_SURVIVAL)));

    LoadMenuSceneData* sceneData = static_cast<LoadMenuSceneData*>(data);
    bool gameModeLobby = m_levelGenerationOptions != NULL && m_levelGenerationOptions->isGameModeLobby();

    EMiniGameId miniGameId = sceneData->miniGameId;
    m_launchData.mMiniGameId = miniGameId;
    m_launchData.mJoinMiniGame = !gameModeLobby && miniGameId != eMiniGameId_Any && miniGameId > eMiniGameId_Lobby;
    m_launchData.mHostMiniGame = gameModeLobby && miniGameId != eMiniGameId_Any && miniGameId > eMiniGameId_Lobby;

    m_texturePackList.init(std::wstring(L10N::GetString(IDS_DLC_MENU_TEXTUREPACKS)), 4);
    m_launchData.mMatchmaking = false;
    if (m_launchData.mJoinMiniGame)
        m_launchData.mOnlineGame = true;
    SetUpVariants();

    m_texturePackName.init(UIString(L""));
    m_texturePackDescription.init(UIString(L""));
    bool hostMiniGame = m_launchData.mHostMiniGame;

    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
    {
        UpdateIfPublicGamesAllowed();
        m_gameTypeId = GameType::LOBBY->getId();
        m_difficulty = 3;
        MasterGameMode::SetDefaults(m_launchData);
#ifdef MINECRAFT_PC
        if (m_launchData.mHostMiniGame && m_publicGamesAllowed)
            m_launchData.mMatchmaking = true;
#endif
        _initCustom();
        hostMiniGame = m_launchData.mHostMiniGame;
    }
    m_thumbnailStateUnknown = false;
    m_thumbnailFailed = false;
    m_thumbnailPending = true;
    m_loadRequested = false;
    m_thumbnailLoaded = false;
    if (!hostMiniGame && !m_launchData.mJoinMiniGame)
    {
        m_state614 = 0;
        m_difficulty = GameSettings::GetGameSettings(m_playerId, eGameSetting_Difficulty);
        wchar_t difficultyLabel[256];
        std::swprintf(
            difficultyLabel, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
            L10N::GetString(
                Difficulty::byId(GameSettings::GetGameSettings(m_playerId, eGameSetting_Difficulty))->getKey()));
        m_difficultySlider.init(UIString(difficultyLabel), 2, 0, 3,
                                GameSettings::GetGameSettings(m_playerId, eGameSetting_Difficulty), false, 10, 1);
        m_gameModeSlider.init(UIString(L10N::GetString(IDS_GAMEMODE_SURVIVAL)), 1, 0, GameType::ADVENTURE->getId(), 0,
                              false, 10, 1);
        m_texturePackList.init(std::wstring(L10N::GetString(IDS_DLC_MENU_TEXTUREPACKS)), 4);
        m_playButton.init(UIString(L10N::GetString(IDS_MORE_OPTIONS)), 3);
        m_loadSettings.init(UIString(L10N::GetString(IDS_LOAD)), 6);
        bool initialOnlineChecked = m_launchData.mOnlineGame;
        m_onlineGameCheckBox.SetEnable(true);
        if (!gProfile->IsSignedInLive(m_playerId))
            m_onlineGameCheckBox.SetEnable(false);
        if (m_launchData.mOnlineSettingChangedBySystem)
        {
            m_onlineGameCheckBox.SetEnable(false);
            initialOnlineChecked = false;
        }
        m_onlineGameCheckBox.init(UIString(L10N::GetString(IDS_ONLINE_GAME)), 5, initialOnlineChecked);

        wchar_t thumbnailName[64];
        if (m_levelGenerationOptions == NULL)
        {
            if (sceneData->saveDetails != NULL)
            {
                std::wstring saveFileName = convUtf8ToWstring(sceneData->saveDetails->m_filename);
                wcsncpy(thumbnailName, saveFileName.c_str(), 32);

                m_thumbnailName = thumbnailName;

                if (sceneData->saveDetails->m_thumbnail == NULL)
                {
                    SAVE_DETAILS* saves = gStorage->ReturnSavesInfo();
                    gStorage->LoadSaveDataThumbnail(&saves->saves[m_saveIndex], LoadSaveDataThumbnailReturned,
                                                    reinterpret_cast<void*>(GetCallbackUniqueId()));
                    m_loadRequested = true;
                }
                else
                {
                    m_thumbnailData = sceneData->saveDetails->m_thumbnail;
                    m_thumbnailSize = sceneData->saveDetails->m_thumbnailSize;
                    m_thumbnailLoaded = true;
                }
                m_gameName.init(UIString(sceneData->saveDetails->m_title));
            }
        }
        else
        {
            std::wstring levelName(m_levelGenerationOptions->getDisplayName());
            m_gameName.init(UIString(levelName));

            m_launchData.mTexturePackId = m_levelGenerationOptions->getTexturePackId();
            if (m_levelGenerationOptions->requiresTexturePack())
                m_texturePackList.setEnabled(false);

            gApp.SetBaseSaveName(m_levelGenerationOptions->getBaseSavePath());
            m_state62B = m_levelGenerationOptions->getLevelHasBeenInCreative();
            m_state62C = m_levelGenerationOptions->requiresTexturePack();

            TexturePack* texturePack = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(
                m_launchData.mTexturePackId);
            unsigned int iconSize = 0;
            const void* icon = texturePack->getPackIcon(iconSize);
            if (iconSize != 0 && icon != NULL)
            {
                wchar_t iconName[64];
                swprintf(iconName, 64, L"loadsave");
                registerSubstitutionTexture(iconName, reinterpret_cast<unsigned char*>(const_cast<void*>(icon)),
                                            iconSize, false);
                m_levelIcon.setTextureName(iconName, true);
            }

            m_createdMode.setLabel(UIString(m_state62B ? IDS_CREATED_IN_CREATIVE : IDS_CREATED_IN_SURVIVAL), false,
                                   false);
        }
    }
    m_state614 = 0;
    _init();
    ::operator delete(sceneData);
    gConsoleUIController.TouchBoxRebuild(this);
    m_updateInProgress = false;
}

void UIScene_LoadMenu::AddVariantButtons(bool alternate)
{
    if (alternate)
    {
        unsigned int label = GetVariantPlayLabel(1, alternate);
        m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(label))), 0x12, true);
    }
    else
    {
        unsigned int label = GetVariantPlayLabel(0, false);
        m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(label))), 0x11, true);
    }
}

void UIScene_LoadMenu::UpdateIfPublicGamesAllowed()
{
    m_publicGamesAllowed = true;
    if (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame)
        return;
    MiniGameDef* game = MiniGameDef::GetCustomGameModeById(eMiniGameId_Lobby, true);
    if (!game->IsCRCValid())
    {
        m_publicGamesAllowed = false;
        m_launchData.mMatchmaking = false;
        return;
    }
    if (m_publicGamesAllowed)
    {
        game = MiniGameDef::GetCustomGameModeById(m_launchData.mMiniGameId, true);
        if (!game->IsCRCValid())
        {
            m_publicGamesAllowed = false;
            m_launchData.mMatchmaking = false;
            return;
        }
        bool valid = m_publicGamesAllowed;
        if (valid)
        {
            if (m_levelGenerationOptions != NULL)
            {
                valid = m_levelGenerationOptions->isCRCValid();
                m_publicGamesAllowed = valid;
                if (!valid)
                {
                    m_launchData.mMatchmaking = false;
                    return;
                }
            }
            std::vector<LevelRules::LevelRulesetGroup*>::iterator group = m_launchData.mLevelRulesetGroups.begin();
            while (valid)
            {
                if (group == m_launchData.mLevelRulesetGroups.end())
                    return;
                std::vector<LevelRuleset*>::iterator ruleset = (*group)->rulesets.begin();
                while (ruleset != (*group)->rulesets.end() && valid)
                {
                    valid = (*ruleset)->getGenerationOptions()->isCRCValid();
                    ++ruleset;
                    m_publicGamesAllowed = valid;
                }
                ++group;
            }
        }
    }
    m_launchData.mMatchmaking = false;
}

void UIScene_LoadMenu::UpdateDescriptionText()
{
    if (m_childFocus == -1.0 || (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame))
        return;

    int childFocus = static_cast<int>(m_childFocus);
    int descriptionId = -1;
#ifdef MINECRAFT_PC
    if (childFocus == 0x30)
        descriptionId = ePcString_ProximityChatDescription;
    else
#endif
        descriptionId = sControlDescriptions[childFocus];
    if (m_childFocus == 3.0 || m_childFocus == 16.0 || m_childFocus == 17.0 || m_childFocus == 18.0)
    {
        descriptionId = static_cast<int>(
            GetVariantDesc(static_cast<int>(m_launchData.mMatchmakingFilters[1]), m_launchData.mHostMiniGame));
    }
    else
    {
        switch (childFocus)
        {
        case 0x11:
            descriptionId = static_cast<int>(GetVariantDesc(0, m_launchData.mHostMiniGame));
            break;
        case 0x12:
            descriptionId = static_cast<int>(GetVariantDesc(1, m_launchData.mHostMiniGame));
            break;
        case 0x13:
            descriptionId = static_cast<int>(GetVariantDesc(2, m_launchData.mHostMiniGame));
            break;
        }
    }

    std::wstring description(L"");
    if (descriptionId != -1 && descriptionId != -2)
    {
        description = gApp.FormatHTMLString(m_playerId, std::wstring(L10N::GetString(descriptionId)), -1);
    }
    if (!gRenderer->IsHiDef())
        gRenderer->IsWidescreen();

    wchar_t colourTag[64];
    std::swprintf(colourTag, 64, L"<font color=\"#%08x\">", gApp.GetHTMLColour(eMinecraftColour_HTMLColor_White));
    description = colourTag + description;
    m_texturePackDescription.setLabel(UIString(description), false, false);
}

void UIScene_LoadMenu::_initCustom()
{
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;

    static_cast<FJ_LoadMenu*>(m_movie->getRootNode()->getFuiNode())->InitCustomLoadMenu();
    m_description.removeUINodeRef();
    m_texturePackList.removeUINodeRef();
    m_gameModeSlider.removeUINodeRef();
    m_onlineGameCheckBox.removeUINodeRef();
    m_playButton.removeUINodeRef();
    m_loadSettings.removeUINodeRef();
    m_difficultySlider.removeUINodeRef();
    m_texturePackName.removeUINodeRef();
    m_texturePackIcon.removeUINodeRef();

    MiniGameDef* game = GetMiniGame();
    m_gameName.init(UIString(L10N::GetString(static_cast<int>(game->GetName()))));
#ifdef MINECRAFT_PC
    m_availableGames = -1;
    m_availableCheckedAt = 0;
#endif
    m_levelIcon.setTextureName(game->getTextureName(), true);
    m_createdMode.init(UIString(L""));

    if (initData->mHostMiniGame)
    {
        MiniGameDef* mode = MiniGameDef::GetCustomGameModeById(initData->mMiniGameId, true);
        mode->SetDefaultMatchmakingSearchData(initData->mJoinMiniGame, initData);
        m_list.init(7);
        if (mode->HasTeamGameOption())
        {
            m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(IDS_MG_OPTION_CENTRE_SPAWN))), 9,
                                  initData->mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_IsTeams) != 0);
        }

        bool allowOnlineExtras = initData->mOnlineGame && m_onlineEnabled;
        bool signedInLive = gProfile->IsSignedInLive(m_playerId);
        if (!signedInLive)
            allowOnlineExtras = false;
        if (initData->mOnlineSettingChangedBySystem)
            allowOnlineExtras = false;

        m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(IDS_ONLINE_GAME))), 5, allowOnlineExtras);
        m_list.EnableItem(5, m_onlineEnabled, true);

        m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0x3fc1a0bd))), 10,
                              initData->mMatchmaking && m_publicGamesAllowed);

        m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(IDS_ALLOWFRIENDSOFFRIENDS))), 0xb,
                              initData->mAllowFriendsOfFriends && allowOnlineExtras);

#ifdef MINECRAFT_PC
        m_list.AddNewCheckbox(L"Proximity Chat", 0x30, PcSettingsHostProximity() == kPcHostProximityForce);
#endif

        game = GetMiniGame();
        if (game->GetId() == 3 && initData->mHostMiniGame)
        {
            m_list.AddNewCheckbox(std::wstring(L10N::GetString(static_cast<int>(0x2e1306d9))), 0x15,
                                  initData->mTimeTrial);
        }

        game = GetMiniGame();
        if (game->HasMultipleMaps() && initData->mMiniGameId != eMiniGameId_Tumble)
        {
            m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(IDS_MG_SELECT_MAPS))), 0xd, true);
            m_list.EnableItem(0xd, true, true);
        }

        game = GetMiniGame();
        if (game->HasGameVariants())
        {
            int value = initData->mMatchmakingFilters[1];
            m_sizeLogic = SliderLogic(0x10, static_cast<int>(IDS_MG_OPTION_GAME_VARIANT),
                                      eCustomGameModeHostOption_ModeVariant, initData->mMiniGameId, true);
            m_sizeLogic.AddTo(m_list, value);
            int clamped = value - 1;
            if (clamped < 0)
                clamped = 0;
            initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, clamped);
        }

        game = GetMiniGame();
        if (game->GetId() != 3)
        {
            m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(IDS_MORE_OPTIONS))), 3, true);
            m_list.EnableItem(3, true, true);
        }

        AddVariantButtons(true);

        m_list.EnableItem(10, allowOnlineExtras && m_publicGamesAllowed, true);
        m_list.EnableItem(0xb, allowOnlineExtras && !initData->mMatchmaking, true);
        m_list.SetCheckboxValue(10, allowOnlineExtras && initData->mMatchmaking && m_publicGamesAllowed, true);
        m_list.SetCheckboxValue(0xb, allowOnlineExtras && !initData->mMatchmaking && initData->mAllowFriendsOfFriends,
                                true);
    }

    if (initData->mJoinMiniGame)
    {
        MiniGameDef* mode = MiniGameDef::GetCustomGameModeById(initData->mMiniGameId, true);
        mode->SetDefaultMatchmakingSearchData(initData->mJoinMiniGame, initData);

        game = GetMiniGame();
        if (game->HasGameVariants())
        {
            int value = initData->mMatchmakingFilters[1];
            m_sizeLogic = SliderLogic(0x10, static_cast<int>(IDS_MG_OPTION_GAME_VARIANT),
                                      eCustomGameModeHostOption_ModeVariant, initData->mMiniGameId, false);
            m_sizeLogic.AddTo(m_list, value);
            int clamped = value - 1;
            if (clamped < 0)
                clamped = 0;
            initData->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, clamped);
        }

        game = GetMiniGame();
        if (game->GetId() != 3 && initData->mMiniGameId != eMiniGameId_Tumble)
        {
            m_list.AddNewButton(std::wstring(L10N::GetString(static_cast<int>(IDS_MORE_OPTIONS))), 3, true);
        }

        AddVariantButtons(false);
    }

    if (!initData->mHostMiniGame)
    {
        if (!initData->mJoinMiniGame)
        {
            int count = static_cast<int>(m_list.getItemCount());
            m_list.setCurrentSelection(count - 1, false, true);
            UpdateDescriptionText();
            static_cast<FJ_LoadMenu*>(m_movie->getRootNode()->getFuiNode())->RescaleCustomLoadMenu(false);
        }
        else
        {
            int index = m_list.getListIndex(0x11);
            m_list.setCurrentSelection(index, false, true);
            m_childFocus = 17.0;
            UpdateDescriptionText();
            static_cast<FJ_LoadMenu*>(m_movie->getRootNode()->getFuiNode())->RescaleCustomLoadMenu(false);
        }
    }
    else
    {
        int index = m_list.getListIndex(0x12);
        m_list.setCurrentSelection(index, false, true);
        m_childFocus = 18.0;
        UpdateDescriptionText();
        static_cast<FJ_LoadMenu*>(m_movie->getRootNode()->getFuiNode())->RescaleCustomLoadMenu(false);
    }
}

int UIScene_LoadMenu::LoadSaveDataThumbnailReturned(void* context, unsigned char* data, unsigned int size)
{
    UIScene_LoadMenu* scene = static_cast<UIScene_LoadMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context))));
    if (scene != NULL)
    {
        if (data != NULL && size != 0)
        {
            scene->registerSubstitutionTexture(scene->m_thumbnailName, data, size, false);
            scene->m_thumbnailData = data;
            scene->m_thumbnailSize = size;
            scene->m_thumbnailLoaded = true;
            scene->m_thumbnailPending = false;
        }
        else
        {
            scene->m_thumbnailFailed = true;
            scene->m_thumbnailPending = false;
        }
    }
    return 0;
}

unsigned int UIScene_LoadMenu::mapElementsAndNames()
{
    IUIScene_StartGame::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_mainPanel.setupControl(this, currentNode, std::string("MainPanel"));
        m_controls.push_back(&m_mainPanel);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_mainPanel.getFuiRenderNode();
        {
            m_gameName.setupControl(this, currentNode, std::string("GameName"));
            m_controls.push_back(&m_gameName);
        }
        {
            m_createdMode.setupControl(this, currentNode, std::string("CreatedMode"));
            m_controls.push_back(&m_createdMode);
        }
        {
            m_description.setupControl(this, currentNode, std::string("Seed"));
            m_controls.push_back(&m_description);
        }
        {
            m_texturePackList.setupControl(this, currentNode, std::string("TexturePackSelector"));
            m_controls.push_back(&m_texturePackList);
        }
        {
            m_gameModeSlider.setupControl(this, currentNode, std::string("GameModeToggle"));
            m_controls.push_back(&m_gameModeSlider);
        }
        {
            m_onlineGameCheckBox.setupControl(this, currentNode, std::string("CheckboxOnline"));
            m_controls.push_back(&m_onlineGameCheckBox);
        }
        {
            m_playButton.setupControl(this, currentNode, std::string("MoreOptions"));
            m_controls.push_back(&m_playButton);
        }
        {
            m_loadSettings.setupControl(this, currentNode, std::string("LoadSettings"));
            m_controls.push_back(&m_loadSettings);
        }
        {
            m_difficultySlider.setupControl(this, currentNode, std::string("Difficulty"));
            m_controls.push_back(&m_difficultySlider);
        }
        {
            m_levelIcon.setupControl(this, currentNode, std::string("LevelIcon"));
            m_controls.push_back(&m_levelIcon);
        }
        {
            m_list.setupControl(this, currentNode, std::string("MultiList"));
            m_controls.push_back(&m_list);
        }
        currentNode = parentNode;
    }
    return 1;
}

void UIScene_LoadMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, false, false);
}

void UIScene_LoadMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
    bool widescreen = false;
    if (gRenderer->IsWidescreen())
        widescreen = true;
    m_layer->showComponent(m_playerId, EUIScene_Logo, widescreen);
}

unsigned int UIScene_LoadMenu::getSubSceneType() const
{
    if (m_launchData.mMiniGameId != eMiniGameId_Any)
        return m_launchData.mJoinMiniGame ? 3 : 2;
    return static_cast<unsigned int>(m_state62C);
}

void UIScene_LoadMenu::tick()
{
    m_updateInProgress = true;
    IUIScene_StartGame::tick();
    m_updateInProgress = false;
#ifdef MINECRAFT_PC
    if (m_launchData.mJoinMiniGame)
    {
        const unsigned long long now = System::processTimeInMilliSecs();
        if (m_availableCheckedAt == 0 || now - m_availableCheckedAt >= 1000)
        {
            m_availableCheckedAt = now;
            const int available =
                PcNetworkSession::CountMatchmakingGames(static_cast<unsigned char>(m_launchData.mMiniGameId));
            if (available != m_availableGames)
            {
                m_availableGames = available;
                wchar_t suffix[32];
                swprintf(suffix, sizeof(suffix) / sizeof(suffix[0]), L" (%d Available)", available);
                m_gameName.setLabel(
                    UIString(std::wstring(L10N::GetString(static_cast<int>(GetMiniGame()->GetName()))) + suffix),
                    false, false);
            }
        }
    }
#endif
    if (gConsoleUIController.IsSceneInStack(m_playerId, EUIScene_LaunchMoreOptionsMenu))
    {
        MiniGameDef* game = MiniGameDef::GetCustomGameModeById(m_launchData.mMiniGameId, true);
        if (game->HasGameVariants() && m_list.IsSlider(0x10))
        {
            int value = static_cast<int>(m_list.GetSliderValue(0x10));
            if (value != m_launchData.mMatchmakingFilters[1])
            {
                m_list.SetSliderValue(0x10, m_launchData.mMatchmakingFilters[1], true);
                m_sizeLogic.Update(m_list, m_launchData.mMatchmakingFilters[1]);
            }
        }
    }
    if (m_loadRequested)
    {
        m_loadRequested = false;
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_Timer, NULL, EUILayer_Scene, EUIGroup_PlayerDefault);
    }

    if (m_thumbnailFailed)
    {
        gConsoleUIController.NavigateBack(m_playerId, false, EUIScene_LoadCreateJoinMenu, EUILayer_Count);
        return;
    }
    if (!m_thumbnailLoaded)
        return;

    m_thumbnailLoaded = false;
    m_levelIcon.setTextureName(m_thumbnailName.c_str(), true);

    CMinecraftApp::PngTagData tagData;
    gApp.GetImageTextData(m_thumbnailData, m_thumbnailSize, &tagData);
    tagData.flags |= CMinecraftApp::PngTagData::eFlag_NumLoads;
    ++tagData.numLoads;
    gApp.SetCurrentImageTextData(&tagData);

    if ((tagData.flags & CMinecraftApp::PngTagData::eFlag_TexturePack) != 0)
        m_launchData.mTexturePackId = tagData.texturePack;

    if ((tagData.flags & CMinecraftApp::PngTagData::eFlag_Seed) == 0)
    {
        m_description.setLabel(UIString(L""), false, false);
    }
    else
    {
        wchar_t text[256];
        swprintf(text, 256, L"%ls: %lli", L10N::GetString(IDS_SEED), tagData.seed);
        m_description.setLabel(UIString(text), false, false);
    }

    if ((tagData.flags & CMinecraftApp::PngTagData::eFlag_HostOptions) != 0)
    {
        unsigned int hostOptions = tagData.hostOptions;
        m_launchData.mPvP = gApp.GetGameHostOption(hostOptions, eGameHostOption_PvP) != 0;
        m_launchData.mTrustPlayers = gApp.GetGameHostOption(hostOptions, eGameHostOption_TrustPlayers) != 0;
        m_launchData.mFireSpreads = gApp.GetGameHostOption(hostOptions, eGameHostOption_FireSpreads) != 0;
        m_launchData.mTNT = gApp.GetGameHostOption(hostOptions, eGameHostOption_TNT) != 0;
        m_launchData.mHostPrivileges = gApp.GetGameHostOption(hostOptions, eGameHostOption_CheatsEnabled) != 0;
        m_launchData.mDisableSaving = gApp.GetGameHostOption(hostOptions, eGameHostOption_DisableSaving) != 0;
        m_launchData.mCurrentWorldSize = gApp.GetGameHostOption(hostOptions, eGameHostOption_WorldSize);
        m_launchData.mWorldSize = m_launchData.mCurrentWorldSize;
        m_launchData.mMobGriefing = gApp.GetGameHostOption(hostOptions, eGameHostOption_MobGriefing) != 0;
        m_launchData.mKeepInventory = gApp.GetGameHostOption(hostOptions, eGameHostOption_KeepInventory) != 0;
        m_launchData.mDoMobSpawning = gApp.GetGameHostOption(hostOptions, eGameHostOption_DoMobSpawning) != 0;
        m_launchData.mDoMobLoot = gApp.GetGameHostOption(hostOptions, eGameHostOption_DoMobLoot) != 0;
        m_launchData.mDoTileDrops = gApp.GetGameHostOption(hostOptions, eGameHostOption_DoTileDrops) != 0;
        m_launchData.mNaturalRegeneration = gApp.GetGameHostOption(hostOptions, eGameHostOption_NaturalRegeneration)
                                            != 0;
        m_launchData.mDoDaylightCycle = gApp.GetGameHostOption(hostOptions, eGameHostOption_DoDaylightCycle) != 0;
        m_launchData.mDoWeatherCycle = gApp.GetGameHostOption(hostOptions, eGameHostOption_DoWeatherCycle) != 0;
        if (!m_launchData.mHostPrivileges)
        {
            m_launchData.mKeepInventory = false;
            m_launchData.mMobGriefing = true;
            m_launchData.mDoMobSpawning = true;
            m_launchData.mDoDaylightCycle = true;
        }
        if (gApp.GetGameHostOption(hostOptions, eGameHostOption_WasntSaveOwner) != 0)
            m_state629 = false;

        m_state62B = gApp.GetGameHostOption(hostOptions, eGameHostOption_HasBeenInCreative) != 0;
        m_createdMode.setLabel(
            UIString(L10N::GetString(m_state62B ? IDS_CREATED_IN_CREATIVE : IDS_CREATED_IN_SURVIVAL)), false, false);

        unsigned int gameMode = gApp.GetGameHostOption(hostOptions, eGameHostOption_GameType);
        if (gameMode == 1)
        {
            m_gameModeSlider.setLabel(UIString(L10N::GetString(IDS_GAMEMODE_CREATIVE)), false, false);
            m_isCreative = true;
            m_gameTypeId = GameType::CREATIVE->getId();
        }
        else if (gameMode == 2)
        {
            m_gameModeSlider.setLabel(UIString(L10N::GetString(IDS_GAMEMODE_ADVENTURE)), false, false);
            m_isCreative = false;
            m_gameTypeId = GameType::ADVENTURE->getId();
        }
        else
        {
            m_gameModeSlider.setLabel(UIString(L10N::GetString(IDS_GAMEMODE_SURVIVAL)), false, false);
            m_isCreative = false;
            m_gameTypeId = GameType::SURVIVAL->getId();
        }
        m_gameModeSlider.SetSliderValue(m_gameTypeId);

        bool onlineSetting = GameSettings::GetGameSettings(m_playerId, eGameSetting_Online);
        if (gApp.GetGameHostOption(hostOptions, eGameHostOption_FriendsOfFriends) != 0 && !m_onlineEnabled
            && !onlineSetting)
        {
            m_launchData.mAllowFriendsOfFriends = true;
        }

        m_difficulty = gApp.GetGameHostOption(eGameHostOption_Difficulty);
        m_difficultySlider.SetSliderValue(m_difficulty);
        m_difficultySlider.handleSliderMove(m_difficulty);
        wchar_t text[256];
        swprintf(text, 256, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                 L10N::GetString(Difficulty::byId(m_difficulty)->getKey()));
        m_difficultySlider.setLabel(UIString(text), false, false);
    }

    if ((tagData.flags & CMinecraftApp::PngTagData::eFlag_ExtraData) != 0
        && gApp.GetExtraGameData(tagData.extraData, eGameExtraData_SaveVersion) > EXTRA_DATA_SAVE_VERSION)
    {
        m_hasGameData = true;
        m_loadSettings.setEnable(false, true);
    }

    m_selectedSlot = getCurrentTextureSelectorIndex(m_launchData.mTexturePackId);
    UpdateTexturePackDescription(m_launchData.mTexturePackId);
    m_texturePackList.selectSlot(m_selectedSlot);
    gConsoleUIController.NavigateBack(m_playerId, false, static_cast<EUIScene>(getSceneType()), EUILayer_Count);
}

std::wstring UIScene_LoadMenu::getMoviePath()
{
    return std::wstring(L"LoadMenu");
}

void UIScene_LoadMenu::handleReload()
{
    IUIScene_StartGame::handleReload();
    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
        _initCustom();
}

void UIScene_LoadMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    if (m_actionPending)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            gApp.SetCorruptSaveDeleted(false);
            navigateBack();
            handled = true;
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuOtherStickUp:
    case EControllerActions_MenuOtherStickDown:
    case EControllerActions_MenuOk:
    {
        sendInputToMovie(action, repeat, pressed, released);
        if (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame)
        {
            bool checked = m_onlineGameCheckBox.IsChecked();
            if (m_launchData.mOnlineGame != checked)
            {
                m_launchData.mOnlineGame = checked;
                if (!checked)
                {
                    m_launchData.mAllowFriendsOfFriends = false;
                    m_launchData.mInviteOnly = false;
                }
            }
        }
        handled = true;
        break;
    }
    }
}

void UIScene_LoadMenu::handlePress(int controlId, int childId)
{
    if (m_actionPending)
        return;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
        controlId = childId;
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;
    switch (controlId)
    {
    case 3:
        if (!m_launchData.mHostMiniGame)
        {
            bool checked = m_onlineGameCheckBox.TouchIsChecked();
            if (initData->mOnlineGame != checked)
            {
                initData->mOnlineGame = checked;
                if (!checked)
                {
                    initData->mInviteOnly = false;
                    initData->mAllowFriendsOfFriends = false;
                }
            }
        }
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_LaunchMoreOptionsMenu, initData, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        return;
    case 4:
        gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
        UpdateCurrentTexturePack(childId);
        return;
    case 0xd:
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_MapSelectMenu, initData, EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        return;
    case 6:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
        break;
    default:
        return;
    }

    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
    {
        if (!MiniGameDef::IsCustomGame(m_launchData.mMiniGameId, initData->mPlayerSettings))
            MasterGameMode::SetDefaults(*initData);

        if (m_launchData.mHostMiniGame && m_launchData.mMiniGameId == eMiniGameId_Battle)
        {
            int mapSelection = 0;
            switch (m_launchData.mMiniGameId)
            {
            case eMiniGameId_Battle:
                mapSelection = GameSettings::GetBattleMiniGameMapSelection(m_playerId);
                break;
            case eMiniGameId_Glide:
                mapSelection = GameSettings::GetGlideMiniGameMapSelection(m_playerId);
                break;
            default:
                break;
            }
            std::vector<LevelRules::LevelRulesetGroup*>& groups = initData->mLevelRulesetGroups;
            if (mapSelection != 0)
            {
                std::vector<LevelRules::LevelRulesetGroup*>::iterator group = groups.begin();
                while (group != groups.end())
                {
                    LevelRules::LevelRulesetGroup* current = *group;
                    LevelRuleset* ruleset = current->rulesets.front();
                    if (!ruleset->getGenerationOptions()->getPack()->hasPurchasedPack()
                        && !gApp.mDLCManager.IsMiniGameTemporarilyFree(static_cast<int>(ruleset->getRuleType())))
                        group = groups.erase(group);
                    else
                        ++group;
                }
            }
            if (groups.empty())
            {
                int messageParameter = static_cast<int>(IDS_CONFIRM_OK);
                gConsoleUIController.RequestAlertMessage(
                    static_cast<int>(IDS_MG_NO_MAPS_SELECTED_TITLE), static_cast<int>(IDS_MG_NO_MAPS_SELECTED_TEXT),
                    &messageParameter, 1, static_cast<unsigned int>(m_playerId), 0, NULL, NULL, true);
                return;
            }
        }
    }

    if (!initData->mOnlineGame)
    {
        initData->mInviteOnly = false;
        initData->mMatchmaking = false;
        initData->mAllowFriendsOfFriends = false;
    }
    else if (initData->mMatchmaking)
    {
        initData->mInviteOnly = false;
        initData->mAllowFriendsOfFriends = false;
    }
    if (!m_launchData.mHostMiniGame)
    {
        bool checked = m_onlineGameCheckBox.TouchIsChecked();
        if (initData->mOnlineGame != checked)
        {
            initData->mOnlineGame = checked;
            if (!checked)
            {
                initData->mInviteOnly = false;
                initData->mAllowFriendsOfFriends = false;
            }
        }
    }
    StartSharedLaunchFlow();
}

void UIScene_LoadMenu::handleCheckboxToggled(int id, bool checked)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    UIScene::handleCheckboxToggled(id, checked);
}

void UIScene_LoadMenu::handleCheckboxElementToggled(int, int second, bool checked)
{
    if (m_updateInProgress)
        return;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    switch (second)
    {
    case 10:
    {
        m_launchData.mMatchmaking = checked;
        if (checked)
            m_launchData.mOnlineGame = true;
        if (GetMiniGame()->HasMultipleMaps())
            m_list.EnableItem(0xd, true, false);

        bool allowOnline = m_launchData.mOnlineGame && m_onlineEnabled;
        if (!gProfile->IsSignedInLive(m_playerId))
            allowOnline = false;
        if (m_launchData.mOnlineSettingChangedBySystem)
            allowOnline = false;
        if (GetMiniGame()->GetId() == 3 && m_launchData.mHostMiniGame)
        {
            if (checked)
                m_launchData.mTimeTrial = false;
            m_list.SetCheckboxValue(
                0x15, !m_launchData.mMatchmaking && !m_launchData.mOnlineGame && m_launchData.mTimeTrial, false);
        }
        m_list.EnableItem(0xb, allowOnline && !m_launchData.mMatchmaking, false);
        m_list.SetCheckboxValue(0xb, allowOnline && !m_launchData.mMatchmaking && m_launchData.mAllowFriendsOfFriends,
                                false);
        return;
    }
    case 5:
    {
        m_launchData.mOnlineGame = checked;
        m_list.EnableItem(10, checked && m_publicGamesAllowed, false);
        if (GetMiniGame()->HasMultipleMaps())
            m_list.EnableItem(0xd, true, false);

        bool allowOnline = m_launchData.mOnlineGame && m_onlineEnabled;
        if (!gProfile->IsSignedInLive(m_playerId))
            allowOnline = false;
        if (m_launchData.mOnlineSettingChangedBySystem)
            allowOnline = false;
        m_list.EnableItem(0xb, allowOnline && checked && !m_launchData.mMatchmaking, false);
        m_list.SetCheckboxValue(10, checked && m_launchData.mMatchmaking && m_publicGamesAllowed, false);
        m_list.SetCheckboxValue(0xb, checked && !m_launchData.mMatchmaking && m_launchData.mAllowFriendsOfFriends,
                                false);
        return;
    }
    case 0xb:
        m_launchData.mAllowFriendsOfFriends = checked;
        return;
#ifdef MINECRAFT_PC
    case 0x30:
        if (checked)
            PcSettingsSetHostProximity(kPcHostProximityForce);
        else
            PcSettingsSetHostProximity(kPcHostProximityUsersChoose);
        PcSettingsSave();
        return;
#endif
    case 8:
        m_launchData.mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_CentreSpawn, checked);
        return;
    case 9:
        m_launchData.mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_IsTeams, checked);
        return;
    case 0x15:
    {
        m_launchData.mTimeTrial = checked;
        m_launchData.mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_TimeTrial, checked);
        bool allowOnline = m_launchData.mOnlineGame && m_onlineEnabled;
        if (!gProfile->IsSignedInLive(m_playerId))
            allowOnline = false;
        if (m_launchData.mOnlineSettingChangedBySystem)
            allowOnline = false;
        m_list.EnableItem(5, !checked && m_onlineEnabled, false);
        m_list.EnableItem(10, !checked && allowOnline && m_publicGamesAllowed, false);
        m_list.EnableItem(0xb, !checked && allowOnline && !m_launchData.mMatchmaking, false);
        m_list.SetCheckboxValue(5, !checked && allowOnline, false);
        m_list.SetCheckboxValue(10, !checked && allowOnline && m_launchData.mMatchmaking && m_publicGamesAllowed,
                                false);
        m_list.SetCheckboxValue(
            0xb, !checked && allowOnline && !m_launchData.mMatchmaking && m_launchData.mAllowFriendsOfFriends, false);
        return;
    }
    }
}

void UIScene_LoadMenu::handleSliderElementMove(int, int second, int value)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    switch (second)
    {
    case 0xe:
        m_launchData.mMatchmakingFilters[0] = value;
        m_seedLogic.Update(m_list, value);
        break;
    case 0x10:
    {
        m_launchData.mMatchmakingFilters[1] = value;
        int variant = value - 1 < 0 ? 0 : value - 1;
        m_launchData.mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, variant);
        m_sizeLogic.Update(m_list, value);
        UpdateDescriptionText();
        break;
    }
    }
}

void UIScene_LoadMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus && !m_launchData.mHostMiniGame)
        m_onlineGameCheckBox.setChecked(m_launchData.mOnlineGame);
    if (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame)
        return;
    UpdateIfPublicGamesAllowed();
    if (m_launchData.mHostMiniGame)
    {
        bool allowOnlineExtras = m_launchData.mOnlineGame && m_onlineEnabled;
        if (!gProfile->IsSignedInLive(m_playerId))
            allowOnlineExtras = false;
        if (m_launchData.mOnlineSettingChangedBySystem)
            allowOnlineExtras = false;
        if (!m_launchData.mTimeTrial)
        {
            m_list.EnableItem(10, allowOnlineExtras && m_publicGamesAllowed, true);
            m_list.SetCheckboxValue(10, m_launchData.mOnlineGame && m_launchData.mMatchmaking && m_publicGamesAllowed,
                                    true);
        }
    }
}

void UIScene_LoadMenu::handleFocusChange(int focus, int childFocus)
{
    m_focus = static_cast<double>(focus);
    m_childFocus = static_cast<double>(childFocus);
    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
        UpdateDescriptionText();
    if (focus == 7)
        m_list.updateChildFocus(childFocus);
}

void UIScene_LoadMenu::handleTimerComplete(int timerId)
{
    if (m_launchData.mHostMiniGame && timerId == 0)
    {
        bool online = gProfile->IsSignedInLive(m_playerId) && gProfile->AllowedToPlayMultiplayer(m_playerId);
        if (online != m_onlineEnabled)
        {
            m_launchData.mOnlineGame = online;
            m_list.EnableItem(5, online, true);
            m_list.SetCheckboxValue(5, m_launchData.mOnlineGame, true);
            m_list.EnableItem(10, m_launchData.mOnlineGame && m_publicGamesAllowed, true);
            m_list.SetCheckboxValue(10, m_launchData.mOnlineGame && m_launchData.mMatchmaking && m_publicGamesAllowed,
                                    true);
            m_list.EnableItem(11, m_launchData.mOnlineGame && !m_launchData.mMatchmaking, true);
            m_list.SetCheckboxValue(
                11, m_launchData.mOnlineGame && !m_launchData.mMatchmaking && m_launchData.mAllowFriendsOfFriends,
                true);
        }
    }
    IUIScene_StartGame::handleTimerComplete(timerId);
}

void UIScene_LoadMenu::checkDialogReturned(bool& value, C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_Decline)
    {
        checkStateAndStartGame();
        return;
    }
    if (result == C4JStorage::EMessageResult_Accept)
    {
        value = false;
        checkStateAndStartGame();
        return;
    }
    m_actionPending = false;
    m_updateEndPending = true;
    m_resetNetherPending = true;
}

uint32_t UIScene_LoadMenu::CheckResetNetherReturned(void* context, int, C4JStorage::EMessageResult result)
{
    UIScene* callbackScene = gConsoleUIController.GetSceneFromCallbackId(
        static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context)));
    UIScene_LoadMenu* scene = static_cast<UIScene_LoadMenu*>(callbackScene);
    if (scene != NULL)
    {
        scene->m_resetNetherPending = false;
        scene->checkDialogReturned(scene->m_launchData.mResetNether, result);
    }
    return 0;
}

uint32_t UIScene_LoadMenu::CheckUpdateTheEndReturned(void* context, int, C4JStorage::EMessageResult result)
{
    UIScene* callbackScene = gConsoleUIController.GetSceneFromCallbackId(
        static_cast<unsigned int>(reinterpret_cast<uintptr_t>(context)));
    UIScene_LoadMenu* scene = static_cast<UIScene_LoadMenu*>(callbackScene);
    if (scene != NULL)
    {
        scene->m_updateEndPending = false;
        scene->checkDialogReturned(scene->m_launchData.mResetEnd, result);
    }
    return 0;
}

void UIScene_LoadMenu::checkStateAndStartGame()
{
    LaunchMoreOptionsMenuInitData* initData = &m_launchData;
    if (initData->mResetNether && m_resetNetherPending)
    {
        int parameters[2] = {static_cast<int>(IDS_DONT_RESET_NETHER), static_cast<int>(IDS_RESET_NETHER)};
        gConsoleUIController.RequestAlertMessage(
            static_cast<int>(IDS_RESETNETHER_TITLE), static_cast<int>(IDS_RESETNETHER_TEXT), parameters, 2,
            static_cast<unsigned int>(m_playerId),
            reinterpret_cast<int (*)(void*, int, C4JStorage::EMessageResult)>(CheckResetNetherReturned),
            reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
        return;
    }
    if (initData->mResetEnd && m_updateEndPending)
    {
        int parameters[2] = {static_cast<int>(IDS_DONT_RESET_THE_END), static_cast<int>(IDS_RESET_THE_END)};
        gConsoleUIController.RequestAlertMessage(
            static_cast<int>(IDS_RESET_THE_END_TITLE), static_cast<int>(IDS_RESET_THE_END_TEXT), parameters, 2,
            static_cast<unsigned int>(m_playerId),
            reinterpret_cast<int (*)(void*, int, C4JStorage::EMessageResult)>(CheckUpdateTheEndReturned),
            reinterpret_cast<void*>(GetCallbackUniqueId()), NULL, true);
        return;
    }
    m_resetNetherPending = true;
    m_updateEndPending = true;
    LaunchGame();
}

UIControl* UIScene_LoadMenu::GetMainPanel()
{
    return &m_mainPanel;
}

void UIScene_LoadMenu::SetEnablePlayGameButton(bool enabled)
{
    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
    {
        if (m_list.CheckElementExists(0x11))
            m_list.EnableItem(0x11, enabled, true);
        if (m_list.CheckElementExists(0x12))
            m_list.EnableItem(0x12, enabled, true);
        if (m_list.CheckElementExists(0x13))
            m_list.EnableItem(0x13, enabled, true);
    }
    else
        m_loadSettings.setEnable(enabled, true);
}

void UIScene_LoadMenu::HandleSaveDeviceRemoved(int)
{
    if (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame && m_levelGenerationOptions == NULL)
        navigateBack();
}

int UIScene_LoadMenu::getSceneType()
{
    return EUIScene_LoadMenu;
}
