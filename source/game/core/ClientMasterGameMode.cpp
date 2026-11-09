#include "game/core/ClientMasterGameMode.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGlide.hpp"
#endif
#include "game/core/EntityType.hpp"
#include "game/core/StringIds.hpp"

#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/core/CameraController.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/ClientVoteables.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/RegionFileCache.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/Mth.hpp"
#include "game/core/System.hpp"
#include "game/core/Team.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/Voteable.hpp"
#include "game/item/Inventory.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRules.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/generation/GhostPlayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/ui/UIScene_FullscreenProgress.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include <algorithm>
#include <boost/unordered_set.hpp>

namespace
{
}

int WaitForNextState(void* context)
{
    ClientMasterGameMode* gameMode = static_cast<ClientMasterGameMode*>(context);
    while (gameMode->IsRoundRestarting())
        Sleep(5);
    return 0;
}

ClientMasterGameMode::ClientMasterGameMode()
{
    mGameModeState = eGameModeState_None;
    mBRUPsRemaining = 0;
    mSubRoundLoadStartTime = 0x7fffffff;
    mBRUPCount = 0;
    mNextLevelGenOptions = NULL;
    mLoadedChunkCount = 0;
    mNewLevelDataPending = false;

    mVoteables = new ClientVoteables();
    mVoteables->addVoteable(new Voteable(Voteables::eVoteable_Map, false, true));
    mVoteables->addVoteable(new Voteable(Voteables::eVoteable_Kick, false, false));
    mVoteables->addVoteable(new Voteable(Voteables::eVoteable_BuildOffScoring, false, false));
    mVoteables->addVoteable(new Voteable(Voteables::eVoteable_BuildOffTheme, false, true));
    ResetReadyStates();

    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        mLocalPlayerInRound[playerIndex] = false;
    mLocalRoundTimeStart = 0;
    mServerIsWaitingOnChunkLoading = false;
    mLocalChunksLoaded = false;
}

ClientMasterGameMode::~ClientMasterGameMode()
{
    delete mVoteables;
}

bool ClientMasterGameMode::IsRoundRestarting() const
{
    return mGameModeState == eGameModeState_LoadingSubRound;
}

void ClientMasterGameMode::ResetReadyStates()
{
    std::memset(mPlayerReady, 0, sizeof(mPlayerReady));
}

void ClientMasterGameMode::Reset()
{
    mRoundInfo.reset();
    mNextLevelGenOptions = NULL;
    mNewLevelDataPending = false;
}

void ClientMasterGameMode::PlaySound(const SoundEvent* sound, float volume, float pitch)
{
    Minecraft::GetInstance()->mSoundEngine->playUI(sound, volume, pitch);
}

void ClientMasterGameMode::OnRoundWaitingToStart()
{
    mLocalChunksLoaded = false;
    mPlayerProgress.clear();
    mLocalRoundTimeStart = 0;
    mSubRoundLoadStartTime = 0x7fffffff;

    Scoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetScoreboard();
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && scoreboard->mCurrentRound > 0)
    {
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
                Minecraft::GetInstance()->mLocalPlayers[playerIndex];
            if (localPlayer != NULL)
            {
                localPlayer->SetWaitingForScoreboard(true);
                localPlayer->ResetThermalVariables();
            }
        }
    }
}

void ClientMasterGameMode::OnRoundStart()
{
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
        if (player != NULL)
        {
            player->mInventory->m_selected = 0;
            mLocalPlayerInRound[playerIndex] = true;
        }
        else
        {
            mLocalPlayerInRound[playerIndex] = false;
        }
    }

    MiniGameDef::SoundSpec sound;
    const MiniGameDef* miniGame = GetMiniGame();
    if (GetMiniGame()->GetId() == eMiniGameId_Tumble)
    {
        miniGame->SetDisabledEditActions(true);
        static_cast<ConsoleSoundEngine*>(Minecraft::GetInstance()->mSoundEngine)->LimitMusicVolume();
    }

    ResetMapDistances();

    if (GetMiniGame()->HasFeatureStartCameraAnimation())
    {
        Random random;
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
            if (player != NULL)
            {
                CameraController* controller = new CameraController(3);
                if (random.nextBoolean())
                {
                    controller->addNode(0.0, 0.0, -25.0, 35.0, 180.0, 0.0, 0.0f);
                    controller->addNode(0.0, 0.0, -15.0, 35.0, 0.0, 0.0, 150.0f);
                }
                else
                {
                    controller->addNode(0.0, 0.0, -25.0, 35.0, -180.0, 0.0, 0.0f);
                    controller->addNode(0.0, 0.0, -15.0, 35.0, 0.0, 0.0, 150.0f);
                }
                controller->attachToPlayer(player, true);
            }
        }
    }
}

