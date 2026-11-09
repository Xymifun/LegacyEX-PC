#include "game/core/IUIScene_StartGame.hpp"
#include "game/core/StringIds.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcMultiplayerMenu.hpp"
#include "platform/pc/PcNetworkSession.hpp"
#endif

#include <algorithm>
#include <cwchar>
#include <string>
#include <vector>
#if defined(RECON_NATIVE)
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#endif

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/core/System.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/MiniGameOptionCache.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/UIScene_QuadrantSignin.hpp"
#include "game/ui/UIGroup.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRules.hpp"
#include "game/world/LevelRuleset.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "vendor/_NetworkGameInitData.hpp"

extern C_4JProfile* gProfile;
extern MiniGameOptionCache gMiniGameOptionCache;

namespace
{
struct StartGameSceneData
{
    int reserved0;
    int saveIndex;
    LevelGenerationOptions* generationOptions;
    void* saveDetails;
    EMiniGameId miniGameId;
};

}

IUIScene_StartGame::IUIScene_StartGame(int sceneType, void* data, UILayer* layer, bool customGame)
    : UIScene(sceneType, layer),
      m_texturePackList(),
      m_mainPanel(),
      m_texturePackName(),
      m_texturePackDescription(),
      m_texturePackIcon(),
      m_texturePackComparison(),
      m_difficultySlider(),
      m_gameModeSlider(),
      m_onlineGameCheckBox(),
      m_launchData(),
      m_texturePacks(),
      m_worldName()
{
    const StartGameSceneData* sceneData = static_cast<const StartGameSceneData*>(data);
    m_worldName = L"";
    m_customGame = customGame;
    m_state629 = true;
    m_saveIndex = 0;
    m_state62C = false;
    m_state62B = false;
    m_saveCorrupt = false;
    m_state614 = 0;
    m_touchBoxRebuild = false;
    m_isCreative = false;
    m_actionPending = false;
    m_signInPending = false;
    m_gameTypeId = GameType::SURVIVAL->getId();
    m_pendingSlot = -1;
    m_seed = 0;
    m_difficulty = 0;
    m_texturePackParentPack = NULL;
    m_launchData.mNaturalRegeneration = true;
    m_launchData.mPad = sceneType;
    m_levelGenerationOptions = sceneData->generationOptions;
    m_saveIndex = sceneData->saveIndex;
    m_launchData.mTNT = true;
    m_launchData.mDoDaylightCycle = true;
    m_launchData.mNewWorld = false;
    m_launchData.mMobGriefing = true;
    m_launchData.mBonusChest = false;
    m_launchData.mDoTileDrops = true;
    m_launchData.mDoMobSpawning = true;
    m_launchData.mKeepInventory = false;
    m_launchData.mStructures = true;
    m_launchData.mFlatWorld = false;
    m_launchData.mDoMobLoot = true;
    m_launchData.mHostPrivileges = false;
    m_launchData.mFireSpreads = true;
    m_launchData.mPvP = true;
    m_launchData.mTrustPlayers = true;

    const EMiniGameId miniGameId = sceneData->miniGameId;
    if (miniGameId == eMiniGameId_Any || sceneData->generationOptions == NULL)
    {
        if (m_customGame && sceneData->generationOptions != NULL)
        {
            m_launchData.mValue44 = 0;
        }
    }
    else
    {
        m_launchData.mHostMiniGame = true;
        gApp.getLevelRules()->getRulesByType(LevelRules::GetRuleTypeForMiniGame(miniGameId),
                                             &m_launchData.mLevelRulesetGroups, true);
        if (miniGameId != eMiniGameId_Lobby)
        {
            unsigned int mapSelection = 0xFFFFFFFFU;
            switch (miniGameId)
            {
            case eMiniGameId_Battle:
                mapSelection = GameSettings::GetBattleMiniGameMapSelection(m_playerId);
                break;
            case eMiniGameId_Tumble:
                mapSelection = GameSettings::GetTumbleMiniGameMapSelection();
                break;
            case eMiniGameId_Glide:
                mapSelection = GameSettings::GetGlideMiniGameMapSelection(m_playerId);
                break;
            default:
                break;
            }
            if (~mapSelection != 0)
            {
                std::vector<LevelRules::LevelRulesetGroup*>::iterator it = m_launchData.mLevelRulesetGroups.begin();
                while (it != m_launchData.mLevelRulesetGroups.end())
                {
                    LevelRules::LevelRulesetGroup* group = *it;
                    LevelGenerationOptions* options = group->rulesets.front()->getGenerationOptions();
                    if ((~mapSelection & (1U << options->getThemeId())) != 0
                        || (!options->getPack()->hasPurchasedPack()
                            && !gApp.mDLCManager.IsMiniGameTemporarilyFree(group->rulesets.front()->getRuleType())))
                    {
                        it = m_launchData.mLevelRulesetGroups.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
                if (m_launchData.mLevelRulesetGroups.empty())
                {
                    m_launchData.mHostMiniGame = true;
                    gApp.getLevelRules()->getRulesByType(static_cast<LevelRules::ELevelRuleType>(miniGameId),
                                                         &m_launchData.mLevelRulesetGroups, true);
                }
            }
        }
        MiniGameDef::SetPlayerSettings(gMiniGameOptionCache.getPreviousOptions(m_playerId, sceneData->miniGameId));
    }

    bool onlineSetting = GameSettings::GetGameSettings(m_playerId, eGameSetting_Online) != 0;
    m_launchData.mOnlineSettingChangedBySystem = false;
#ifdef MINECRAFT_PC
    m_launchData.mPublicWorldTitle = GameSettings::GetGameSettings(m_playerId, eGameSetting_PublicWorldTitle) != 0;
#endif
    bool allowedOnline = gProfile->IsSignedInLive(m_playerId) && gProfile->AllowedToPlayMultiplayer(m_playerId);
    m_onlineEnabled = allowedOnline;
    if (allowedOnline)
    {
        m_launchData.mOnlineGame = onlineSetting;
        if (onlineSetting)
        {
            m_launchData.mInviteOnly = GameSettings::GetGameSettings(m_playerId, eGameSetting_InviteOnly) != 0;
            m_launchData.mAllowFriendsOfFriends =
                GameSettings::GetGameSettings(m_playerId, eGameSetting_FriendsOfFriends) != 0;
            m_selectedSlot = 0;
            addTimer(0, 100);
        }
        else
        {
            m_selectedSlot = 0;
            m_launchData.mAllowFriendsOfFriends = false;
            m_launchData.mInviteOnly = false;
            addTimer(0, 100);
        }
    }
    else
    {
        m_launchData.mInviteOnly = false;
        m_launchData.mAllowFriendsOfFriends = false;
        m_launchData.mOnlineGame = false;
        if (onlineSetting)
        {
            m_launchData.mOnlineSettingChangedBySystem = true;
        }
        m_selectedSlot = 0;
        addTimer(0, 100);
    }
}

IUIScene_StartGame::~IUIScene_StartGame()
{
}

unsigned int IUIScene_StartGame::GetCustomGameModeID()
{
    return m_launchData.mMiniGameId;
}

unsigned int IUIScene_StartGame::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_mainPanel.setupControl(this, currentNode, std::string("MainPanel"));
        m_controls.push_back(&m_mainPanel);
    }
    {
        fuiRenderNode* parentNode = currentNode;
        currentNode = m_mainPanel.getFuiRenderNode();
        {
            m_texturePackName.setupControl(this, currentNode, std::string("TexturePackName"));
            m_controls.push_back(&m_texturePackName);
        }
        {
            m_texturePackDescription.setupControl(this, currentNode, std::string("TexturePackDescription"));
            m_controls.push_back(&m_texturePackDescription);
        }
        {
            m_texturePackIcon.setupControl(this, currentNode, std::string("Icon"));
            m_controls.push_back(&m_texturePackIcon);
        }
        {
            m_texturePackComparison.setupControl(this, currentNode, std::string("ComparisonPic"));
            m_controls.push_back(&m_texturePackComparison);
        }
        currentNode = parentNode;
    }
    return 1;
}