void ClientMasterGameMode::OnRoundPlay()
{
    mLocalRoundTimeStart = System::processTimeInMilliSecs();

    MiniGameDef::SoundSpec sound;
    const MiniGameDef* miniGame = GetMiniGame();
    Minecraft::GetInstance()->mSoundEngine->FlagMusicEventStart();

    switch (GetMiniGame()->GetId())
    {
    case eMiniGameId_Tumble:
        miniGame->SetDisabledEditActions(false);
        static_cast<ConsoleSoundEngine*>(Minecraft::GetInstance()->mSoundEngine)->RemoveMusicVolumeLimit();
        sound = miniGame->getSoundSpec(EGameModeSound_OnRoundPlay);
        PlaySound(SoundEvents::MG02_GAME_START, sound.Volume(), 1.0f);
        break;
    case eMiniGameId_Glide:
        sound = miniGame->getSoundSpec(EGameModeSound_OnRoundPlay);
        PlaySound(SoundEvents::MG03_COUNTDOWN_ZERO, sound.Volume(), 1.0f);

#ifdef MINECRAFT_PC
        PcGlideClearPractice();
#endif
        if (Minecraft::GetInstance()->GetGhostController() != NULL)
        {
            for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
                if (player != NULL && !player->isSpectator())
                    Minecraft::GetInstance()->GetGhostController()->startRecording(playerIndex, player->getPos(1.0f));
            }
            if (GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
                Minecraft::GetInstance()->GetGhostController()->startPlayback();
        }
        break;
    case eMiniGameId_Battle:
    case eMiniGameId_Build:
        sound = miniGame->getSoundSpec(EGameModeSound_OnRoundPlay);
        PlaySound(SoundEvents::MG01_GAME_START, sound.Volume(), 1.0f);
        break;
    }
}

void ClientMasterGameMode::OnRoundEnding()
{
    MiniGameDef::SoundSpec sound;
    const MiniGameDef* miniGame = GetMiniGame();
    switch (GetMiniGame()->GetId())
    {
    case 2:
        miniGame->SetDisabledEditActions(false);
        sound = miniGame->getSoundSpec(EGameModeSound_OnRoundEnding);
        PlaySound(SoundEvents::MG02_GAME_END, sound.Volume(), 1.0f);
        break;
    case 1:
    case 4:
        sound = miniGame->getSoundSpec(EGameModeSound_OnRoundEnding);
        PlaySound(SoundEvents::MG01_CELEBRATION, sound.Volume(), 1.0f);
        break;
    }

    ClientScoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetClientScoreboard();
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
        if (player != NULL)
        {
            const StatsUID suid = player->getSUID();
            if (!scoreboard->IsWinner(suid, -1))
            {
                gConsoleUIController.CloseUIScenes(playerIndex, true);
                if (Minecraft::InMiniGame(eMiniGameId_Battle, true))
                    mStrings[player->getName()] = 0;
            }
            else if (Minecraft::InMiniGame(eMiniGameId_Battle, true)
                     && CGameNetworkManager::sInstance.IsInPublicJoinableGame())
            {
                player->awardStat(GenericStats::theStudent(), GenericStats::param_theStudent());
                const bool isMaster = ++mStrings[player->getName()] >= 3;
                if (isMaster)
                    player->awardStat(GenericStats::hasBecomeTheMaster(), GenericStats::param_hasBecomeTheMaster());
            }
        }
    }

    telemetryManagerInstance->RecordGameModeRoundFinished();
}

void ClientMasterGameMode::OnRoundEndingToLobby()
{
    mRoundInfo.reset();
    mLocalRoundTimeStart = 0;
}

void ClientMasterGameMode::OnStateChange(EGameModeState state)
{
    const EGameModeState previousState = mGameModeState;
    if (previousState == state)
        return;

    mGameModeState = state;
    if (previousState == eGameModeState_None)
    {
        ResetMapDistances();
        if (!CGameNetworkManager::sInstance.IsHost())
            CMinecraftApp::SetupMiniGameMusic();
        return;
    }

    switch (state)
    {
    case eGameModeState_None:
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        minecraft->SetGhostController(NULL);
        minecraft->SetupMiniGameInstance(*MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true), 0);
        mStrings.clear();
        Reset();
        break;
    }
    case eGameModeState_RoundPlaying:
        OnRoundPlay();
        break;
    case eGameModeState_RoundEnding:
        OnRoundEnding();
        break;
    case eGameModeState_WaitingToStart:
        OnRoundWaitingToStart();
        break;
    case eGameModeState_RoundStarting:
        OnRoundStart();
        break;
    case eGameModeState_LoadingSubRound:
    {
        CleanUpMapGenerator();
        gConsoleUIController.CloseAllPlayersScenes(true);
        gConsoleUIController.HideAllGameUIElements();

        _LoadingInputParams* params = new _LoadingInputParams();
        params->m_threadProc = WaitForNextState;
        params->m_threadParam = this;
        params->m_miniGameId = Minecraft::GetInstance()->GetMiniGame()->GetId();

        FullscreenProgressState* progressState = new FullscreenProgressState();
        progressState->showConfirmation = false;
        progressState->reservedFlag1 = true;
        progressState->reservedFlag2 = true;
        progressState->playerId = gProfile->GetPrimaryPad();
        progressState->closeMode = 3;

        params->m_completeFunc = NULL;
        params->m_completeFuncParam = NULL;
        params->m_completionData = progressState;

        gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress, params,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);

        mSubRoundLoadStartTime = System::processTimeInMilliSecs();
        mLocalChunksLoaded = false;
        mServerIsWaitingOnChunkLoading = false;
        break;
    }
    case eGameModeState_Lobby:
        if (previousState == eGameModeState_RoundEnding)
            OnRoundEndingToLobby();
        break;
    case eGameModeState_SelectingGameRules:
        Minecraft::GetInstance()->mLevel->GetScoreboard()->mCurrentRound = 0;
        mVoteables->getVoteable(Voteables::eVoteable_Map)->clearVotes();
        break;
    case eGameModeState_LobbyCountdown:
    case eGameModeState_LoadingRound:
    case eGameModeState_PostRound:
        break;
    }
}

void ClientMasterGameMode::OnExitedGame()
{
    CommonMasterGameMode::OnExitedGame();
    OnStateChange(eGameModeState_None);
}

void ClientMasterGameMode::ClearPlaylist()
{
    mPlaylist.clear();
    mVoteables->getVoteable(Voteables::eVoteable_Map)->clearVotes();
}

void ClientMasterGameMode::AddMapToPlaylist(int uniqueId, const std::wstring& name, int texturePackId)
{
    MapSelectInfo info;
    info.uniqueId = uniqueId;
    info.name.assign(name);
    info.texturePackId = texturePackId;
    mPlaylist.push_back(info);
}

void ClientMasterGameMode::SetMapVote(int playerIndex, int vote, bool remove)
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    mVoteables->getVoteable(Voteables::eVoteable_Map)->setVoteAndSync(localPlayer->mPlayerIndex, vote, remove ? -1 : 1);
}

unsigned int ClientMasterGameMode::GetMapVote(int playerIndex, int vote)
{
    return static_cast<unsigned int>(mVoteables->getVoteable(Voteables::eVoteable_Map)
                                         ->getVote(Minecraft::GetInstance()->mLocalPlayers[playerIndex], vote)
                                     >= 0);
}

void ClientMasterGameMode::GetUniquePlaylist(std::vector<MapSelectInfo>& playlist)
{
    boost::unordered_set<int> themes;
    for (std::vector<MapSelectInfo>::const_iterator it = mPlaylist.begin(); it != mPlaylist.end(); ++it)
    {
        const int texturePackId = it->texturePackId;
        const std::wstring name = it->name;
        const short themeId = LevelGenerationOptions::GetThemeFromUniqueId(it->uniqueId);
        short mapSize = LevelGenerationOptions::GetMapSizeFromUniqueId(it->uniqueId);
        if (!gApp.DebugArtToolsOn())
        {
            if (themes.find(themeId) != themes.end())
                continue;
            themes.insert(themeId);
            mapSize = -1;
        }

        MapSelectInfo info;
        info.uniqueId = LevelGenerationOptions::CreateUniqueId(themeId, mapSize);
        info.name = name;
        info.texturePackId = texturePackId;
        playlist.push_back(info);
    }
}

void ClientMasterGameMode::GetCurrentMapVotesText(std::vector<HtmlString>& text)
{
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_Map);
    boost::unordered_map<int, int> totals = voteable->getTotalledVotes();
    std::vector<MapSelectInfo> playlist;
    GetUniquePlaylist(playlist);

    const LevelRules::ELevelRuleType ruleType = LevelRules::GetRuleTypeForMiniGame(Minecraft::GetMiniGameId());

    for (std::vector<MapSelectInfo>::const_iterator it = playlist.begin(); it != playlist.end(); ++it)
    {
        LevelGenerationOptions* options = gApp.getLevelRules()->getLevelGenFromUniqueId(ruleType, it->uniqueId);
        const std::wstring worldName = options != NULL ? options->getWorldName() : L"";
        if (gApp.DebugArtToolsOn())
        {
            if (totals.find(it->uniqueId) == totals.end())
                text.push_back(
                    HtmlString(formatwstr(L"%ls : 0", worldName.c_str()), eMinecraftColour_NOTSET, false, false, 0));
            else
                text.push_back(HtmlString(formatwstr(L"%ls : %d", worldName.c_str(), totals[it->uniqueId]),
                                          eMinecraftColour_NOTSET, false, false, 0));
        }

        const int themeUniqueId = LevelGenerationOptions::CreateUniqueId(
            LevelGenerationOptions::GetThemeFromUniqueId(it->uniqueId), -1);
        if (totals.find(themeUniqueId) == totals.end())
            text.push_back(
                HtmlString(formatwstr(L"%ls : 0", worldName.c_str()), eMinecraftColour_NOTSET, false, false, 0));
        else
            text.push_back(HtmlString(formatwstr(L"%ls : %d", worldName.c_str(), totals[themeUniqueId]),
                                      eMinecraftColour_NOTSET, false, false, 0));
    }
}
void ClientMasterGameMode::GetCurrentVariantVotesText(std::vector<HtmlString>& text)
{
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_Map);
    boost::unordered_map<int, int> totals = voteable->getTotalledVotes();
    MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(GetNextGameMode(), true);
    int count = static_cast<int>(definition->GetGameVariantCount()) - 2;
    if (definition->IsCustomGame())
        count = definition->GetLootSetCount();
    for (int i = 0; i < count; ++i)
    {
        const std::wstring label = definition->GetLootSetText(i);
        if (totals.find(i) != totals.end())
            text.push_back(HtmlString(formatwstr(L"%ls : %d", label.c_str(), totals[i]), eMinecraftColour_NOTSET, false,
                                      false, 0));
        else
            text.push_back(HtmlString(formatwstr(L"%ls : 0", label.c_str()), eMinecraftColour_NOTSET, false, false, 0));
    }
}

void ClientMasterGameMode::SetKickVote(int playerIndex, unsigned char colour, bool vote)
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    if (localPlayer == NULL)
        return;
    INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerBySmallId(colour);
    if (networkPlayer != NULL)
        mVoteables->getVoteable(Voteables::eVoteable_Kick)
            ->setVoteAndSync(localPlayer->mPlayerIndex, gApp.GetPlayerColour(colour), vote);
}

unsigned int ClientMasterGameMode::GetKickVote(int playerIndex, unsigned char colour)
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    return static_cast<unsigned int>(mVoteables->getVoteable(Voteables::eVoteable_Kick)
                                         ->getVote(localPlayer->mPlayerIndex, gApp.GetPlayerColour(colour))
                                     >= 0);
}

void ClientMasterGameMode::SetBuildOffScoringVote(int playerIndex, int vote, int value)
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    mVoteables->getVoteable(Voteables::eVoteable_BuildOffScoring)
        ->setVoteAndSync(localPlayer->mPlayerIndex, vote, static_cast<char>(value));
}