bool IUIScene_StartGame::SortTexturePacks(const TEXTUREPACKSORTSTRUCT& first, const TEXTUREPACKSORTSTRUCT& second)
{
    return second.sortValue < first.sortValue;
}

void IUIScene_StartGame::UpdateTexturePackDescription(int packId)
{
    if (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame)
    {
        TexturePack* pack = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(packId);
        if (pack != NULL)
        {
            m_texturePackName.setLabel(UIString(pack->getName()), false, false);
            m_texturePackDescription.setLabel(UIString(pack->getDesc1()), false, false);
            unsigned int size = 0;
            wchar_t buffer[64];
            pack->getPackIcon(size);
            swprintf(buffer, 0x40, L"tpack%08x", pack->getId());
            m_texturePackIcon.setTextureName(buffer, true);
            const void* comparison = pack->getPackComparison(size);
            if (size == 0 || comparison == NULL)
            {
                m_texturePackComparison.setTextureName(L"", true);
            }
            else
            {
                swprintf(buffer, 0x40, L"texturePackComparison%08x", pack->getId());
                registerSubstitutionTexture(buffer, reinterpret_cast<unsigned char*>(const_cast<void*>(comparison)),
                                            size, false);
                m_texturePackComparison.setTextureName(buffer, true);
            }
        }
    }
    else
    {
        Minecraft::GetInstance()->GetMiniGame();
        std::wstring image = MiniGameDef::getComparisonImageForGameMode(
            static_cast<EMiniGameId>(GetCustomGameModeID()));
        if (!gApp.hasArchiveFile(image, false))
        {
            m_texturePackComparison.setTextureName(L"", true);
        }
        else
        {
            arrayWithLength<unsigned char> file = gApp.getArchiveFile(image, false);
            if (file.length != 0 && file.data != NULL)
            {
                registerSubstitutionTexture(image, file.data, file.length, true);
            }
            m_texturePackComparison.setTextureName(image, true);
        }
    }
}

void IUIScene_StartGame::OrganizeTexturePacks()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    unsigned int count = minecraft->mTexturePackRepository->getTexturePackCount();
    m_texturePacks.resize(count, TEXTUREPACKSORTSTRUCT());
    m_texturePacks[0].sortValue = 1000;
    m_texturePacks[0].packId = 0;
    int sortValue = 999;
    unsigned int index;
    for (index = 1; index < count; ++index)
    {
        TexturePack* pack = minecraft->mTexturePackRepository->getTexturePackByIndex(index);
        unsigned int packId = pack->getId();
        DLC_INFO* info = gApp.GetDLCInfoInstance()->GetDLCInfoForPackID(packId);
        if (info == NULL)
        {
            DLCPack* parent = static_cast<DLCTexturePack*>(pack)->getDLCInfoParentPack();
            m_texturePacks[index].sortValue = sortValue;
            --sortValue;
            m_texturePacks[index].packId = parent->GetPackId();
        }
        else
        {
            m_texturePacks[index].sortValue = info->SortIndex;
            m_texturePacks[index].packId = packId;
        }
    }
    std::sort(m_texturePacks.begin(), m_texturePacks.end(), SortTexturePacks);
    for (index = 0; index < count; ++index)
    {
        TexturePack* pack = minecraft->mTexturePackRepository->getTexturePackById(m_texturePacks[index].packId);
#ifdef MINECRAFT_PC
        if (pack == NULL)
            continue;
#endif
        if (m_texturePacks[index].packId == m_launchData.mTexturePackId)
        {
            m_selectedSlot = index;
        }
        unsigned int size = 0;
        const void* icon = pack->getPackIcon(size);
        if (size != 0 && icon != NULL)
        {
            wchar_t buffer[64];
            swprintf(buffer, 0x40, L"tpack%08x", pack->getId());
            registerSubstitutionTexture(buffer, reinterpret_cast<unsigned char*>(const_cast<void*>(icon)), size, false);
            m_texturePackList.addPack(index, buffer, false);
        }
    }
    UpdateTexturePackDescription(m_launchData.mTexturePackId);
    m_texturePackList.selectSlot(m_selectedSlot);
}