unsigned int ClientMasterGameMode::GetBuildOffTeamIndexToScore(int playerIndex)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    if (player != NULL)
    {
        Team* team = GetTeamAtPosition(player->mX, player->mY, player->mZ, 6.0);
        if (team != NULL && team != player->getTeam())
            return team->getTeamIndex();
    }

    return 0xffffffffU;
}

unsigned int ClientMasterGameMode::GetBuildOffScoringVote(int playerIndex, int vote)
{
    return static_cast<unsigned int>(mVoteables->getVoteable(Voteables::eVoteable_BuildOffScoring)
                                         ->getVote(Minecraft::GetInstance()->mLocalPlayers[playerIndex], vote));
}

void ClientMasterGameMode::SetBuildOffThemeVote(int playerIndex, int vote, int value)
{
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
    mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme)->setVoteAndSync(localPlayer->mPlayerIndex, vote, 1);
}

unsigned int ClientMasterGameMode::GetBuildOffThemeVote(int playerIndex)
{
    return static_cast<unsigned int>(mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme)
                                         ->getVote(Minecraft::GetInstance()->mLocalPlayers[playerIndex], 0));
}

const wchar_t* ClientMasterGameMode::GetString(const int& id)
{
    return mStringTable.getString(id);
}
void ClientMasterGameMode::GetCurrentThemeVotesText(std::vector<HtmlString>& text)
{
    Voteable* voteable = mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme);
    boost::unordered_map<int, int> totals = voteable->getTotalledVotes();
    for (unsigned int index = 0; index < mThemeWordIds.size(); ++index)
    {
        const std::wstring name = GetString(mThemeWordIds.at(index));
        if (totals.find(mThemeWordIds.at(index)) != totals.end())
            text.push_back(HtmlString(formatwstr(L"%ls : %d", name.c_str(), totals[mThemeWordIds.at(index)]),
                                      eMinecraftColour_NOTSET, false, false, 0));
        else
            text.push_back(
                HtmlString(formatwstr(L"%ls : 0", name.c_str()), eMinecraftColour_NOTSET, false, false, 0));
    }
}

std::wstring ClientMasterGameMode::GetTransitionTextOverride()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();

    switch (miniGame->GetId())
    {
    case eMiniGameId_Battle:
    case eMiniGameId_Glide:
    {
        const wchar_t* worldName = L"";
        if (mNextLevelGenOptions != NULL)
            worldName = mNextLevelGenOptions->getWorldName();
        return worldName;
    }
    case eMiniGameId_Tumble:
        if (miniGame->IsCustomGame())
            return miniGame->GetLootSetText(miniGame->GetLootSetId());
        return miniGame->GetLootSetText(miniGame->GetGameVariant());
    default:
        return L"";
    }
}

void ClientMasterGameMode::SetPlayerReady(unsigned int playerIndex, bool ready)
{
    if (playerIndex < 8)
        mPlayerReady[playerIndex] = ready;
}

bool ClientMasterGameMode::IsPlayerReady(unsigned int playerIndex)
{
    if (playerIndex < 8)
        return mPlayerReady[playerIndex];
    return false;
}

void ClientMasterGameMode::SetCurrentTheme(int themeId, int subId)
{
    mRoundInfo.setTheme(themeId, subId);
}

void ClientMasterGameMode::SetCurrentUniqueId(int uniqueId)
{
    if (uniqueId >= 0)
        mRoundInfo.setUniqueThemeId(uniqueId);
    else
        mRoundInfo.reset();
}

unsigned int ClientMasterGameMode::GetCurrentThemeId() const
{
    return mRoundInfo.getThemeId();
}
unsigned int ClientMasterGameMode::GetCurrentUniqueId() const
{
    return mRoundInfo.getThemeUniqueId();
}

void ClientMasterGameMode::SetNextLevelGenerationOptions(LevelGenerationOptions* options, bool owned)
{
    mNewLevelDataPending = owned;
    mNextLevelGenOptions = options;
    if (owned)
        mNextLevelGenOptions->loadBaseSaveData();
    mActiveChunkAreas.clear();
    mNextLevelGenOptions->getRequiredGameRules()->getActiveChunkAreas(&mActiveChunkAreas);
}

void ClientMasterGameMode::OnShowdown()
{
    MiniGameDef::SoundSpec sound;
    switch (GetMiniGame()->GetId())
    {
    case 1:
        sound = Minecraft::GetInstance()->GetMiniGame()->getSoundSpec(EGameModeSound_OnShowdown);
        PlaySound(SoundEvents::MG01_SHOWDOWN, sound.Volume(), 1.0f);
        break;
    case 2:
    case 4:
        sound = Minecraft::GetInstance()->GetMiniGame()->getSoundSpec(EGameModeSound_OnShowdown);
        PlaySound(SoundEvents::MG02_SHOWDOWN, sound.Volume(), 1.0f);
        break;
    }
}
void ClientMasterGameMode::OnChestRefill()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    const MiniGameDef::SoundSpec sound = miniGame->getSoundSpec(EGameModeSound_OnChestRefill);
    PlaySound(SoundEvents::MG01_CHEST, static_cast<float>(sound.Volume()), 1.0f);
}
void ClientMasterGameMode::OnDeath(const Vec3* position)
{
    const float deadPlayers = static_cast<float>(
        8 - Minecraft::GetInstance()->mLevel->GetClientScoreboard()->GetActivePlayerCount(eMiniGameId_Any));
    const float pitchStep = deadPlayers * 0.125f * 0.25f;
    const float volumeStep = pitchStep;
    MiniGameDef::SoundSpec sound;
    const MiniGameDef* miniGame = GetMiniGame();
    const float pitch = 1.0f;
    switch (GetMiniGame()->GetId())
    {
    case 1:
        sound = miniGame->getSoundSpec(EGameModeSound_OnDeath);
        PlaySound(SoundEvents::MG01_DEATH, sound.Volume() - volumeStep, pitch - pitchStep);
        break;
    case 2:
        miniGame->SetDisabledEditActions(false);
        sound = miniGame->getSoundSpec(EGameModeSound_OnDeath);
        PlaySound(SoundEvents::MG02_DEATH, sound.Volume(), pitch);
        break;
    case 3:
        if (position != NULL)
        {
            sound = miniGame->getSoundSpec(EGameModeSound_OnDeath);
            Minecraft::GetInstance()->mSoundEngine->play(SoundEvents::MG03_DEATH, static_cast<float>(position->x),
                                                         static_cast<float>(position->y),
                                                         static_cast<float>(position->z), sound.Volume(), pitch, NULL);
        }
        break;
    }
}