void IUIScene_StartGame::_init()
{
    if (gApp.StartInstallDLCProcess(m_playerId) != 0)
    {
        m_actionPending = true;
        return;
    }
    m_actionPending = false;
    if (m_levelGenerationOptions == NULL || !m_levelGenerationOptions->isGameModeLobby())
    {
        OrganizeTexturePacks();
        return;
    }
    UpdateTexturePackDescription(0);
}

void IUIScene_StartGame::RequestPrivMessageBox()
{
    int parameters[1];
    parameters[0] = IDS_CONFIRM_OK;
    int pad = gProfile->GetPrimaryPad();
    if (m_launchData.mJoinMiniGame)
    {
        gConsoleUIController.RequestAlertMessage(0x456F1349, 0x2FAC754A, parameters, 1, pad, NULL, NULL, NULL, true);
        return;
    }
    gConsoleUIController.RequestAlertMessage(0x252AB13F, -0x3B844EA3, parameters, 1, pad, NULL, NULL, NULL, true);
}

void IUIScene_StartGame::FindAndJoinGame(unsigned int playerMask)
{
    EMiniGameId miniGameId = m_launchData.mMiniGameId;
    if (m_launchData.mMiniGameId != eMiniGameId_Any)
    {
        gApp.SetGameHostOption(eGameHostOption_MiniGame, 1);
        gApp.SetFirstMiniGameType((MiniGameDef::GetCustomGameModeById(m_launchData.mMiniGameId, true)->GetId()));
    }
    unsigned int* values = NULL;
    unsigned int* ids = NULL;
    unsigned int count = MiniGameDef::GetCustomGameModeById(m_launchData.mMiniGameId, true)
                             ->CalculateJoiningMatchmakingData(&m_launchData, &values, &ids);
    unsigned int result = CGameNetworkManager::sInstance.FindAndJoinGame(static_cast<int>(playerMask), miniGameId,
                                                                         count, values, ids);
    delete values;
    delete ids;
    gApp.SetLiveLinkRequired(false);
    switch (result)
    {
    case 2:
    {
        int parameters[1];
        parameters[0] = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(IDS_CONNECTION_FAILED, -0x479419FD, parameters, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        gConsoleUIController.NavigateToHomeMenu(false);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        break;
    }
    case 3:
    {
        int parameters[1];
        parameters[0] = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(IDS_CONNECTION_FAILED, -0x554D4923, parameters, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        gConsoleUIController.NavigateToHomeMenu(false);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        break;
    }
    case 4:
    {
        int parameters[1];
        parameters[0] = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(0x252AB13F, -0x33A5A6F1, parameters, 1, gProfile->GetPrimaryPad(),
                                                 NULL, NULL, NULL, true);
        gConsoleUIController.NavigateToHomeMenu(false);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        break;
    }
    case 1:
#ifdef MINECRAFT_PC
        m_actionPending = false;
        setVisible(true);
        PcShowAlertSoon(IDS_MG_PUBLIC_GAME_TITLE, IDS_NO_GAMES_FOUND, gProfile->GetPrimaryPad());
#else
        gConsoleUIController.NavigateBack(m_playerId, false, EUIScene_None, EUILayer_Count);
#endif
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        break;
    case 0:
        telemetryManagerInstance->PrepareMiniGameHostOptions(m_playerId, &m_launchData,
                                                             eTelemetryJoinMethod_Matchmaking);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        break;
    default:
        gConsoleUIController.NavigateBack(m_playerId, false, EUIScene_None, EUILayer_Count);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        break;
    }
}

void IUIScene_StartGame::HostGame(unsigned int playerMask)
{
    _NetworkGameInitData* initData = new _NetworkGameInitData();
    if (m_customGame)
    {
        if (m_levelGenerationOptions == NULL)
        {
            initData->m_recordLevelResume = 1;
            gApp.ClearTerrainFeaturePosition();
        }
        else
        {
            gStorage->ResetSaveData();
            gStorage->SetSaveTitle(m_levelGenerationOptions->getDefaultSaveName().c_str());
            gApp.ClearTerrainFeaturePosition();
        }
    }
    else
    {
        {
            std::wstring saveName;
            saveName.assign(m_worldName, 0, std::wstring::npos);
            gStorage->ResetSaveData();
            gStorage->SetSaveTitle(saveName.c_str());
            std::wstring seedText;
            if (m_launchData.mSeed.size() != 0)
            {
                seedText.assign(m_launchData.mSeed, 0, std::wstring::npos);
            }
            else
            {
                seedText.assign(L"", wcslen(L""));
            }
            if (seedText.size() != 0)
            {
                const unsigned int length = seedText.size();
                bool numeric = length != 0;
                unsigned int i;
                for (i = 0; i < length; ++i)
                {
                    if ((seedText.at(i) < L'0' || seedText.at(i) > L'9') && (i != 0 || seedText.at(0) != L'-'))
                    {
                        numeric = false;
                        break;
                    }
                }
                if (numeric)
                {
                    m_seed = _fromString<long long>(seedText);
                }
                else
                {
                    int hash = 0;
                    for (i = 0; i < length; ++i)
                    {
                        hash = hash * 31 + seedText.at(i);
                    }
                    m_seed = hash;
                }
            }
            else if (m_launchData.mFindBalancedSeed)
            {
                initData->m_findSeed = true;
            }
            else
            {
                Random* random = new Random(System::processTimeInNanoSecs());
                m_seed = random->nextLong();
#if defined(RECON_NATIVE)
                const char* fixture = std::getenv("MC_WORLD_SEED");
                if (fixture != NULL && *fixture != '\0')
                {
                    char* end = NULL;
                    errno = 0;
                    const long long fixtureSeed = std::strtoll(fixture, &end, 10);
                    if (errno == 0 && end != fixture && *end == '\0')
                        m_seed = fixtureSeed;
                    else
                        std::fprintf(stderr, "[native-world] invalid MC_WORLD_SEED: %s\n", fixture);
                }
#endif
            }
        }
        gApp.ClearTerrainFeaturePosition();
    }

    bool onlineGame = false;
    if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad()) && (m_launchData.mOnlineGame || m_launchData.mMatchmaking)
        && !m_launchData.mTimeTrial)
    {
        onlineGame = true;
    }
    initData->m_seed = m_seed;
    initData->m_levelGenerationOptions = m_levelGenerationOptions;
    initData->m_texturePackId = m_launchData.mTexturePackId;
    initData->m_saveData = 0;
    initData->m_gameType = m_gameTypeId;
    Minecraft::GetInstance()->mTexturePackRepository->selectTexturePackById(m_launchData.mTexturePackId, false);
    gApp.SetGameHostOption(eGameHostOption_Difficulty, m_difficulty);
    gApp.SetGameHostOption(eGameHostOption_FriendsOfFriends, m_launchData.mAllowFriendsOfFriends);
    gApp.SetGameHostOption(eGameHostOption_Gamertags,
                           GameSettings::GetGameSettings(m_playerId, eGameSetting_DisplayMii));
    gApp.SetGameHostOption(eGameHostOption_GameType, m_gameTypeId);
    gApp.SetGameHostOption(eGameHostOption_LevelType, m_launchData.mFlatWorld);
    gApp.SetGameHostOption(eGameHostOption_Structures, m_launchData.mStructures);
    gApp.SetGameHostOption(eGameHostOption_BonusChest, m_launchData.mBonusChest);
    gApp.SetGameHostOption(eGameHostOption_PvP, m_launchData.mPvP);
    gApp.SetGameHostOption(eGameHostOption_TrustPlayers, m_launchData.mTrustPlayers);
    gApp.SetGameHostOption(eGameHostOption_FireSpreads, m_launchData.mFireSpreads);
    gApp.SetGameHostOption(eGameHostOption_TNT, m_launchData.mTNT);
    gApp.SetGameHostOption(eGameHostOption_HostCanFly, m_launchData.mHostPrivileges);
    gApp.SetGameHostOption(eGameHostOption_HostCanChangeHunger, m_launchData.mHostPrivileges);
    gApp.SetGameHostOption(eGameHostOption_HostCanBeInvisible, m_launchData.mHostPrivileges);
    gApp.SetGameHostOption(eGameHostOption_MobGriefing, m_launchData.mMobGriefing);
    gApp.SetGameHostOption(eGameHostOption_KeepInventory, m_launchData.mKeepInventory);
    gApp.SetGameHostOption(eGameHostOption_DoMobSpawning, m_launchData.mDoMobSpawning);
    gApp.SetGameHostOption(eGameHostOption_DoMobLoot, m_launchData.mDoMobLoot);
    gApp.SetGameHostOption(eGameHostOption_DoTileDrops, m_launchData.mDoTileDrops);
    gApp.SetGameHostOption(eGameHostOption_NaturalRegeneration, m_launchData.mNaturalRegeneration);
    gApp.SetGameHostOption(eGameHostOption_DoDaylightCycle, m_launchData.mDoDaylightCycle);
    gApp.SetGameHostOption(eGameHostOption_DoWeatherCycle, m_launchData.mDoWeatherCycle);
    if (m_launchData.mFlatWorld)
    {
        initData->m_levelTypeOptions = new SuperflatConfig(m_launchData.mSuperflatConfig);
    }
    if (!m_customGame)
    {
#ifdef MINECRAFT_PC
        static const int kWorldWidths[] = {54, 64, 192, 320, 640};
        static const int kHellScales[] = {3, 3, 6, 8, 8};
        int worldSize = m_launchData.mWorldSize < 1 || m_launchData.mWorldSize > 5 ? 1 : m_launchData.mWorldSize;
        initData->m_xzSize = kWorldWidths[worldSize - 1];
        initData->m_hellScale = kHellScales[worldSize - 1];
        gApp.SetGameHostOption(eGameHostOption_WorldSize, worldSize);
#else
        initData->m_xzSize = 54;
        initData->m_hellScale = 3;
#endif
        initData->m_biomeScale = m_launchData.mBiomeScale;
        initData->m_biomeCentreXChunk = m_launchData.mBiomeCentreXChunk;
        initData->m_biomeCentreZChunk = m_launchData.mBiomeCentreZChunk;
    }
    gApp.SetResetNether(m_launchData.mResetNether);
    gApp.SetResetEnd(m_launchData.mResetEnd);
    gApp.SetGameHostOption(eGameHostOption_GameType, m_gameTypeId);
    unsigned char gameType = static_cast<unsigned char>(m_gameTypeId);
    if (m_launchData.mHostMiniGame || m_launchData.mJoinMiniGame)
    {
        gApp.SetGameHostOption(eGameHostOption_MiniGame, 1);
        gameType = static_cast<unsigned char>(GameType::LOBBY->getId());
    }
    EMiniGameId hostMiniGameId = eMiniGameId_Any;
    if (m_launchData.mMiniGameId != eMiniGameId_Any)
    {
        hostMiniGameId = m_launchData.mMiniGameId;
    }
    unsigned int matchmakingCount = 0;
    unsigned int* matchmakingValues = NULL;
    if (m_launchData.mMiniGameId != eMiniGameId_Any)
    {
        MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(m_launchData.mMiniGameId, true);
        gApp.SetFirstMiniGameType(definition->GetId());
        Minecraft::GetInstance()->GetClientMasterGameMode()->Setup(definition->GetId());
        definition->SetGameSettingsFromMatchmakingFilter(&m_launchData);
        if (m_launchData.mMatchmaking)
        {
            matchmakingCount = definition->CalculateHostingMatchmakingData(&m_launchData, &matchmakingValues);
        }
        initData->m_ruleGroups.insert(initData->m_ruleGroups.begin(), m_launchData.mLevelRulesetGroups.begin(),
                                      m_launchData.mLevelRulesetGroups.end());
    }
#ifdef MINECRAFT_PC
    PcNetworkSession::SetPublicWorldTitle(
        onlineGame && m_launchData.mPublicWorldTitle && m_launchData.mMiniGameId == eMiniGameId_Any
            ? gStorage->GetSaveTitle()
            : L"");
#endif
    CGameNetworkManager::sInstance.HostGame(static_cast<int>(playerMask), onlineGame, m_launchData.mInviteOnly,
                                            gameType, hostMiniGameId, matchmakingCount, matchmakingValues);
    delete matchmakingValues;
    initData->m_hostOptions = gApp.GetGameHostOption(eGameHostOption_All);
    initData->m_miniGameHostOptions = MiniGameDef::GetCustomGameModeById(m_launchData.mMiniGameId, true)
                                          ->GetHostOption(eCustomGameModeHostOption_All);
    CGameNetworkManager::sInstance.FakeLocalPlayerJoined();
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_threadProc = &CGameNetworkManager::RunNetworkGameThreadProc;
    params->m_threadParam = initData;
    if (m_launchData.mMiniGameId != eMiniGameId_Any)
    {
        params->m_miniGameId = m_launchData.mMiniGameId;
    }
    gApp.SetAutosaveTimerTime();
    FullscreenProgressState* state = new FullscreenProgressState;
    params->m_completionData = state;
    state->closeMode = 3;
    state->playerId = 0;
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
    telemetryManagerInstance->PrepareMiniGameHostOptions(gProfile->GetPrimaryPad(), &m_launchData,
                                                         eTelemetryJoinMethod_Host);
    Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
}