void ClientMasterGameMode::DoLobbyTickSound(int tick, const MiniGameDef::SoundSpec& sound)
{
    if (tick <= static_cast<int>(sound.ForLast()))
    {
        PlaySound(tick == 1         ? SoundEvents::MG01_LOBBY_ZERO
                  : (tick & 1) != 0 ? SoundEvents::MG01_LOBBY
                                    : SoundEvents::MG01_LOBBY_ACCENT,
                  static_cast<float>(sound.Volume()), 1.0f);
    }
}

void ClientMasterGameMode::DoRoundStartTickSound(int tick, const MiniGameDef::SoundSpec& sound)
{
    if (tick <= static_cast<int>(sound.ForLast()))
    {
        const SoundEvent* event = GetMiniGame()->GetId() == 2 ? SoundEvents::MG02_START_COUNTDOWN
                                                              : SoundEvents::MG01_START_COUNTDOWN;
        PlaySound(event, static_cast<float>(sound.Volume()), 1.0f);
    }
}

void ClientMasterGameMode::OnRoundStartTimerTick(int tick)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Scoreboard* scoreboard = minecraft->mLevel->GetScoreboard();
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && scoreboard->mCurrentRound >= 0 && tick == 3)
    {
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            if (gConsoleUIController.IsSceneInStack(playerIndex, EUIScene_GameModeScoreboardMenu))
                gConsoleUIController.CloseUIScenes(playerIndex, true);
        }
    }
}

void ClientMasterGameMode::DoGraceTickSound(int tick, const MiniGameDef::SoundSpec& sound)
{
    if (tick <= static_cast<int>(sound.ForLast()))
    {
        PlaySound(tick == 0 ? SoundEvents::MG01_GRACE_ZERO : SoundEvents::MG01_GRACE,
                  static_cast<float>(sound.Volume()), 1.0f);
    }
}

void ClientMasterGameMode::OnChatPacket(boost::shared_ptr<ClientboundChatPacket> packet)
{
    switch (packet->message)
    {
    case ClientboundChatPacket::eChatPacketMessage_TimeToGameChangeCountdown:
    {
        const MiniGameDef::SoundSpec& spec = Minecraft::GetInstance()->GetMiniGame()->getSoundSpec(
            EGameModeSound_LobbyTimer);
        DoLobbyTickSound(packet->ints[0], spec);
        break;
    }
    case ClientboundChatPacket::eChatPacketMessage_TimeToStartCountdown:
    {
        const MiniGameDef::SoundSpec& spec = Minecraft::GetInstance()->GetMiniGame()->getSoundSpec(
            EGameModeSound_RoundStartTimer);
        DoRoundStartTickSound(packet->ints[0], spec);
        OnRoundStartTimerTick(packet->ints[0]);
        break;
    }
    case ClientboundChatPacket::eChatPacketMessage_InvulnerabilityOff:
    case ClientboundChatPacket::eChatPacketMessage_InvulnerabilityCountdown:
    {
        const MiniGameDef::SoundSpec& spec = Minecraft::GetInstance()->GetMiniGame()->getSoundSpec(
            EGameModeSound_GraceTimer);
        DoGraceTickSound(packet->ints.size() == 0 ? 0 : packet->ints[0], spec);
        break;
    }
    case ClientboundChatPacket::eChatPacketMessage_SecondsToReachEnd:
    case ClientboundChatPacket::eChatPacketMessage_RoundEndIn:
    {
        const MiniGameDef::SoundSpec& spec = Minecraft::GetInstance()->GetMiniGame()->getSoundSpec(
            EGameModeSound_RoundLimitTimer);
        DoLobbyTickSound(packet->ints[0], spec);
        break;
    }
    }
}
void ClientMasterGameMode::SetGeneratorSeed(long long seed, float scale)
{
    UpdateGeneratorSeed(seed, scale);
    RestartMapGenerator();
}
bool ClientMasterGameMode::IsWaitingToStart() const
{
    return mGameModeState == eGameModeState_WaitingToStart;
}
bool ClientMasterGameMode::IsRoundStarting() const
{
    return mGameModeState == eGameModeState_RoundStarting;
}
bool ClientMasterGameMode::IsWaitingForMusicStartEvent() const
{
    return IsWaitingToStart() || IsRoundStarting() || mGameModeState == eGameModeState_SelectingGameRules
           || mGameModeState == eGameModeState_LoadingRound;
}
bool ClientMasterGameMode::IsRoundEnding() const
{
    return mGameModeState == eGameModeState_RoundEnding;
}
bool ClientMasterGameMode::IsRoundPlaying() const
{
    return mGameModeState == eGameModeState_RoundPlaying;
}
bool ClientMasterGameMode::IsInScoringState() const
{
    return Minecraft::InMiniGame(eMiniGameId_Build, true) && mGameModeState == eGameModeState_PostRound;
}
int ClientMasterGameMode::IsInLobbyState()
{
    int inLobby = 0;
    if (static_cast<unsigned int>(mGameModeState) <= eGameModeState_LoadingRound)
        inLobby = 1;
    return inLobby;
}
int ClientMasterGameMode::IsInActiveRoundState()
{
    int active = 0;
    switch (mGameModeState)
    {
    case eGameModeState_RoundStarting:
    case eGameModeState_RoundPlaying:
    case eGameModeState_PostRound:
    case eGameModeState_RoundEnding:
    case eGameModeState_LoadingSubRound:
        active = 1;
        break;
    default:
        break;
    }
    return active;
}
bool ClientMasterGameMode::IsInTrasitionFromLobby()
{
    switch (mGameModeState)
    {
    case eGameModeState_None:
    case eGameModeState_SelectingGameRules:
    case eGameModeState_LoadingRound:
        return true;
    default:
        return false;
    }
}
bool ClientMasterGameMode::CheckLocalChunksLoaded()
{
    bool allLoaded = true;
    Minecraft* minecraft = Minecraft::GetInstance();

    mLoadedChunkCount = 0;
    const unsigned int areaCount = mActiveChunkAreas.size();
    for (unsigned int i = 0; i < areaCount; ++i)
    {
        const AABB* area = mActiveChunkAreas.at(i);
        if (area == NULL)
            continue;

        const int minChunkX = static_cast<int>(area->mMinX);
        const int maxChunkX = static_cast<int>(area->mMaxX);
        const int minChunkZ = static_cast<int>(area->mMinZ);
        const int maxChunkZ = static_cast<int>(area->mMaxZ);

        for (int chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX)
        {
            for (int chunkZ = minChunkZ; chunkZ <= maxChunkZ; ++chunkZ)
            {
                if (!minecraft->getLevel(0)->reallyHasChunk(chunkX, chunkZ))
                {
                    allLoaded = false;
                    break;
                }
                ++mLoadedChunkCount;
            }
            if (!allLoaded)
                break;
        }
    }

    bool sent = false;
    if (allLoaded && mBRUPsRemaining < 1)
    {
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
            if (localPlayer != NULL && localPlayer->mPacketListener != NULL)
            {
                GameModePacket::EmptyData emptyData;
                localPlayer->mPacketListener->send(
                    GameModePacket::Create(GameModePacket::eMessage_FinishedLoadingLocally, emptyData));
#ifdef MINECRAFT_PC
                if (!sent)
                {
                    printf("[pc] mini game: the new map has loaded (%d region updates waited for), at %.1f %.1f "
                           "%.1f\n",
                           mBRUPCount, localPlayer->mX, localPlayer->mY, localPlayer->mZ);
                    fflush(stdout);
                }
#endif
                sent = true;
                mServerIsWaitingOnChunkLoading = false;
            }
        }
    }
    return sent;
}
void ClientMasterGameMode::FireGameRoundStats()
{
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
        if (mCurrentLevelGenOptions != NULL && options == NULL)
            options = mCurrentLevelGenOptions;

        MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(GetMiniGame()->GetId(), true);
        if (definition->IsCRCValid() && options != NULL && options->isCRCValid())
        {
            ClientScoreboard* scoreboard = Minecraft::GetInstance()->mLevel->GetClientScoreboard();
            GetMiniGame()->GetGameVariant();
            for (int slot = 0; slot < 4; ++slot)
            {
                boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[slot];
                if (player != NULL && !player->isSpectator())
                {
                    GhostController* ghosts = Minecraft::GetInstance()->GetGhostController();
                    const MiniGameDef* miniGame = GetMiniGame();
                    if (ghosts != NULL && scoreboard->GetEndZoneEntered(player->getSUID(), -1)
                        && ghosts->mRecordingPending[slot])
                    {
                        int recorded;
                        if (!miniGame->ArePointsActive())
                        {
                            recorded = ghosts->RecordTime(slot, options->getUniqueId(),
                                                          scoreboard->GetLifeSpan(player->getSUID(), -1));
                        }
                        else
                        {
                            recorded = ghosts->RecordScore(slot, options->getUniqueId(),
                                                           scoreboard->GetPoints(player->getSUID(), -1),
                                                           scoreboard->GetLifeSpan(player->getSUID(), -1));
                        }
                        if (recorded != 0)
                            scoreboard->SetPersonalBest(player->getSUID(), -1);
                    }
                }
            }
        }
    }
}