void IUIScene_StartGame::StartGame(unsigned int playerMask)
{
    if (m_launchData.mJoinMiniGame)
    {
        FindAndJoinGame(playerMask);
        return;
    }
    HostGame(playerMask);
}

int IUIScene_StartGame::StartGame_SignInReturned(void* context, bool signedIn, int)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (!signedIn)
    {
        scene->m_actionPending = false;
        return 0;
    }
    if (!gProfile->IsSignedIn(scene->m_playerId))
    {
        return 0;
    }
    int primaryPad = gProfile->GetPrimaryPad();
    bool multiplayerBlocked = false;
    unsigned int playerMask = 0;
    bool allSignedInLive = gProfile->IsSignedInLive(primaryPad);
    bool onlineGame = false;
    if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad())
        && (scene->m_launchData.mOnlineGame || scene->m_launchData.mMatchmaking) && !scene->m_launchData.mTimeTrial)
    {
        onlineGame = true;
    }
    bool localMultiplayer = gApp.IsLocalMultiplayerAvailable();
    int index;
    for (index = 0; index < 4; ++index)
    {
        if (gProfile->IsSignedIn(index) && (index == primaryPad || localMultiplayer))
        {
            if (allSignedInLive)
            {
                gProfile->IsSignedInLive(index);
            }
            if (!gProfile->AllowedToPlayMultiplayer(index))
            {
                multiplayerBlocked = true;
            }
            playerMask |= CGameNetworkManager::GetLocalPlayerMask(index);
            allSignedInLive = allSignedInLive && gProfile->IsSignedInLive(index);
        }
    }
    if (onlineGame && !allSignedInLive)
    {
        scene->m_actionPending = false;
        int parameters[1];
        parameters[0] = IDS_CONFIRM_OK;
        gConsoleUIController.RequestAlertMessage(-0x75DE76B5, -0x47C8FDB2, parameters, 1, gProfile->GetPrimaryPad(),
                                                 NULL, NULL, NULL, true);
        return 0;
    }
    bool ugcRestricted = scene->IsUGCRestrictedMode();
    if (!allSignedInLive || !onlineGame || (!multiplayerBlocked && !ugcRestricted))
    {
        scene->StartGame(playerMask);
        return 0;
    }
    scene->m_actionPending = false;
    scene->setVisible(true);
    if (!ugcRestricted)
    {
        scene->RequestPrivMessageBox();
        return 0;
    }
    gConsoleUIController.RequestUGCMessageBox(-1, -1, -1, NULL, NULL);
    return 0;
}