void ClientMasterGameMode::updateProgressRenderer()
{
    ProgressRenderer* renderer = Minecraft::GetInstance()->mProgressRenderer;

    switch (GetMiniGame()->GetId())
    {
    case eMiniGameId_Battle:
    {
        int progress = 0;
        if (!mActiveChunkAreas.empty())
        {
            const AABB* area = mActiveChunkAreas.at(0);
            const int area2D = (static_cast<int>(area->mMaxX) - static_cast<int>(area->mMinX))
                               * (static_cast<int>(area->mMaxZ) - static_cast<int>(area->mMinZ));
            if (area2D > 0)
                progress = (mLoadedChunkCount * 100) / area2D;
        }
        if (progress <= 99)
        {
            renderer->progressStage(0xebb9a246);
            renderer->progressStagePercentage(progress);
        }
        else
        {
            renderer->progressStage(IDS_PROGRESS_GENERATING_WAITING_FOR_OTHERS);
            renderer->progressStagePercentage(99);
        }
        break;
    }
    case eMiniGameId_Tumble:
    {
        const int layerCount = static_cast<int>(GetLayerCount());
        if (mSubRoundLoadStartTime != 0x7fffffff)
        {
            const long long elapsed = System::processTimeInMilliSecs() - mSubRoundLoadStartTime;
            const int progress = static_cast<int>(elapsed) * 100 / (layerCount * 1500 + 1500);
            if (progress >= 100)
                renderer->progressStage(0xebb9a246);
            else
                renderer->progressStage(IDS_PROGRESS_WAITING_ON_SERVER_GENERATING);
            float percent = static_cast<float>(progress);
            if (percent < 0.0f)
                percent = 0.0f;
            else if (percent > 100.0f)
                percent = 100.0f;
            renderer->progressStagePercentage(Mth::floor((percent / 100.0f) * 80.0f));
        }
        else
        {
            renderer->progressStage(IDS_PROGRESS_WAITING_ON_SERVER_GENERATING);
            renderer->progressStagePercentage(0);
        }
        break;
    }
    }
}

void ClientMasterGameMode::Tick()
{
    CommonMasterGameMode::Tick();

    if (mNextLevelGenOptions != NULL && mNewLevelDataPending && mNextLevelGenOptions->hasBaseSaveData())
    {
        Minecraft::GetInstance()->getLevel(0)->BlockUntilSafeToChangeSaveData(false);
        Minecraft::GetInstance()->getLevel(0)->refreshFromLevelGen();
        gRegionFileCache._clear();
        mCurrentLevelGenOptions = mNextLevelGenOptions;
        mNextLevelGenOptions = NULL;
        mNewLevelDataPending = false;
    }

    if (mServerIsWaitingOnChunkLoading && CheckLocalChunksLoaded())
        mLocalChunksLoaded = true;

    switch (mGameModeState)
    {
    case eGameModeState_RoundPlaying:
    {
        CheckProgress(Minecraft::GetInstance()->getLevel(0)->mPlayers);
        CheckCheckpoints(Minecraft::GetInstance()->getLevel(0)->mPlayers);

        GhostController* ghosts = Minecraft::GetInstance()->GetGhostController();
        if (ghosts != NULL)
        {
            const std::vector<boost::shared_ptr<Player> > ghostPlayers = ghosts->getPlayers();
            CheckCheckpoints(ghostPlayers);
            CheckProgress(ghostPlayers);

            const std::vector<boost::shared_ptr<GhostPlayer> > ghostEntities = ghosts->getGhostPlayers();
            for (std::vector<boost::shared_ptr<GhostPlayer> >::const_iterator it = ghostEntities.begin();
                 it != ghostEntities.end(); ++it)
            {
                boost::shared_ptr<GhostPlayer> ghost = *it;
                ghost->mXOld = ghost->mX;
                ghost->mYOld = ghost->mY;
                ghost->mZOld = ghost->mZ;
            }
        }
    }
    case eGameModeState_PostRound:
    case eGameModeState_RoundEnding:
        FireGameRoundStats();
        if (Minecraft::GetInstance()->GetGhostController() != NULL && mGameModeState == eGameModeState_RoundEnding)
            Minecraft::GetInstance()->GetGhostController()->stopPlayback();
        break;
    case eGameModeState_LoadingSubRound:
        updateProgressRenderer();
        break;
    default:
        break;
    }
}
bool ClientMasterGameMode::isNewLevelDataPending()
{
    return mNewLevelDataPending;
}
bool ClientMasterGameMode::isWaitingToTransition()
{
    return !mTransitions.empty() && gApp.GetGameStarted()
           && gApp.GetXuiAction(gProfile->GetPrimaryPad()) == eXuiAction_Idle;
}

GameModeTransitionParams* ClientMasterGameMode::popTransition()
{
    if (!mTransitions.empty())
    {
        GameModeTransitionParams* transition = mTransitions[0];
        mTransitions.erase(mTransitions.begin());
        return transition;
    }
    return NULL;
}

void ClientMasterGameMode::queueTransition(GameModeTransitionParams* transition)
{
    if (transition->mReturningToLobby)
    {
        for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer =
                Minecraft::GetInstance()->mLocalPlayers[playerIndex];
            if (localPlayer != NULL)
            {
                localPlayer->clientSideCloseContainer();
                localPlayer->mHasOpenedInventory = false;
            }
        }
    }

    Minecraft::GetInstance()->mGameRenderer->DisableUpdateThread(1);
    Minecraft::GetInstance()->mLevels[0]->DropChunksOutsideArea(transition->mMinChunkX, transition->mMinChunkZ,
                                                                transition->mMaxChunkX, transition->mMaxChunkZ, true);
    Minecraft::GetInstance()->mGameRenderer->EnableUpdateThread(1);
    mTransitions.push_back(transition);
}

void ClientMasterGameMode::dropAllTransitions()
{
    while (!mTransitions.empty())
        delete popTransition();
}

void ClientMasterGameMode::onChangeCustomGameMode()
{
    if (gApp.getLevelGenerationOptions() != NULL)
    {
        LevelRuleset* rules = gApp.getLevelGenerationOptions()->getRequiredGameRules();
        setCheckpoints(*rules->getCheckpoints());

        std::vector<TargetAreaRuleDefinition*> targetAreas;
        rules->getTargetAreas(&targetAreas);
        mPrimaryTargetAreas.clear();
        for (std::vector<TargetAreaRuleDefinition*>::iterator it = targetAreas.begin(); it != targetAreas.end(); it++)
        {
            if ((*it)->mPrimary)
                mPrimaryTargetAreas.push_back((*it)->getArea());
        }
    }
}