void IUIScene_StartGame::tick()
{
    UIScene::tick();
    if (m_pendingSlot >= 0)
    {
        UpdateTexturePackDescription(m_texturePacks[m_pendingSlot].packId);
        int slot = m_pendingSlot;
        m_pendingSlot = -1;
        m_selectedSlot = slot;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    }
    if (m_signInPending)
    {
        UIScene_QuadrantSignin::SignInInfo request;
        request.callback = StartGame_SignInReturned;
        request.context = this;
        request.onlineGame = m_launchData.mOnlineGame;
        request.returnToHome = false;
        m_signInPending = false;
        gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_QuadrantSignin, &request,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
    }
}

void IUIScene_StartGame::handleReload()
{
    if (m_selectedSlot != -1)
    {
        UpdateTexturePackDescription(m_texturePacks[m_selectedSlot].packId);
        m_texturePackList.selectSlot(m_selectedSlot);
    }
}

void IUIScene_StartGame::handleSelectionChanged(int selection)
{
    m_pendingSlot = selection;
}

void IUIScene_StartGame::handleSliderMove(int id, int value)
{
    switch (id)
    {
    case 2:
        m_difficulty = value;
        m_difficultySlider.handleSliderMove(value);
        if (!m_customGame)
        {
            GameSettings::SetGameSettings(m_playerId, eGameSetting_Difficulty, static_cast<unsigned char>(value));
        }
        wchar_t buffer[256];
        swprintf(buffer, 0x100, L"%ls: %ls", L10N::GetString(IDS_SLIDER_DIFFICULTY),
                 L10N::GetString(Difficulty::byId(value)->getKey()));
        m_difficultySlider.setLabel(UIString(buffer), false, false);
        break;
    case 1:
        m_gameModeSlider.handleSliderMove(value);
        switch (value)
        {
        case 0:
            m_gameModeSlider.setLabel(UIString(L10N::GetString(IDS_GAMEMODE_SURVIVAL)), false, false);
            m_gameTypeId = GameType::SURVIVAL->getId();
            m_isCreative = false;
            break;
        case 1:
            m_gameModeSlider.setLabel(UIString(L10N::GetString(IDS_GAMEMODE_CREATIVE)), false, false);
            m_gameTypeId = GameType::CREATIVE->getId();
            m_isCreative = true;
            break;
        case 2:
            m_gameModeSlider.setLabel(UIString(L10N::GetString(IDS_GAMEMODE_ADVENTURE)), false, false);
            m_gameTypeId = GameType::ADVENTURE->getId();
            m_isCreative = false;
            break;
        }
        break;
    }
}

void IUIScene_StartGame::handleTouchBoxRebuild()
{
    m_touchBoxRebuild = true;
}

void IUIScene_StartGame::handleTimerComplete(int timerId)
{
    if (m_touchBoxRebuild)
    {
        GetMainPanel()->UpdateControl();
        gConsoleUIController.TouchBoxRebuild(this);
        m_touchBoxRebuild = false;
    }
    if (timerId == 0)
    {
        bool allowedOnline = gProfile->IsSignedInLive(m_playerId) && gProfile->AllowedToPlayMultiplayer(m_playerId);
        if (allowedOnline != m_onlineEnabled)
        {
            if (allowedOnline)
            {
                m_launchData.mOnlineGame = GameSettings::GetGameSettings(m_playerId, eGameSetting_Online) != 0;
                if (m_launchData.mOnlineGame)
                {
                    m_launchData.mInviteOnly = GameSettings::GetGameSettings(m_playerId, eGameSetting_InviteOnly) != 0;
                    m_launchData.mAllowFriendsOfFriends =
                        GameSettings::GetGameSettings(m_playerId, eGameSetting_FriendsOfFriends) != 0;
                    m_onlineGameCheckBox.SetEnable(allowedOnline);
                    m_onlineGameCheckBox.setChecked(m_launchData.mOnlineGame);
                    m_onlineEnabled = allowedOnline;
                }
                else
                {
                    m_launchData.mAllowFriendsOfFriends = false;
                    m_launchData.mInviteOnly = false;
                    m_onlineGameCheckBox.SetEnable(allowedOnline);
                    m_onlineGameCheckBox.setChecked(m_launchData.mOnlineGame);
                    m_onlineEnabled = allowedOnline;
                }
            }
            else
            {
                m_launchData.mAllowFriendsOfFriends = false;
                m_launchData.mInviteOnly = false;
                m_launchData.mOnlineGame = false;
                m_onlineGameCheckBox.SetEnable(allowedOnline);
                m_onlineGameCheckBox.setChecked(m_launchData.mOnlineGame);
                m_onlineEnabled = allowedOnline;
            }
        }
    }
}

void IUIScene_StartGame::HandleDLCMountingComplete()
{
    if ((m_levelGenerationOptions == NULL || !m_levelGenerationOptions->isGameModeLobby()) && m_texturePacks.empty())
    {
        OrganizeTexturePacks();
        m_selectedSlot = 0;
    }
    else
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        m_texturePackList.clearSlots();
        unsigned int count = minecraft->mTexturePackRepository->getTexturePackCount();
        unsigned int index;
        for (index = 0; index < count; ++index)
        {
            TexturePack* pack = minecraft->mTexturePackRepository->getTexturePackByIndex(index);
            unsigned int size = 0;
            const void* icon = pack->getPackIcon(size);
            if (size != 0 && icon != NULL)
            {
                wchar_t buffer[64];
                swprintf(buffer, 0x40, L"tpack%08x", pack->getId());
                registerSubstitutionTexture(buffer, reinterpret_cast<unsigned char*>(const_cast<void*>(icon)), size,
                                            false);
                m_texturePackList.addPack(index, buffer, false);
            }
        }
        m_state614 = 0;
        m_selectedSlot = 0;
    }
    if (m_texturePacks.empty())
    {
        UpdateTexturePackDescription(0);
    }
    else
    {
        UpdateTexturePackDescription(m_texturePacks[0].packId);
    }
    m_texturePackList.selectSlot(m_selectedSlot);
    m_actionPending = false;
    gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
}

void IUIScene_StartGame::UpdateCurrentTexturePack(int slot)
{
    m_selectedSlot = slot;
    TexturePack* pack = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(
        m_texturePacks[slot].packId);
    if (pack != NULL)
    {
        m_launchData.mTexturePackId = pack->getId();
    }
}

int IUIScene_StartGame::TrialTexturePackWarningReturned(void* context, int, C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_Accept)
    {
        scene->checkStateAndStartGame();
        return 0;
    }
    scene->m_actionPending = false;
    return 0;
}

int IUIScene_StartGame::UnlockTexturePackReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_Accept)
    {
        gProfile->IsSignedIn(userIndex);
    }
    scene->m_actionPending = false;
    return 0;
}

int IUIScene_StartGame::TexturePackDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_Accept && gProfile->IsSignedIn(userIndex))
    {
        if (gProfile->IsSignedInLive(userIndex))
        {
            if (!gApp.GetCommerceInstance()->PurchaseDLCByPackID(scene->m_launchData.mTexturePackId))
            {
                int parameters[1];
                parameters[0] = IDS_CONFIRM_OK;
                gConsoleUIController.RequestErrorMessage(IDS_DLC_ALREADY_INSTALLED_TITLE,
                                                         IDS_DLC_ALREADY_INSTALLED_TEXT, parameters, 1, userIndex, NULL,
                                                         NULL, NULL, true);
            }
        }
        else
        {
            int parameters[1];
            parameters[0] = IDS_CONFIRM_OK;
            gConsoleUIController.RequestErrorMessage(-0x75DE76B5, -0x3CB3F489, parameters, 1, userIndex, NULL, NULL,
                                                     NULL, true);
        }
    }
    scene->m_actionPending = false;
    return 0;
}