void ClientMasterGameMode::SetServerIsWaitingOnChunkLoading()
{
    mServerIsWaitingOnChunkLoading = true;
}
void ClientMasterGameMode::ClearServerIsWaitingOnChunkLoading()
{
    mServerIsWaitingOnChunkLoading = false;
}
bool ClientMasterGameMode::ServerIsWaitingOnUsLoading()
{
    return mServerIsWaitingOnChunkLoading;
}
void ClientMasterGameMode::RecievedBRUP()
{
    if (mBRUPsRemaining > 0)
        --mBRUPsRemaining;
}
void ClientMasterGameMode::SetBRUPSToWaitOn(int count)
{
    mBRUPCount = count;
    mBRUPsRemaining = count;
}
bool ClientMasterGameMode::IsValidStateForItemUse() const
{
    return !IsWaitingToStart() && !IsRoundStarting() && !IsRoundRestarting()
           && mGameModeState != eGameModeState_LoadingRound;
}
EGameModeState ClientMasterGameMode::GetGameModeState() const
{
    return mGameModeState;
}
Level* ClientMasterGameMode::GeneratorTargetLevel() const
{
    return Minecraft::GetInstance()->mLevels[0];
}
Scoreboard* ClientMasterGameMode::getScoreboard()
{
    return Minecraft::GetInstance()->mLevel->GetClientScoreboard();
}

double ClientMasterGameMode::GetPlayerMapProgress(PlayerUID uid) const
{
    boost::unordered_map<PlayerUID, double, boost::hash<PlayerUID> >::const_iterator it = mPlayerProgress.find(uid);
    if (it != mPlayerProgress.end())
        return it->second;
    return 0.0;
}

void ClientMasterGameMode::OnProgressMade(const boost::shared_ptr<Player>& player, double progress)
{
    if (!eTYPE_DERIVED_FROM(EntityType::GHOST_PLAYER, player->GetType()))
        mPlayerProgress[player->getXuid()] = progress;
}

void ClientMasterGameMode::ResetMapDistances()
{
    CommonMasterGameMode::ResetMapDistances();
    mPlayerProgress.clear();
    mLastCheckpoints.clear();
}
int ClientMasterGameMode::convertCheckpointIDToSpawnableID(int checkpoint)
{
    int spawnableIndex = 0;
    int checkpointIndex = 0;

    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL)
    {
        LevelRuleset* rules = options->getRequiredGameRules();
        std::vector<CheckpointRuleDefinition*> checkpoints = *rules->getCheckpoints();
        int lastSequenceId = -1;
        for (std::vector<CheckpointRuleDefinition*>::iterator it = checkpoints.begin(); it != checkpoints.end();
             it++, checkpointIndex++)
        {
            if ((*it)->hasSpawnPoint())
            {
                if (checkpointIndex == checkpoint)
                {
                    --spawnableIndex;
                    if (lastSequenceId == (*it)->getSequenceID())
                        --spawnableIndex;
                    break;
                }

                if (lastSequenceId != (*it)->getSequenceID())
                {
                    ++spawnableIndex;
                    lastSequenceId = (*it)->getSequenceID();
                }
            }
        }
    }

    return spawnableIndex;
}
void ClientMasterGameMode::SetLocalRoundTimeStart(long long value)
{
    mLocalRoundTimeStart = System::processTimeInMilliSecs() - value;
    if (value != 0)
        return;
#ifdef MINECRAFT_PC
    PcGlideClearPractice();
#endif

    Minecraft* minecraft = Minecraft::GetInstance();
    GhostController* ghosts = minecraft->GetGhostController();
    if (ghosts == NULL)
        return;

    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = minecraft->mLocalPlayers[playerIndex];
        if (localPlayer != NULL && !localPlayer->isSpectator())
        {
            if (ghosts->mRecordingActive[playerIndex])
                ghosts->stopRecording(playerIndex);
            ghosts->startRecording(playerIndex, Vec3::newTemp(localPlayer->mX, localPlayer->mY, localPlayer->mZ));
        }
    }

    if (CommonMasterGameMode::GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        ghosts->startPlayback();
}

void ClientMasterGameMode::SetLocalRoundTimeStartOverride(long long value)
{
    mLocalRoundTimeStart = value;
}
unsigned long long ClientMasterGameMode::GetLocalRoundTimeStart() const
{
    return static_cast<unsigned long long>(mLocalRoundTimeStart);
}
AABB* ClientMasterGameMode::GetSweptVolumeFromLastKnownPosition(boost::shared_ptr<Player> player)
{
    return player->getSweptVolume();
}
unsigned int ClientMasterGameMode::GetLastCheckpointID(const StatsUID& uid)
{
    boost::unordered_map<StatsUID, int, boost::hash<StatsUID> >::const_iterator it = mLastCheckpoints.find(uid);
    if (it != mLastCheckpoints.end())
        return it->second;
    return 0xffffffffU;
}

unsigned int ClientMasterGameMode::SetLastCheckpointID(const StatsUID& uid, int checkpoint, bool notify)
{
    if (CheckpointIDtoSequenceID(checkpoint) == CheckpointIDtoSequenceID(GetLastCheckpointID(uid)) + 1 || notify)
    {
        mLastCheckpoints[uid] = checkpoint;
        return 1;
    }
    return 0;
}