int IUIScene_StartGame::TexturePackDialogBattleMiniGameReturned(void* context, int userIndex,
                                                                C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_Accept && gProfile->IsSignedIn(userIndex))
    {
        if (gProfile->IsSignedInLive(userIndex))
        {
            if (!gApp.GetCommerceInstance()->PurchaseDLCByPackID(scene->m_launchData.mTexturePackId))
            {
                int parameters[1];
                parameters[0] = IDS_CONFIRM_OK;
                gConsoleUIController.RequestErrorMessage(IDS_DLC_ALREADY_INSTALLED_TITLE,
                                                         IDS_DLC_ALREADY_INSTALLED_TEXT, parameters, 1, userIndex, NULL,
                                                         NULL, NULL, true);
            }
        }
        else
        {
            int parameters[1];
            parameters[0] = IDS_CONFIRM_OK;
            gConsoleUIController.RequestErrorMessage(-0x75DE76B5, -0x3CB3F489, parameters, 1, userIndex, NULL, NULL,
                                                     NULL, true);
        }
    }
    scene->m_actionPending = false;
    return 0;
}

void IUIScene_StartGame::CheckPermissionsAndContinue(bool fromLoad)
{
    int primaryPad = gProfile->GetPrimaryPad();
    bool multiplayerBlocked = false;
    bool allSignedInLive = true;
    bool onlineGame = (m_launchData.mOnlineGame || m_launchData.mMatchmaking) && !m_launchData.mTimeTrial;
    bool localMultiplayer = gApp.IsLocalMultiplayerAvailable();
    int index;
    for (index = 0; index < 4; ++index)
    {
        if (gProfile->IsSignedIn(index) && (index == primaryPad || localMultiplayer))
        {
            if (allSignedInLive)
            {
                gProfile->IsSignedInLive(index);
            }
            if (!gProfile->AllowedToPlayMultiplayer(index))
            {
                multiplayerBlocked = true;
            }
            allSignedInLive = allSignedInLive && gProfile->IsSignedInLive(index);
        }
    }
    if (onlineGame && !allSignedInLive)
    {
        m_actionPending = false;
        int parameters[1];
        parameters[0] = IDS_CONFIRM_OK;
        gConsoleUIController.RequestAlertMessage(-0x75DE76B5, -0x47C8FDB2, parameters, 1, gProfile->GetPrimaryPad(),
                                                 NULL, NULL, NULL, true);
        return;
    }
    bool ugcRestricted = IsUGCRestrictedMode();
    if (onlineGame)
    {
        if (ugcRestricted)
        {
            setVisible(true);
            gConsoleUIController.RequestUGCMessageBox(-1, -1, -1, NULL, NULL);
            m_actionPending = false;
            return;
        }
    }
    else if (!m_launchData.mJoinMiniGame)
    {
        StartGame(0);
        return;
    }
    if (allSignedInLive && gApp.IsLocalMultiplayerAvailable() && fromLoad)
    {
        if (ugcRestricted)
        {
            m_actionPending = false;
            gConsoleUIController.RequestUGCMessageBox(-1, -1, -1, NULL, NULL);
            return;
        }
        if (gApp.IsDrcPreventingSplitscreen())
        {
            int parameters[2];
            parameters[0] = 0x1A65D275;
            parameters[1] = 0x7A458ED2;
            gConsoleUIController.RequestAlertMessage(0x337F2CFC, 0x6F86265B, parameters, 2, gProfile->GetPrimaryPad(),
                                                     WiiUDRCWarningReturned, this, NULL, true);
            return;
        }
        m_signInPending = true;
        return;
    }
    if (onlineGame && allSignedInLive && (ugcRestricted || multiplayerBlocked))
    {
        m_actionPending = false;
        setVisible(true);
        if (!ugcRestricted)
        {
            RequestPrivMessageBox();
            return;
        }
        gConsoleUIController.RequestUGCMessageBox(-1, -1, -1, NULL, NULL);
        return;
    }
    StartGame(0);
}

int IUIScene_StartGame::WiiUDRCWarningReturned(void* context, int, C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result != C4JStorage::EMessageResult_Decline && result == C4JStorage::EMessageResult_Accept)
    {
        scene->CheckPermissionsAndContinue(false);
    }
    scene->m_actionPending = false;
    return 0;
}

void IUIScene_StartGame::StartSharedLaunchFlow()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    m_actionPending = true;
    if (!m_launchData.mHostMiniGame && !m_launchData.mJoinMiniGame)
    {
        if (!m_launchData.mOnlineSettingChangedBySystem)
        {
            int online = 1;
            if (!m_launchData.mOnlineGame && !m_launchData.mMatchmaking)
            {
                online = 0;
            }
            GameSettings::SetGameSettings(m_playerId, eGameSetting_Online, online);
        }
        GameSettings::SetGameSettings(m_playerId, eGameSetting_InviteOnly, m_launchData.mInviteOnly);
        GameSettings::SetGameSettings(m_playerId, eGameSetting_FriendsOfFriends, m_launchData.mAllowFriendsOfFriends);
#ifdef MINECRAFT_PC
        GameSettings::SetGameSettings(m_playerId, eGameSetting_PublicWorldTitle, m_launchData.mPublicWorldTitle);
#endif
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
    }
    else
    {
        gMiniGameOptionCache.updateCachedOptions(m_playerId, m_launchData.mMiniGameId, m_launchData.mPlayerSettings);
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
    }
    if (m_customGame)
    {
        gApp.SetGameHostOption(eGameHostOption_WasntSaveOwner, m_state629 ? 0 : 1);
    }
    RiderPositionOverrides::ClearOverrides();
    if (m_launchData.mTexturePackId != 0)
    {
        TexturePack* pack = minecraft->mTexturePackRepository->getTexturePackById(m_launchData.mTexturePackId);
        if (pack != NULL)
        {
            m_texturePackParentPack = static_cast<DLCTexturePack*>(pack)->getDLCInfoParentPack();
            if (m_texturePackParentPack != NULL && !m_texturePackParentPack->hasPurchasedPack())
            {
                int parameters[2];
                parameters[0] = IDS_CONFIRM_OK;
                parameters[1] = IDS_CONFIRM_CANCEL;
                if (m_launchData.mHostMiniGame)
                {
                    gConsoleUIController.RequestAlertMessage(0x3BB3096E, -0x0AF6FB3A, parameters, 2, m_playerId,
                                                             TexturePackDialogReturned, this, NULL, true);
                }
                else
                {
                    gConsoleUIController.RequestAlertMessage(0x3BB3096E, 0x1CFE436F, parameters, 2, m_playerId,
                                                             TrialTexturePackWarningReturned, this, NULL, true);
                }
                gApp.SetExtraGameData(eGameExtraData_TrialTexturePack, 1);
                return;
            }
        }
    }
    if (!gProfile->IsFullVersion())
    {
        gApp.SetExtraGameData(eGameExtraData_TrialTexturePack, 0);
    }
    checkStateAndStartGame();
}

int IUIScene_StartGame::DeleteSaveDataReturned(void* context, bool)
{
    gApp.SetCorruptSaveDeleted(true);
    static_cast<IUIScene_StartGame*>(context)->navigateBack();
    return 0;
}

int IUIScene_StartGame::DeleteSaveDialogReturned(void* context, int, C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_Decline)
    {
        SAVE_DETAILS* details = gStorage->ReturnSavesInfo();
        gStorage->DeleteSaveData(&details->saves[scene->m_saveIndex], DeleteSaveDataReturned, scene);
        return 0;
    }
    scene->m_actionPending = false;
    return 0;
}

int IUIScene_StartGame::LoadDataComplete(void* context)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (!scene->m_saveCorrupt)
    {
        scene->CheckPermissionsAndContinue(true);
        return 0;
    }
    int parameters[2];
    parameters[0] = IDS_CONFIRM_CANCEL;
    scene->m_actionPending = false;
    parameters[1] = IDS_CONFIRM_OK;
    gConsoleUIController.RequestAlertMessage(-0x75879076, -0x637D3399, parameters, 2, scene->m_playerId,
                                             DeleteSaveDialogReturned, scene, NULL, true);
    return 0;
}

int IUIScene_StartGame::LoadSaveDataReturned(void* context, bool corrupt, bool complete)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    scene->m_saveCorrupt = corrupt;
    if (complete)
    {
        LoadDataComplete(context);
    }
    else
    {
        scene->m_actionPending = false;
    }
    return 0;
}

int IUIScene_StartGame::CheckCreativeReturned(void* context, int, C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_Accept)
    {
        if (scene->m_levelGenerationOptions != NULL)
        {
            scene->m_saveCorrupt = false;
            LoadDataComplete(scene);
        }
        else if (scene->m_customGame && !scene->m_launchData.mJoinMiniGame)
        {
            SAVE_DETAILS* details = gStorage->ReturnSavesInfo();
            gStorage->LoadSaveData(&details->saves[scene->m_saveIndex], LoadSaveDataReturned, scene);
        }
        else
        {
            scene->CheckPermissionsAndContinue(true);
        }
    }
    else
    {
        scene->m_actionPending = false;
    }
    return 0;
}

void IUIScene_StartGame::LaunchGame()
{
    int parameters[2];
    parameters[0] = IDS_CONFIRM_OK;
    parameters[1] = IDS_CONFIRM_CANCEL;
    if (!m_isCreative)
    {
        if (m_state62C)
        {
            gConsoleUIController.RequestAlertMessage(IDS_TITLE_START_GAME, 0x7BC65289, parameters, 2, m_playerId,
                                                     CheckCreativeReturned, this, NULL, true);
            return;
        }
        if (m_state62B)
        {
            gConsoleUIController.RequestAlertMessage(IDS_TITLE_START_GAME, IDS_CONFIRM_START_SAVEDINCREATIVE,
                                                     parameters, 2, m_playerId, CheckCreativeReturned, this, NULL,
                                                     true);
            return;
        }
    }
    else
    {
        if (!m_state62B || m_state62C)
        {
            gConsoleUIController.RequestAlertMessage(IDS_TITLE_START_GAME, IDS_CONFIRM_START_CREATIVE, parameters, 2,
                                                     m_playerId, CheckCreativeReturned, this, NULL, true);
            return;
        }
    }
    if (m_launchData.mFlatWorld && !m_state62B)
    {
        gConsoleUIController.RequestAlertMessage(IDS_TITLE_START_GAME, -0x6C429A80, parameters, 2, m_playerId,
                                                 CheckCreativeReturned, this, NULL, true);
        return;
    }
    if (m_launchData.mHostPrivileges)
    {
        gConsoleUIController.RequestAlertMessage(IDS_TITLE_START_GAME, 0x020D77D5, parameters, 2, m_playerId,
                                                 CheckCreativeReturned, this, NULL, true);
        return;
    }
    CheckCreativeReturned(this, m_playerId, C4JStorage::EMessageResult_Accept);
}

int IUIScene_StartGame::getCurrentTextureSelectorIndex(int packId)
{
    if (m_texturePacks.empty())
    {
        return 0;
    }
    int result = 0;
    int count = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackCount();
    int index;
    for (index = 0; index < count; ++index)
    {
        if (m_texturePacks[index].packId == packId)
        {
            result = index;
            break;
        }
    }
    return result;
}
