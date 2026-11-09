#include "game/core/MasterGameMode.hpp"

#include <cstdio>

#include "game/core/StringIds.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/GameType.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/System.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"
#include "game/core/Settings.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/GameModeUtilities.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/KillboxRuleDefinition.hpp"
#include "game/minigame/GameRuleDefinition.hpp"
#include "game/minigame/NamedAreaRuleDefinition.hpp"
#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/minigame/WorldPositionRuleDefinition.hpp"
#include "game/minigame/ThermalAreaRuleDefinition.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/item/DistributeItemsRuleDefinition.hpp"
#include "game/item/Items.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/Team.hpp"
#include "game/world/generation/PlayerTeam.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/GameModePacket.hpp"
#include "game/network/ClientboundMGPlayerSettingsUpdatePacket.hpp"
#include "game/network/PlayerReadyPacket.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientboundPlayerAbilitiesPacket.hpp"
#include "game/network/MapSelectInfoPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerVoteables.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/GameCondition.hpp"
#include "game/core/ConditionConnective.hpp"
#include "game/world/generation/PlayersCondition.hpp"
#include "game/world/generation/MapGenerator.hpp"
#include "game/world/generation/WallGenerator.hpp"
#include "game/world/chunk/ChunkSource.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/MiniGamePlayerSettings.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/block/BlockRemover.hpp"
#include "game/world/block/DissolveBlockRemover.hpp"
#include "game/network/BlockChangeListener.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerStats.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <deque>
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/File.hpp"
#include "game/core/OnGameStartSpawnPositions.hpp"
#include "game/core/RegionFileCache.hpp"
#include "game/core/SpawnPositionSet.hpp"
#include "game/core/maybe.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/world/chunk/MinigamePlayerChunkMap.hpp"
#include "game/world/generation/TumbleGenerator.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/FoodData.hpp"
#include "game/data/StringTable.hpp"
#include "game/item/ItemId.hpp"
#include "game/item/ElytraItem.hpp"
#include "game/minigame/DegradationSequenceRuleDefinition.hpp"
#include "game/world/block/BlockDegradeRoutine.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/minigame/PowerupRuleDefinition.hpp"

namespace
{
const int RANDOM_LOOT_SET = 5;
const int MIN_SCALED_PLAYERS = 4;
const int MAX_SCALED_PLAYERS = 16;
const double SPREAD_PLAYERS_DISTANCE = 3.0;
const float TUMBLE_PLAYER_HEALTH = 1.0f;
const float GLIDE_PLAYER_HEALTH = 3.0f;
const float FULL_PLAYER_HEALTH = -1.0f;
const double NEAR_BLOCKS_DISTANCE = 2.0;
const unsigned int BEACON_CHECKPOINTS_AHEAD = 3;
const int FULL_ROUND_END_TIMER_PLAYERS = 4;
const unsigned int TUMBLE_ROUNDS_TO_WIN = 3;
const unsigned char EMPTY_SPAWN_SLOT = 0xff;
const float KILLBOX_DAMAGE = 4.0f;
const long long CHUNK_LOAD_TIMEOUT_SECONDS = 20;
const double TEAM_AREA_SPREAD_DISTANCE = 1.0;
const int NEXT_ROUND_DELAY_SECONDS = 5;
const int WALL_DISSOLVE_PASSES = 5;
const int WALL_DISSOLVE_TICKS_PER_PASS = 15;
const unsigned int PUMPKIN_SPECTATOR_TEXTURE_PACK_ID = 1029;
const unsigned int CENTRAL_SPAWN_MAX_PLAYERS = 8;
const int ACHIEVEMENT_EXPLOIT_PLAYER_COUNT = 2;
const int MAX_KICK_VOTES_REQUIRED = 3;
const int SPAWN_LAYER_REMAINING_BLOCKS = 11;
const double FINISH_LINE_SOUND_RANGE = 30.0;
const int END_ZONE_SHOWDOWN_REDUCTION = 60000;
const int END_ZONE_SHOWDOWN_MINIMUM = 30000;
const float UNMEASURED_CHEST_DISTANCE = -1.0f;
const int CHEST_REFILL_ATTEMPTS_PER_CHEST = 3;
const double TEAM_AREA_BOUNDS_HEIGHT_MARGIN = 1.0;

inline PlayerList* ServerPlayers()
{
    return gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers;
}

inline void BroadcastGameModePacket(GameModePacket::EMessage message, const GameModePacket::Data& data)
{
    ServerPlayers()->broadcastAll(GameModePacket::Create(message, data));
}

inline void BroadcastChatMessage(ClientboundChatPacket::EChatPacketMessage message)
{
    ServerPlayers()->broadcastAll(boost::shared_ptr<Packet>(new ClientboundChatPacket(message, -1)));
}
}

std::map<MasterGameMode::EInternalGameModeState, EGameModeState> MasterGameMode::sStateMap;
bool MasterGameMode::sLobbyTimerNeedsReset = false;

std::vector<boost::shared_ptr<ServerPlayer> > MasterGameMode::GetPlayers(bool excludeSpectators, bool readyOnly)
{
    std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> > result;
    for (unsigned int i = 0; i < players->size(); ++i)
    {
        if (excludeSpectators && players->at(i)->isSpectator())
            continue;
        if (readyOnly && !mPlayerReady[players->at(i)->mPlayerIndex])
            continue;
        result.push_back(players->at(i));
    }
    return result;
}

int MasterGameMode::GetPlayerCount(bool excludeSpectators, bool readyOnly)
{
    return GetPlayers(excludeSpectators, readyOnly).size();
}

long long MasterGameMode::GetCurrentTimeMillis()
{
    return System::processTimeInMilliSecs();
}

bool MasterGameMode::HasTimer(ETimerID timer)
{
    return mTimers.find(timer) != mTimers.end();
}

void MasterGameMode::KillTimer(ETimerID timer)
{
    boost::unordered_map<ETimerID, _TimerInfo>::iterator it = mTimers.find(timer);
    if (it != mTimers.end())
        it->second.active = false;
}

void MasterGameMode::ResetRoundTime()
{
    mRoundStartTime = GetCurrentTimeMillis();
    mShowdownStartTime = 0;
    mPostRoundStartTime = 0;
    if (HasTimer(eTimerID_TimeToReachEnd))
        KillTimer(eTimerID_TimeToReachEnd);
}

void MasterGameMode::GeneratePlaylistSyncInfo()
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    const EMiniGameId miniGameType = gApp.GetFirstMiniGameType();
    if (miniGameType == eMiniGameId_Battle || miniGameType == eMiniGameId_Glide)
    {
        StringTable nameTable;
        int rulesetCount = 0;
        for (std::vector<LevelRules::LevelRulesetGroup*>::const_iterator group = mPlaylist.begin();
             group != mPlaylist.end(); ++group)
            rulesetCount += (*group)->rulesets.size();
        stream.writeInt(rulesetCount);

        std::wstring worldNameKey(L"");
        for (std::vector<LevelRules::LevelRulesetGroup*>::const_iterator group = mPlaylist.begin();
             group != mPlaylist.end(); ++group)
        {
            for (std::vector<LevelRuleset*>::const_iterator ruleset = (*group)->rulesets.begin();
                 ruleset != (*group)->rulesets.end(); ++ruleset)
            {
                LevelGenerationOptions* options = (*ruleset)->getGenerationOptions();
                stream.writeInt(options->getUniqueId());
                worldNameKey = options->getWorldNameKey();
                stream.writeUTF(worldNameKey);
                stream.writeInt((*ruleset)->getGenerationOptions()->getTexturePackId());
                (*ruleset)->getGenerationOptions()->copyStringToTable(worldNameKey, &nameTable);
            }
        }
        nameTable.write(&stream);
    }
    else if (miniGameType == eMiniGameId_Build)
    {
        Random random;
        std::vector<int> themeWordIds = mLocalizedStringTable->getRandomStringIds(5, &random);
        stream.writeInt(themeWordIds.size());
        for (unsigned int i = 0; i < themeWordIds.size(); ++i)
            stream.writeInt(themeWordIds[i]);
        mThemeWordIds = std::vector<int>(themeWordIds);
    }

    if (byteStream.mCount == 0)
    {
        mPlaylistSyncData.data = NULL;
        mPlaylistSyncData.length = 0;
    }
    else
    {
        mPlaylistSyncData = byteStream.toByteArray();
    }
}

void MasterGameMode::ResetReadyStates(bool ready)
{
    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex)
    {
        mPlayerReady[playerIndex] = ready;
        if (gMinecraftServer != NULL && gMinecraftServer->mPlayers != NULL)
            ServerPlayers()->broadcastAll(boost::shared_ptr<Packet>(new PlayerReadyPacket(playerIndex, ready)));
    }
}

MasterGameMode::MasterGameMode(std::vector<LevelRules::LevelRulesetGroup*>* playlist) : mTimers(11)
{
    mPlaylistSyncData.data = NULL;
    mPlaylistSyncData.length = 0;
    mPlayerSpawnSlots.data = NULL;
    mPlayerSpawnSlots.length = 0;
    mSpectatorSpawnSlots.data = NULL;
    mSpectatorSpawnSlots.length = 0;
    mBlockDegradeRoutine = NULL;
    mLobbyChestsPopulated = false;
    mRandomLootSetChosen = false;
    mGameRulesChanged = true;
    mState = eInternalGameModeState_Initialising;

    mVoteables = new ServerVoteables();
    mVoteables->addVoteable(new Voteable(Voteables::eVoteable_Map, true, true));
    mVoteables->addVoteable(new Voteable(Voteables::eVoteable_Kick, true, false));
    if (gApp.GetFirstMiniGameType() == eMiniGameId_Build)
    {
        mVoteables->addVoteable(new Voteable(Voteables::eVoteable_BuildOffScoring, true, false));
        mVoteables->addVoteable(new Voteable(Voteables::eVoteable_BuildOffTheme, true, true));
    }

    Setup(gApp.GetFirstMiniGameType());
    mLobbyLevelGenOptions = gApp.getLevelGenerationOptions();
    AABB* activeArea = mLobbyLevelGenOptions->getRequiredGameRules()->getNamedArea(L"ActiveArea");
    if (activeArea != NULL)
    {
        mLobbyMinX = activeArea->mMinX;
        mLobbyMaxX = activeArea->mMaxX;
        mLobbyMinZ = activeArea->mMinZ;
        mLobbyMaxZ = activeArea->mMaxZ;
        mActiveChunkAreas.push_back(activeArea);
    }

    Minecraft::GetInstance()->SetupMiniGameInstance(*MiniGameDef::GetCustomGameModeById(eMiniGameId_Lobby, true), 0);
    mRoundStartTime = 0;
    mShowdownStartTime = 0;
    mPostRoundStartTime = 0;
    mFinalCheckpointSequenceID = 0;
    mBeaconCheckpointIndex = 0;
    mPlaylist.insert(mPlaylist.end(), playlist->begin(), playlist->end());
    std::sort(mPlaylist.begin(), mPlaylist.end(), LevelRules::LevelRulesetGroup::SortMapRules);
    GeneratePlaylistSyncInfo();
    ResetReadyStates(false);

    mTutorialDirty = false;
    mLastTutorialPlayerCount = 0;
    mTutorialData.tutorial = eTutorial_GameMode_Scoreboard;
    mBlockChangeListener = NULL;
    mTickCount = 0;
    mLastChatMessage = 0;
    mLastChatTime = 0;
    mBlockRemover = NULL;

    mPlayerSpawnSlots.data = new unsigned char[32];
    mPlayerSpawnSlots.length = 32;
    mSpectatorSpawnSlots.data = new unsigned char[32];
    mSpectatorSpawnSlots.length = 32;
    std::memset(mPlayerSpawnSlots.data, 0xff, mPlayerSpawnSlots.length);
    std::memset(mSpectatorSpawnSlots.data, 0xff, mSpectatorSpawnSlots.length);

    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex)
        mLastKnownPositions[playerIndex] = NULL;
    mPlayersLoading = 0;
    mChunkLoadStartTime = 0;
    mLayerBlocksDestroyed = 0;
    mSpawnLayerIndex = 0;
    mChunkLoadStartTick = 0;
    mWaitingForChunkLoading = false;
}

void MasterGameMode::ResetLastKnownPosition(int playerIndex)
{
    delete mLastKnownPositions[playerIndex];
    mLastKnownPositions[playerIndex] = NULL;
}

MasterGameMode::~MasterGameMode()
{
    delete mVoteables;
    delete mBlockChangeListener;
    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex)
        ResetLastKnownPosition(playerIndex);
}

int MasterGameMode::GetMinimumPlayersToStart()
{
    if (GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial))
        return 1;

    int minimumPlayers = 0;
    EMiniGameId miniGameId = GetMiniGame()->GetId();
    if (miniGameId == eMiniGameId_Lobby)
        miniGameId = GetNextGameMode();
    if (miniGameId != eMiniGameId_Lobby && miniGameId != eMiniGameId_Any)
    {
        ConditionConnective* startConditions = dynamic_cast<ConditionConnective*>(
            MiniGameDef::GetCustomGameModeById(miniGameId, true)->GetStartConditions());
        if (startConditions != NULL && startConditions->GetConditions().size() == 1)
        {
            PlayersCondition* playersCondition = dynamic_cast<PlayersCondition*>(
                startConditions->GetConditions().at(0));
            if (playersCondition != NULL)
                minimumPlayers = playersCondition->GetThreshold(this);
        }
    }
    return minimumPlayers;
}

MasterGameMode::EInternalGameModeState MasterGameMode::GetState()
{
    return mState;
}

void MasterGameMode::RestartTimer(ETimerID timer, int duration)
{
    if (mTimers.find(timer) == mTimers.end())
        return;

    const long long now = GetCurrentTimeMillis();
    _TimerInfo info = mTimers[timer];
    info.active = true;
    info.endTime = now + duration;
    info.nextAnnouncement = now + info.announceEvery;
    info.duration = duration;
    mTimers[timer] = info;
}

void MasterGameMode::TickTimers()
{
    if (sLobbyTimerNeedsReset)
    {
        RestartTimer(eTimerID_LobbyInterval, GetMiniGame()->getTimerSpec(eTimerID_LobbyInterval).TotalDurationMs());
        sLobbyTimerNeedsReset = false;
    }

    const long long now = GetCurrentTimeMillis();
    boost::unordered_map<ETimerID, _TimerInfo>::iterator it = mTimers.begin();
    while (it != mTimers.end())
    {
        _TimerInfo& info = it->second;
        if (!info.active)
        {
            delete static_cast<CountdownInfo*>(info.callbackData);
            it = mTimers.erase(it);
            continue;
        }

        if (info.nextAnnouncement < now)
        {
            if (info.changeAnnounceAfter < 0 || now - info.startTime <= info.changeAnnounceAfter)
                info.nextAnnouncement += info.announceEvery;
            else
                info.nextAnnouncement += info.finalAnnounceEvery;

            if (info.endTime < now && info.endTime > 0)
                info.active = false;

            if (info.callback != NULL)
                info.callback(it->first, this, info.callbackData);
        }
        ++it;
    }
}

void MasterGameMode::SyncNextLevelGen()
{
    if (mNextLevelGenOptions == NULL)
        return;

    const MiniGameDef* nextMiniGame = MiniGameDef::GetCustomGameModeById(
        static_cast<EMiniGameId>(mNextLevelGenOptions->getRequiredGameRules()->getRuleType()), true);
    GameModePacket::ChangeCustomGameMode data(nextMiniGame->GetId(), mNextLevelGenOptions->getUniqueId());
    BroadcastGameModePacket(GameModePacket::eMessage_SetNextCustomGameMode, data);
}

bool MasterGameMode::OnGameModeChange(boost::shared_ptr<ServerPlayer> player)
{
    if (player->mConnection == NULL)
        return false;
    player->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_Tutorial, mTutorialData));
    return true;
}

void MasterGameMode::CheckTutorialUpdate()
{
    std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
    if (!mTutorialDirty && static_cast<int>(players->size()) == mLastTutorialPlayerCount)
        return;

    mLastTutorialPlayerCount = players->size();
    if (mLastTutorialPlayerCount > 0)
        SyncNextLevelGen();

    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator player = players->begin();
         player != players->end(); ++player)
    {
        if (OnGameModeChange(*player))
            mTutorialDirty = false;
    }
}

bool MasterGameMode::IsInKillBox(boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<ServerPlayer> player;
    if (entity->GetType() == EntityType::SERVER_PLAYER)
    {
        player = boost::static_pointer_cast<ServerPlayer>(entity);
        if (player != NULL && player->isSpectator())
            return false;
    }

    Vec3* position = Vec3::newTemp(entity->mX, entity->mY, entity->mZ);
    for (std::vector<KillboxRuleDefinition*>::const_iterator killbox = mKillboxes.begin(); killbox != mKillboxes.end();
         ++killbox)
    {
        if ((*killbox)->getArea()->contains(position))
        {
            if (player != NULL)
                (*killbox)->GetDeathMessageIDS(player->mDeathMessageId, player->mDeathMessage);
            return true;
        }
    }
    return false;
}

bool MasterGameMode::AreLevelChunksLoaded()
{
    bool loaded = true;
    for (std::vector<AABB*>::const_iterator area = mActiveChunkAreas.begin(); area != mActiveChunkAreas.end() && loaded;
         ++area)
    {
        const int minX = (*area)->mMinX;
        const int maxX = (*area)->mMaxX;
        const int minZ = (*area)->mMinZ;
        const int maxZ = (*area)->mMaxZ;
        ChunkSource* chunkSource = gMinecraftServer->getLevel(0)->getChunkSource();
        for (int x = minX; x <= maxX && loaded; ++x)
        {
            for (int z = minZ; z <= maxZ; ++z)
            {
                if (!chunkSource->hasChunk(x, z))
                {
                    chunkSource->create(x, z);
                    loaded = false;
                    break;
                }
            }
        }
    }

    const bool generated = mMapGenerator == NULL || mMapGenerator->FinishedGeneration();
    return loaded && generated;
}

void MasterGameMode::ChooseNextGameRules(bool preview)
{
    LevelGenerationOptions* chosenOptions = NULL;
    LevelRules::LevelRulesetGroup* chosenGroup = NULL;

    int voteCount;
    const int vote = mVoteables->getVoteable(Voteables::eVoteable_Map)->getWinningVote(voteCount);
    if (vote != -1)
    {
        for (std::vector<LevelRules::LevelRulesetGroup*>::const_iterator group = mPlaylist.begin();
             group != mPlaylist.end() && chosenGroup == NULL; ++group)
        {
            std::vector<LevelRuleset*>& rulesets = (*group)->rulesets;
            if (static_cast<short>(LevelGenerationOptions::GetMapSizeFromUniqueId(vote)) == -1)
            {
                if (rulesets.front()->getThemeId() == LevelGenerationOptions::GetThemeFromUniqueId(vote))
                    chosenGroup = *group;
                continue;
            }

            for (std::vector<LevelRuleset*>::const_iterator ruleset = rulesets.begin(); ruleset != rulesets.end();
                 ++ruleset)
            {
                if (static_cast<int>((*ruleset)->getGenerationOptions()->getUniqueId()) == vote)
                {
                    chosenOptions = (*ruleset)->getGenerationOptions();
                    chosenGroup = *group;
                    break;
                }
            }
        }

        std::deque<LevelRules::LevelRulesetGroup*>::iterator remaining = std::find(
            mRemainingRuleGroups.begin(), mRemainingRuleGroups.end(), chosenGroup);
        if (remaining != mRemainingRuleGroups.end())
        {
            mRemainingRuleGroups.erase(remaining);
            mPlayedRuleGroups.push_back(chosenGroup);
        }
    }

    if (chosenGroup == NULL)
    {
        chosenGroup = mRemainingRuleGroups.front();
        if (!preview)
        {
            mRemainingRuleGroups.pop_front();
            mPlayedRuleGroups.push_back(chosenGroup);
        }
    }

    if (mRemainingRuleGroups.empty())
    {
        mRemainingRuleGroups.insert(mRemainingRuleGroups.begin(), mPlayedRuleGroups.begin(), mPlayedRuleGroups.end());
        mPlayedRuleGroups.clear();
    }

    if (chosenOptions == NULL)
    {
        unsigned int mapSize;
        if (GetMiniGame()->GetMapSizeOverride() == 0)
        {
            mapSize = static_cast<unsigned int>(GetPlayerCount(false, false)) > MAX_PLAYERS
                          ? LevelGenerationOptions::eMapSize_Large
                          : LevelGenerationOptions::eMapSize_Medium;
            if (static_cast<unsigned int>(GetPlayerCount(false, false)) <= GetMiniGame()->GetMaxPlayersForSmallMaps())
                mapSize = LevelGenerationOptions::eMapSize_Small;
        }
        else
        {
            mapSize = GetMiniGame()->GetMapSizeOverride() - 1;
        }

        for (;;)
        {
            std::vector<LevelRuleset*>& rulesets = chosenGroup->rulesets;
            for (std::vector<LevelRuleset*>::const_iterator ruleset = rulesets.begin(); ruleset != rulesets.end();
                 ++ruleset)
            {
                if ((*ruleset)->getGenerationOptions() != NULL
                    && (*ruleset)->getGenerationOptions()->getMapSize() == mapSize)
                {
                    chosenOptions = (*ruleset)->getGenerationOptions();
                    break;
                }
            }
            if (chosenOptions != NULL)
                break;

            if (mapSize == LevelGenerationOptions::eMapSize_Large)
            {
                mapSize = LevelGenerationOptions::eMapSize_Medium;
            }
            else if (mapSize == LevelGenerationOptions::eMapSize_Medium)
            {
                mapSize = LevelGenerationOptions::eMapSize_Small;
            }
            else
            {
                chosenOptions = rulesets.front()->getGenerationOptions();
                if (chosenOptions == NULL)
                    return;
                break;
            }
        }
    }

    mNextLevelGenOptions = chosenOptions;
    SyncNextLevelGen();
}

void MasterGameMode::UpdateClientState(boost::shared_ptr<ServerPlayer> player)
{
    GameModePacket::StateChangeData data(sStateMap[mState]);
    if (player == NULL)
        BroadcastGameModePacket(GameModePacket::eMessage_StateChange, data);
    else
        player->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_StateChange, data));
}

void MasterGameMode::SetState(EInternalGameModeState state)
{
    const EInternalGameModeState previousState = mState;
    mState = state;
    if (sStateMap[previousState] != sStateMap[mState])
        UpdateClientState(boost::shared_ptr<ServerPlayer>());
}

void MasterGameMode::sendTutorialUpdate(eTutorial_GameMode tutorial)
{
    if (mTutorialData.tutorial == tutorial)
        return;
    mTutorialData.tutorial = tutorial;
    mTutorialDirty = true;
}

void MasterGameMode::BroadcastChat(int message, std::vector<int>& arguments)
{
    if (message == 0)
        return;

    const unsigned long long now = System::processTimeInMilliSecs();
    if (message == mLastChatMessage && static_cast<long long>(now - mLastChatTime) <= 500)
        return;

    const unsigned int argumentCount = arguments.size();
    int* intParams = new int[argumentCount + 1];
    for (unsigned int i = 0; i < arguments.size(); ++i)
        intParams[i] = arguments[i];

    ServerPlayers()->broadcastAll(boost::shared_ptr<Packet>(new ClientboundChatPacket(
        static_cast<ClientboundChatPacket::EChatPacketMessage>(message), intParams, argumentCount + 1, NULL, 0)));
    delete[] intParams;
    mLastChatTime = now;
    mLastChatMessage = message;
}

void MasterGameMode::CompleteTimer(ETimerID timer)
{
    if (mTimers.find(timer) == mTimers.end())
        return;

    const long long now = GetCurrentTimeMillis();
    _TimerInfo info = mTimers[timer];
    info.active = true;
    info.endTime = now;
    info.nextAnnouncement = now;
    mTimers[timer] = info;
}

void MasterGameMode::CheckReadyStates()
{
    unsigned int readyPlayers = 0;
    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex)
    {
        if (mPlayerReady[playerIndex])
            ++readyPlayers;
    }

    const unsigned int minimumPlayers = GetMinimumPlayersToStart();
    unsigned int playerCount = GetPlayerCount(false, false);
    if (playerCount < minimumPlayers)
    {
        std::vector<int> arguments;
        arguments.push_back(minimumPlayers - playerCount);
        BroadcastChat(ClientboundChatPacket::eChatPacketMessage_MorePlayersRequired, arguments);
    }

    unsigned int requiredReady = playerCount;
    if (!GetMiniGame()->CanAutoStart())
    {
        requiredReady = minimumPlayers;
        const unsigned int halfPlayers = std::ceil(playerCount * 0.5f);
        if (minimumPlayers < halfPlayers)
            requiredReady = halfPlayers;
    }

    if (readyPlayers < requiredReady)
    {
        if (!GetMiniGame()->CanAutoStart())
        {
            std::vector<int> arguments;
            arguments.push_back(requiredReady - readyPlayers);
            BroadcastChat(ClientboundChatPacket::eChatPacketMessage_WaitingForPlayersReady, arguments);
            if (GetState() == eInternalGameModeState_LobbyCountdown)
            {
                KillTimer(eTimerID_LobbyInterval);
                SetState(eInternalGameModeState_Lobby);
                mLobbyChestsPopulated = true;
            }
        }
    }
    else if (!GetMiniGame()->CanAutoStart())
    {
        if (GetState() == eInternalGameModeState_Lobby)
            SetState(eInternalGameModeState_LobbyCountdown);
    }
    else if (GetState() == eInternalGameModeState_LobbyCountdown
             && static_cast<unsigned int>(GetPlayerCount(false, false))
                    >= static_cast<unsigned int>(GetMinimumPlayersToStart()))
    {
        CompleteTimer(eTimerID_LobbyInterval);
    }
}

void MasterGameMode::AddTimer(ETimerID timer, TimerCallback callback, void* context, int duration, int announceEvery,
                              int changeAnnounceAfter, int finalAnnounceEvery)
{
    const long long now = GetCurrentTimeMillis();
    _TimerInfo info;
    info.endTime = duration > 0 ? now + duration : -1;
    info.startTime = now;
    info.duration = duration;
    info.announceEvery = announceEvery;
    info.changeAnnounceAfter = changeAnnounceAfter;
    info.finalAnnounceEvery = finalAnnounceEvery;
    if (announceEvery >= 0)
        info.nextAnnouncement = now + announceEvery;
    else if (changeAnnounceAfter >= 0)
        info.nextAnnouncement = now + changeAnnounceAfter;
    else
        info.nextAnnouncement = info.endTime;
    info.active = true;
    info.callbackData = context;
    info.callback = callback;
    mTimers[timer] = info;
}

void MasterGameMode::AddTimer(ETimerID timer, TimerCallback callback, void* context,
                              MiniGameDef::TimerSpec specification)
{
    AddTimer(timer, callback, context, specification.TotalDurationMs(), specification.AnnounceEveryMs(false),
             specification.ChangeAnnounceAfterMs(), specification.AnnounceEveryMs(true));
}

int MasterGameMode::GetTimerRemainingTime(ETimerID timer)
{
    boost::unordered_map<ETimerID, _TimerInfo>::iterator it = mTimers.find(timer);
    if (it == mTimers.end() || !it->second.active)
        return 0;
    if (it->second.endTime > 0)
        return it->second.endTime - GetCurrentTimeMillis();
    return 0x7fffffff;
}

void MasterGameMode::TimerCountdown(ETimerID timer, CountdownInfo* countdown)
{
    const int remaining = GetTimerRemainingTime(timer);
    if (remaining > 0)
    {
        const int seconds = std::ceil(remaining / 1000.0f);
        std::vector<int> arguments(countdown->arguments);
        arguments.insert(arguments.begin(), seconds);
        BroadcastChat(countdown->message, arguments);
        if (countdown->tickCallback != NULL)
            countdown->tickCallback(this, countdown->callbackData);
        return;
    }

    if (countdown->completeCallback != NULL)
        countdown->completeCallback(this, countdown->callbackData);
}

void MasterGameMode::PopulateChests()
{
    LevelRuleset* ruleDefinitions = gApp.getGameRuleDefinitions();
    GameRuleDefinition* worldRules = ruleDefinitions == NULL ? NULL
                                                             : ruleDefinitions->getRuleByType(
                                                                   ConsoleGameRules::eGameRuleType_OnInitialiseWorld);
    if (worldRules == NULL)
        return;

    ServerLevel* level = gMinecraftServer->getLevel(0);
    std::vector<GameRuleDefinition*> rules;
    worldRules->getChildren(&rules, ConsoleGameRules::eGameRuleType_PlaceContainer);
    for (std::vector<GameRuleDefinition*>::const_iterator rule = rules.begin(); rule != rules.end(); ++rule)
        (*rule)->evaluateRule(level, ruleDefinitions, *GetMiniGame());

    rules.clear();
    worldRules->getChildren(&rules, ConsoleGameRules::eGameRuleType_DistributeItems);
    for (std::vector<GameRuleDefinition*>::const_iterator rule = rules.begin(); rule != rules.end(); ++rule)
        (*rule)->evaluateRule(level, ruleDefinitions, *GetMiniGame());

    rules.clear();
    worldRules->getChildren(&rules, ConsoleGameRules::eGameRuleType_PopulateContainer);
    for (std::vector<GameRuleDefinition*>::const_iterator rule = rules.begin(); rule != rules.end(); ++rule)
        (*rule)->evaluateRule(level, ruleDefinitions, *GetMiniGame());
}

int MasterGameMode::ChooseItemSet(const MiniGameDef& definition, bool central)
{
    const EMiniGameId miniGameId = definition.GetId();
    if (miniGameId == eMiniGameId_Tumble)
    {
        if (central && definition.HostOption_Bool(eCustomGameModeHostOption_EnableVoting))
        {
            const unsigned int variant = definition.GetGameVariant();
            int voteCount;
            if (mVoteables->getVoteable(Voteables::eVoteable_Map)->getWinningVote(voteCount) == -1)
            {
                const unsigned int selectableVariants = definition.GetGameVariantCount() - 2;
                definition.SetHostOption(eCustomGameModeHostOption_GameVariant, (variant + 1) % selectableVariants);
            }
            else
            {
                definition.SetHostOption(eCustomGameModeHostOption_GameVariant,
                                         mVoteables->getVoteable(Voteables::eVoteable_Map)->getWinningVote(voteCount));
            }
        }

        if (definition.HasGameVariants() && !CGameNetworkManager::sInstance.IsInPublicJoinableGame()
            && CGameNetworkManager::sInstance.IsHost())
        {
            int voteCount;
            const int vote = mVoteables->getVoteable(Voteables::eVoteable_Map)->getWinningVote(voteCount);
            if (vote != -1)
            {
                definition.SetHostOption(eCustomGameModeHostOption_GameVariant, vote);
                return vote;
            }
        }

        if (!definition.IsCustomGame())
            return definition.GetGameVariant();
    }
    else if (miniGameId == eMiniGameId_Battle)
    {
        const int lootSetId = definition.GetLootSetId();
        if (mRandomLootSetChosen || lootSetId == RANDOM_LOOT_SET)
        {
            mRandomLootSetChosen = true;
            return Random().nextInt(RANDOM_LOOT_SET);
        }
        return lootSetId;
    }
    return definition.GetLootSetId();
}

unsigned int MasterGameMode::GetTeamCount()
{
    if (Minecraft::GetInstance()->GetMiniGame()->GetId() == eMiniGameId_Build && mNextLevelGenOptions != NULL)
    {
        std::vector<AABB*> buildAreas;
        mNextLevelGenOptions->getRequiredGameRules()->getNamedAreas(0xff, &buildAreas);
        return buildAreas.size();
    }
    return Minecraft::GetInstance()->GetMiniGame()->GetTeamCount();
}

void MasterGameMode::SetupTeams()
{
    Scoreboard* scoreboard = gMinecraftServer->getLevel(0)->GetScoreboard();
    const EMiniGameId miniGameId = Minecraft::GetInstance()->GetMiniGame()->GetId();
    const Team::Visibility deathMessageVisibility = miniGameId == eMiniGameId_Tumble || miniGameId == eMiniGameId_Glide
                                                        ? Team::NEVER
                                                        : Team::ALWAYS;

    const unsigned int teamCount = GetTeamCount();
    for (unsigned int teamIndex = 0; teamIndex < teamCount; ++teamIndex)
    {
        wchar_t teamName[0x20];
        swprintf(teamName, 0x20, L"%d", teamIndex);
        PlayerTeam* team = scoreboard->getPlayerTeam(std::wstring(teamName));
        if (team == NULL)
            team = scoreboard->addPlayerTeam(std::wstring(teamName));
        if (team == NULL)
            continue;

        team->setAllowFriendlyFire(true);
        team->setNameTagVisibility(Team::HIDE_FOR_OTHER_TEAMS);
        team->setDeathMessageVisibility(deathMessageVisibility);
        team->setSeeFriendlyInvisibles(false);
        team->syncChanges();
    }

    if (scoreboard->getPlayerTeam(SPECTATOR_TEAM_NAME) == NULL)
    {
        PlayerTeam* spectatorTeam = scoreboard->addPlayerTeam(SPECTATOR_TEAM_NAME);
        spectatorTeam->setNameTagVisibility(Team::HIDE_FOR_OTHER_TEAMS);
        spectatorTeam->setDeathMessageVisibility(deathMessageVisibility);
        spectatorTeam->setSeeFriendlyInvisibles(false);
        spectatorTeam->syncChanges();
    }
}

void MasterGameMode::SelectNewGameRules()
{
    ChooseNextGameRules(false);
    if (mNextLevelGenOptions == NULL)
        return;

    mNextLevelGenOptions->loadBaseSaveData();
    const MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(
        static_cast<EMiniGameId>(mNextLevelGenOptions->getRequiredGameRules()->getRuleType()), true);
    miniGame->SetLootSet(ChooseItemSet(*miniGame, true));
    SetupMiniGameInstance(*miniGame, 0);
    SetupTeams();

    if (miniGame->GetId() == eMiniGameId_Build)
    {
        int voteCount;
        int themeWordId = mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme)->getWinningVote(voteCount);
        if (themeWordId == -1)
        {
            Random random;
            themeWordId = mThemeWordIds.at(random.nextInt(mThemeWordIds.size() - 1));
        }
        setChosenThemeWordId(themeWordId);
        clearBuildOffVotes();
        GeneratePlaylistSyncInfo();
        ServerPlayers()->broadcastAll(boost::shared_ptr<Packet>(new MapSelectInfoPacket(mPlaylistSyncData, false)));
    }
}

void MasterGameMode::GetKillBoxes(LevelGenerationOptions* options, std::vector<KillboxRuleDefinition*>& killboxes)
{
    options->getRequiredGameRules()->getKillboxes(&killboxes);

    std::vector<AABB*> killboxAreas;
    options->getRequiredGameRules()->getNamedAreas(0x10000000, &killboxAreas);
    for (std::vector<AABB*>::const_iterator area = killboxAreas.begin(); area != killboxAreas.end(); ++area)
        killboxes.push_back(new KillboxRuleDefinition(*area));
}

void MasterGameMode::ResetPlayers()
{
    std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end();
         ++it)
    {
        const boost::shared_ptr<ServerPlayer>& player = *it;
        player->mGameMode->SetDeaths(0);
        player->mInventory->setCarried(ItemInstance::EMPTY);
        player->getFoodData()->setExhaustion(0.0f);

        player->getFoodData()->setFoodLevel(
            GetMiniGame()->GetFoodBalance()->mMaxFood.otherwise(FoodConstants::MaxFood()));
        player->getFoodData()->setSaturation(
            GetMiniGame()->GetFoodBalance()->mMaxSaturation.otherwise(FoodConstants::MaxSaturation()));
        player->removeAllEffects();
    }

    std::memset(mPlayerSpawnSlots.data, 0xff, mPlayerSpawnSlots.length);
    std::memset(mSpectatorSpawnSlots.data, 0xff, mSpectatorSpawnSlots.length);
}

bool MasterGameMode::IsChunkWithinActiveBounds(void* context, int chunkX, int chunkZ)
{
    MasterGameMode* gameMode = static_cast<MasterGameMode*>(context);
    if (chunkX >= gameMode->mLobbyMinX && chunkX <= gameMode->mLobbyMaxX && chunkZ >= gameMode->mLobbyMinZ
        && chunkZ <= gameMode->mLobbyMaxZ)
        return true;
    return gameMode->mActiveChunkBounds.IsChunkWithinActiveArea(chunkX, chunkZ, gameMode->mActiveChunkAreas);
}

void MasterGameMode::GenerateChestRefillInfo()
{
    const MiniGameDef* miniGame = GetMiniGame();
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    ServerLevel* level = gMinecraftServer->getLevel(0);
    const std::wstring centralLootGroup = miniGame->getCentralLootGroup();
    const std::wstring outerLootGroup = miniGame->getOuterLootGroup();
    const std::wstring hvLootGroup = miniGame->getHVLootGroup();
    if (ruleset != NULL && level != NULL)
    {
        GameRuleDefinition* initialiseRule = ruleset->getRuleByType(ConsoleGameRules::eGameRuleType_OnInitialiseWorld);
        if (initialiseRule != NULL)
        {
            std::vector<GameRuleDefinition*> distributeRules;
            std::vector<WorldPositionRuleDefinition*> chestPositions;
            mChestRefills.clear();
            initialiseRule->getChildren(&distributeRules, ConsoleGameRules::eGameRuleType_DistributeItems);

            for (std::vector<GameRuleDefinition*>::iterator ruleIt = distributeRules.begin();
                 ruleIt != distributeRules.end(); ++ruleIt)
            {
                DistributeItemsRuleDefinition* distributeRule = dynamic_cast<DistributeItemsRuleDefinition*>(*ruleIt);
                if (distributeRule == NULL)
                    continue;

                if (!equalsIgnoreCase(distributeRule->getId(), L"StartItems")
                    && !equalsIgnoreCase(distributeRule->getId(), L"HVItems")
                    && !equalsIgnoreCase(distributeRule->getId(), L"OuterItems"))
                    continue;

                std::vector<GameRuleDefinition*> positionRules;
                distributeRule->getChildren(&positionRules);
                for (std::vector<GameRuleDefinition*>::iterator positionIt = positionRules.begin();
                     positionIt != positionRules.end(); ++positionIt)
                {
                    WorldPositionRuleDefinition* position = static_cast<WorldPositionRuleDefinition*>(*positionIt);
                    chestPositions.push_back(position);

                    ChestRefillInfo info;
                    info.pos = BlockPos(position->mX, position->mY, position->mZ);
                    info.itemSet = distributeRule->getId();
                    if (equalsIgnoreCase(info.itemSet, L"StartItems"))
                        info.lootGroup = centralLootGroup;
                    else if (equalsIgnoreCase(info.itemSet, L"HVItems"))
                        info.lootGroup = hvLootGroup;
                    else if (equalsIgnoreCase(info.itemSet, L"OuterItems"))
                        info.lootGroup = outerLootGroup;
                    info.distance = UNMEASURED_CHEST_DISTANCE;
                    mChestRefills.push_back(info);
                }
            }
        }
    }
}

bool MasterGameMode::LoadNewGameRules()
{
    MinecraftServer* server = gMinecraftServer;
    PlayerList* players = server == NULL ? NULL : server->mPlayers;
    if (mNextLevelGenOptions == NULL)
        return false;

    if (!mNextLevelGenOptions->hasLoadedData()
        || (mNextLevelGenOptions->requiresBaseSave() && !mNextLevelGenOptions->hasBaseSaveData()))
    {
        boost::shared_ptr<Packet> waitingPacket(
            new ClientboundChatPacket(ClientboundChatPacket::eChatPacketMessage_WaitingForLoad, -1));
        players->broadcastAll(waitingPacket);
        return false;
    }

    unsigned int texturePackId = mNextLevelGenOptions->getTexturePackId();
    if (texturePackId == 0)
        texturePackId = mTexturePackId;
    server->texturePackId = texturePackId;

    mKillboxes.clear();
    GetKillBoxes(mNextLevelGenOptions, mKillboxes);

    mActiveChunkAreas.clear();
    mNextLevelGenOptions->getRequiredGameRules()->getActiveChunkAreas(&mActiveChunkAreas);
    mActiveChunkBounds.DeactivateCache();
    mActiveChunkBounds.ClearCache();

    mActiveViewAreas.clear();
    mNextLevelGenOptions->getRequiredGameRules()->getActiveViewAreas(&mActiveViewAreas);

    mRoundInfo.setUniqueThemeId(mNextLevelGenOptions->getUniqueId());
    GameModePacket::ChangeCustomGameMode customGameMode(GetMiniGame()->GetId(), mRoundInfo.getThemeUniqueId());
    BroadcastGameModePacket(GameModePacket::eMessage_ChangeCustomGameMode, customGameMode);
    ServerPlayers()->broadcastAll(GetMiniGame()->GetPlayerSettingsPacket());
    ResetPlayers();

    GameModePacket::TransitionToRoundData transition(texturePackId, mRoundInfo.getThemeId(), mRoundInfo.getThemeSubId(),
                                                     mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ);
    players->broadcastAll(GameModePacket::Create(GameModePacket::eMessage_TransitionToRound, transition));

    gApp.setLevelGenerationOptions(mNextLevelGenOptions);
    server->getLevel(0)->refreshFromLevelGen();
    gRegionFileCache._clear();
    players->resetAllChunkFlagsOutsideArea(mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ, 0);
    server->getLevel(0)->invalidateChunksOutsideArea(mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ);

    if (GetMiniGame()->LoadOnlyActiveAreaChunks())
    {
        ServerChunkCache* chunkCache = static_cast<ServerChunkCache*>(server->getLevel(0)->getChunkSource());
        chunkCache->setOverrideBoundFunction(IsChunkWithinActiveBounds, this);
    }

    ServerPlayers()->resetAllChunkFlagsOutsideArea(mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ, 0);
    gMinecraftServer->getLevel(0)->getChunkMap()->reloadAllPlayers(mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ);

    ResetReadyStates(true);
    mCurrentLevelGenOptions = mNextLevelGenOptions;
    mNextLevelGenOptions = NULL;
    mVoteables->getVoteable(Voteables::eVoteable_Map)->clearVotes();
    GenerateChestRefillInfo();
    return true;
}

void MasterGameMode::SetLockPlayerPositions(bool locked)
{
    GameModePacket::LockPlayerLocation data(locked);
    BroadcastGameModePacket(GameModePacket::eMessage_LockPlayerLocation, data);

    std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        player->SetPositionLocked(locked);
    }
}

void MasterGameMode::UpdateGeneratorSeed()
{
    Random random;
    const long long seed = random.nextLong();
    const int playerCount = std::max(std::min(GetPlayerCount(false, false), MAX_SCALED_PLAYERS), MIN_SCALED_PLAYERS);
    const float playerScale = sqrtf(static_cast<float>(playerCount)) / sqrtf(static_cast<float>(MAX_SCALED_PLAYERS));
    CommonMasterGameMode::UpdateGeneratorSeed(seed < 0 ? -seed : seed,
                                              static_cast<float>(GetMiniGame()->GetLayerScale(playerScale)));
}

AABB* MasterGameMode::GeneratorSpawn() const
{
    if (mMapGenerator == NULL)
        return NULL;

    TumbleGenerator* tumbleGenerator = dynamic_cast<TumbleGenerator*>(mMapGenerator);
    if (tumbleGenerator != NULL)
        return tumbleGenerator->GetSpawnArea(mSpawnLayerIndex);
    return mMapGenerator->GetSpawnArea();
}

boost::shared_ptr<ServerPlayer> MasterGameMode::GetRoundWinner()
{
    return mGameStats.GetRoundWinner();
}

int MasterGameMode::GetSpawnLayerIndex() const
{
    return mSpawnLayerIndex;
}

void MasterGameMode::PlacePlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players, bool atStart, bool spectators)
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (player->mSpectatorSkullEquipped)
            player->setItemSlot(EquipmentSlot::HEAD, ItemInstance::EMPTY);
    }

    if (ruleset != NULL)
    {
        if (GetMiniGame()->IsTeamBasedSpawn())
        {
            std::vector<AABB*> teamAreas;
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
                 playerIt != players->end(); ++playerIt)
            {
                boost::shared_ptr<ServerPlayer> player = *playerIt;
                const unsigned int teamIndex = player->getTeam()->getTeamIndex();
                gApp.getLevelGenerationOptions()->getRequiredGameRules()->getNamedAreas(1 << teamIndex, &teamAreas);

                AABB* teamArea = teamAreas.front();
                if (teamAreas.size() > 1)
                    teamAreas.at(mRandom.nextInt(static_cast<int>(teamAreas.size()) - 1));
                GameModeUtilities::SpawnWithinArea(player, teamArea);
                teamAreas.clear();
            }
            return;
        }

        if (Minecraft::InMiniGame(eMiniGameId_Glide, false) && GetMiniGame()->IsCheckpointBasedSpawn())
        {
            std::vector<CheckpointRuleDefinition*>* checkpoints =
                gApp.getLevelGenerationOptions()->getRequiredGameRules()->getCheckpoints();
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
                 playerIt != players->end(); ++playerIt)
            {
                boost::shared_ptr<ServerPlayer> player = *playerIt;
                ServerPlayerGameMode* playerGameMode = static_cast<ServerPlayerGameMode*>(player->GetGameMode());
                if (atStart)
                    playerGameMode->setLastCheckpoint(0);

                for (int checkpointIndex = static_cast<int>(playerGameMode->getLastCheckpoint()); checkpointIndex >= 0;
                     --checkpointIndex)
                {
                    if (static_cast<unsigned int>(checkpointIndex) >= checkpoints->size())
                        continue;

                    if (!(*checkpoints)[checkpointIndex]->hasSpawnPoint())
                        continue;

                    (*checkpoints)[checkpointIndex]->setPlayerPosition(player);
                    playerGameMode->ResetTempPoints();
                    mGameStats.RecordSpawnedAtCheckpoint(player->getSUID(),
                                                         (*checkpoints)[checkpointIndex]->getSequenceID());
                    playerGameMode->RecordLastCheckpoint(checkpointIndex,
                                                         (*checkpoints)[checkpointIndex]->getSequenceID(), false,
                                                         true, true);
                    ResetLastKnownPosition(player->mPlayerIndex);
                    break;
                }
            }
            return;
        }

        OnGameStartSpawnPositions* spawnPositions = static_cast<OnGameStartSpawnPositions*>(
            ruleset->getRuleByType(ConsoleGameRules::eGameRuleType_OnGameStartSpawnPositions));
        if (spawnPositions != NULL)
        {
            const bool centreSpawn = !spectators
                                     && GetMiniGame()->HostOption_Bool(eCustomGameModeHostOption_CentreSpawn);
            GameRuleDefinition* spawnSet = spawnPositions->getSpawnSet(
                centreSpawn ? SpawnPositionSet::eSpawnSetType_Centre : SpawnPositionSet::eSpawnSetType_Standard);
            if (spawnSet == NULL)
            {
                std::vector<GameRuleDefinition*> spawnSets;
                spawnPositions->getChildren(&spawnSets);
                spawnSet = spawnSets.front();
            }

            std::vector<GameRuleDefinition*> spawns;
            if (spawnSet != NULL)
                spawnSet->getChildren(&spawns);

            SpawnPositionSet* winnerSpawnSet = spawnPositions->getSpawnSet(SpawnPositionSet::eSpawnSetType_Winner);
            if (winnerSpawnSet == NULL)
            {
                GameModeUtilities::DistributePlayers(players, &spawns, &mPlayerSpawnSlots);
                return;
            }

            std::vector<boost::shared_ptr<ServerPlayer> > winners;
            std::vector<boost::shared_ptr<ServerPlayer> > others;
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
                 playerIt != players->end(); ++playerIt)
            {
                bool isWinner = false;
                if (GetRoundWinner() != NULL && (*playerIt)->mPlayerIndex == GetRoundWinner()->mPlayerIndex
                    && CGameNetworkManager::sInstance.GetPlayerCount() > 1)
                    isWinner = true;
                (isWinner ? winners : others).push_back(*playerIt);
            }

            std::vector<GameRuleDefinition*> winnerSpawns;
            winnerSpawnSet->getChildren(&winnerSpawns);
            GameModeUtilities::DistributePlayers(&winners, &winnerSpawns, NULL);
            GameModeUtilities::DistributePlayers(&others, &spawns, &mPlayerSpawnSlots);
            return;
        }

        AABB* spawnArea = GeneratorSpawn() != NULL
                              ? GeneratorSpawn()
                              : gApp.getLevelGenerationOptions()->getRequiredGameRules()->getNamedArea(L"RoundSpawn");
        if (spawnArea != NULL)
        {
            TumbleGenerator* tumbleGenerator =
                mMapGenerator == NULL ? NULL : dynamic_cast<TumbleGenerator*>(mMapGenerator);
            GameModeUtilities::SpreadPlayers(players, spawnArea, SPREAD_PLAYERS_DISTANCE,
                                             gMinecraftServer->getLevel(0), false, tumbleGenerator);
        }
    }
}

void MasterGameMode::PositionPlayersAtStartingPosition()
{
    PlacePlayers(ServerPlayers()->getPlayers(), true, false);
}

void MasterGameMode::RemoveAllEntities(bool keepLobby)
{
    ServerLevel* level = gMinecraftServer->getLevel(0);
    if (keepLobby)
        level->removeAllEntities(true, true, true, true, mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ);
    else
        level->removeAllEntities(false, true, false, true, 1, 1, 0, 0);
}

void MasterGameMode::AddPlayersToTeams()
{
    std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
    if (GetTeamCount() > 1)
        GameModeUtilities::DistributePlayersInTeams(players, gMinecraftServer->getLevel(0)->GetScoreboard());
}

bool MasterGameMode::CanRoundStart()
{
    GameCondition* startConditions = GetMiniGame()->GetStartConditions();
    if (startConditions == NULL)
        return false;

    startConditions->UpdateMessage(this, IDS_MG_START);
    return true;
}

void MasterGameMode::SetGameMode(const GameType* gameType)
{
    std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
        (*playerIt)->setGameMode(gameType);
}

void MasterGameMode::ResetPlayersFlyingState(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
        (*playerIt)->stopFallFlying();

    if (!GetMiniGame()->HasFeatureElytraFlyingOnSpawn())
        return;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        AABB* jumpOffPoint = gApp.getLevelGenerationOptions()->getRequiredGameRules()->getNamedArea(L"JumpOffPoint");
        if (jumpOffPoint == NULL || !jumpOffPoint->containsIncludingLowerBound(player->getPos(1.0f)))
        {
            player->startFallFlying();
            player->SetGlideCollisionDamage(true);
        }
    }
}

void MasterGameMode::ResetPlayerHealths(std::vector<boost::shared_ptr<ServerPlayer> >* players, float health)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        player->setHealth(health < 0.0f ? player->getMaxHealth() : health);
    }
}

void MasterGameMode::SetPlayerAbilities(boost::shared_ptr<ServerPlayer> player, const Abilities* abilities)
{
    if (player->isSpectator())
        return;

    player->mAbilities = *abilities;
    player->onUpdateAbilities();
}

void MasterGameMode::SetPlayersAbilities(std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                         const Abilities* abilities)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
        SetPlayerAbilities(*playerIt, abilities);
}

void MasterGameMode::UpdatePlayersAbilities(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    const MiniGameDef* miniGame = GetMiniGame();
    const Abilities abilities = miniGame->getPlayerAbilities();
    switch (miniGame->GetId())
    {
    case eMiniGameId_Tumble:
        ResetPlayerHealths(players, TUMBLE_PLAYER_HEALTH);
        break;
    case eMiniGameId_Glide:
        ResetPlayerHealths(players, GLIDE_PLAYER_HEALTH);
        break;
    default:
        ResetPlayerHealths(players, FULL_PLAYER_HEALTH);
        break;
    }
    SetPlayersAbilities(players, &abilities);
}

bool MasterGameMode::ShouldRemoveArmourPiece(int itemId)
{
    if (itemId == ItemId::SKULL || itemId == BlockId::CARVED_PUMPKIN)
        return false;
    return true;
}

void MasterGameMode::EmptyPlayerInventory(std::vector<boost::shared_ptr<ServerPlayer> >* players, bool clearAll)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<Inventory> inventory = (*playerIt)->mInventory;
        if (clearAll)
        {
            inventory->clearContent();
            continue;
        }

        for (int slot = 0; slot < Inventory::INVENTORY_SIZE; ++slot)
            inventory->removeItemNoUpdate(slot);

        for (unsigned int armorSlot = 0; armorSlot < inventory->m_armor.size(); ++armorSlot)
        {
            not_null_ptr<ItemInstance>& armor = inventory->m_armor[armorSlot];
            if (!armor->isEmpty() && ShouldRemoveArmourPiece(Item::getId(armor->getItem())))
                armor = ItemInstance::EMPTY;
        }
        inventory->m_offhand[0] = ItemInstance::EMPTY;
    }
}

void MasterGameMode::ResetPlayersEquipmentAndInventory(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    EmptyPlayerInventory(players, false);

    if (GetMiniGame()->HasLootGroup(MiniGameDef::sPlayerInventoryLootGroup))
    {
        std::vector<boost::shared_ptr<Container> > inventories;
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
             playerIt != players->end(); ++playerIt)
            inventories.push_back((*playerIt)->mInventory);

        GetMiniGame()->PopulateContainers(&inventories, MiniGameDef::sPlayerInventoryLootGroup);
        GetMiniGame()->EquipPlayers(players);
    }

    if (mShowdownStartTime == 0 || GetMiniGame()->GetId() != eMiniGameId_Tumble)
        return;

    if (GetMiniGame()->GetGameVariant() != 0 && (!GetMiniGame()->IsCustomGame() || GetMiniGame()->GetLootSetId() != 0))
        return;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        player->setSlot(player->mInventory->m_selected,
                        not_null_ptr<ItemInstance>(new ItemInstance(Items::snowball, 1)));
    }
}

void MasterGameMode::PreparePlayersForGamemode(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    ResetPlayersFlyingState(players);
    UpdatePlayersAbilities(players);
    ResetPlayersEquipmentAndInventory(players);
}

void MasterGameMode::ResetGameRules()
{
    if (GetMiniGame()->GetId() != eMiniGameId_Glide)
        return;

    std::vector<ThermalAreaRuleDefinition*> thermalAreas;
    gApp.getLevelGenerationOptions()->getRequiredGameRules()->getThermalAreas(&thermalAreas);
    for (std::vector<ThermalAreaRuleDefinition*>::iterator areaIt = thermalAreas.begin(); areaIt != thermalAreas.end();
         ++areaIt)
        (*areaIt)->resetConditions();
}

void MasterGameMode::SetPlayerInvulnerable(boost::shared_ptr<ServerPlayer> player, bool invulnerable)
{
    const bool applyInvulnerable = invulnerable || GetMiniGame()->getPlayerAbilities().mInvulnerable;
    player->setInvulnerable(applyInvulnerable);
    player->mAbilities.mInvulnerable = applyInvulnerable;
    ServerPlayers()->broadcastAll(
        boost::shared_ptr<Packet>(new ClientboundPlayerAbilitiesPacket(&player->mAbilities, player->getId())));
}

void MasterGameMode::SetPlayersInvulnerable(bool invulnerable)
{
    mPlayersInvulnerable = invulnerable;

    std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (!player->isSpectator())
            SetPlayerInvulnerable(player, invulnerable);
    }
}

MasterGameMode::CountdownInfo::CountdownInfo()
{
    message = 0;
    arguments = std::vector<int>();
    callbackData = NULL;
    tickCallback = NULL;
    completeCallback = NULL;
}

bool MasterGameMode::StartTimer(ETimerID timer, CountdownInfo* countdown, int duration)
{
    MiniGameDef::TimerSpec specification = Minecraft::GetInstance()->GetMiniGame()->getTimerSpec(timer);
    if (duration <= 0)
        return false;

    specification.SetDurationS(duration);
    AddTimer(timer, OnTimerCountdown, countdown, specification);
    return true;
}

bool MasterGameMode::StartTimer(ETimerID timer, CountdownInfo* countdown)
{
    MiniGameDef::TimerSpec specification = Minecraft::GetInstance()->GetMiniGame()->getTimerSpec(timer);
    return StartTimer(timer, countdown, specification.TotalDurationS());
}

void MasterGameMode::SetLastKnownPosition(boost::shared_ptr<Player> player)
{
    const int playerIndex = player->mPlayerIndex;
    Vec3* position = player->getPos(1.0f);
    if (mLastKnownPositions[playerIndex] == NULL)
        mLastKnownPositions[playerIndex] = Vec3::newPermanent(position->x, position->y, position->z);
    else
        mLastKnownPositions[playerIndex]->set(position);
}

void MasterGameMode::UpdateClientsCameras(boost::shared_ptr<ServerPlayer> player)
{
    player->updateClientsCameras();
}

void MasterGameMode::Tick_Round_Playing()
{
    const std::vector<boost::shared_ptr<ServerPlayer> > roundPlayers = GetPlayers(true, false);
    std::vector<boost::shared_ptr<Player> > players;
    players.insert(players.end(), roundPlayers.begin(), roundPlayers.end());

    const MiniGameDef* miniGame = GetMiniGame();
    CheckCheckpoints(players);
    CheckTargets(players);
    CheckPowerups(players);
    CheckProgress(players);
    mGameStats.RecordTimeSpentInPosition(miniGame->ArePointsActive());

    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator playerIt = roundPlayers.begin();
         playerIt != roundPlayers.end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        const StatsUID suid = player->getSUID();

        if (GetMiniGame()->GetId() == eMiniGameId_Glide)
        {
            AABB* jumpOffPoint = gApp.getLevelGenerationOptions()->getRequiredGameRules()->getNamedArea(
                L"JumpOffPoint");
            bool gliding = !mGameStats.GetEndZoneEntered(suid);
            if (gliding && jumpOffPoint != NULL && jumpOffPoint->containsIncludingLowerBound(player->getPos(1.0f)))
                gliding = false;

            if (gliding)
            {
                mGameStats.TakeTimestamp(player->getSUID(), eTimeSpentCategory_NearBlocks,
                                         !player->getLevel()
                                              ->getCollisionAABBs(player,
                                                                  player->mBoundingBox->grow(NEAR_BLOCKS_DISTANCE),
                                                                  false, false, false)
                                              ->empty());
                mGameStats.TakeTimestamp(player->getSUID(), eTimeSpentCategory_InWater, player->isInWater());
            }
            player->SetGlideCollisionDamage(gliding && player->isAlive());

            if (mGameStats.GetEndZoneEntered(suid) && !player->mAbilities.mInvulnerable)
                SetPlayerInvulnerable(player, mGameStats.GetEndZoneEntered(suid) != 0);
            else if (player->mAbilities.mInvulnerable
                     && !player->CheckPowerup(PowerupItems::eGlide_Timed_Powerup_ID_Invulnerable))
                SetPlayerInvulnerable(player, false);

            if (mBeaconCheckpointIndex < mGameStats.GetFarthestCheckpointReached())
            {
                for (unsigned int checkpoint = mBeaconCheckpointIndex;
                     checkpoint < mGameStats.GetFarthestCheckpointReached() + BEACON_CHECKPOINTS_AHEAD; ++checkpoint)
                {
                    if (checkpoint < mCheckpoints.size())
                        mCheckpoints.at(checkpoint)->processBeacons(gMinecraftServer->getLevel(0));
                }
                mBeaconCheckpointIndex = mGameStats.GetFarthestCheckpointReached();
            }
        }

        if (std::find(mLastKnownPositionPlayers.begin(), mLastKnownPositionPlayers.end(), suid)
                != mLastKnownPositionPlayers.end()
            && mGameStats.GetPlayerStats(suid) != NULL)
        {
            const unsigned int deathCount = mGameStats.GetDeathCount(suid);
            player->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(suid, deathCount);
        }
    }
    mLastKnownPositionPlayers.clear();

    std::vector<boost::shared_ptr<ServerPlayer> > allPlayers = GetPlayers(false, false);
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = allPlayers.begin();
         playerIt != allPlayers.end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        SetLastKnownPosition(player);

        if (GetMiniGame()->GetId() == eMiniGameId_Glide && player->isSpectator() && !player->IsSpectatingOtherPlayer())
        {
            boost::shared_ptr<ServerPlayer> trailingPlayer = mGameStats.GetTrailingPlayer(player);
            if (trailingPlayer != NULL)
            {
                player->SetCamera(trailingPlayer);
                UpdateClientsCameras(player);
            }
        }
    }
}

bool MasterGameMode::CanRoundEnd()
{
    bool announce = true;
    if (GetMiniGame()->ShowdownAnnouncements() && mShowdownStartTime == 0)
        announce = false;

    GameCondition* endConditions = GetMiniGame()->GetEndConditions();
    if (endConditions == NULL)
        return false;
    endConditions->UpdateMessage(this, announce ? IDS_MG_END : -1);
    return endConditions->IsSatisfied(this);
}

bool MasterGameMode::CanStartShowdown()
{
    if (!GetMiniGame()->IsShowdownEnabled())
        return false;

    const bool announce = GetMiniGame()->ShowdownAnnouncements();
    GameCondition* showdownConditions = GetMiniGame()->GetShowdownConditions();
    if (showdownConditions == NULL)
        return false;
    showdownConditions->UpdateMessage(this, announce ? IDS_MG_END : -1);
    return showdownConditions->IsSatisfied(this);
}

std::vector<boost::shared_ptr<ServerPlayer> > MasterGameMode::GetGlidePlayers()
{
    std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> > glidePlayers;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (!player->isSpectator())
        {
            glidePlayers.push_back(player);
        }
        else if (mGameStats.GetPlayerStats(player->getSUID()) != NULL
                 && mGameStats.GetPlayerStats(player->getSUID())->getReachedEnd(mGameStats.mCurrentRound))
        {
            glidePlayers.push_back(player);
        }
    }
    return glidePlayers;
}

int MasterGameMode::GetShowdownTime()
{
    if (mShowdownStartTime == 0)
        return 0;
    return static_cast<unsigned int>(GetCurrentTimeMillis() - mShowdownStartTime) / 1000;
}

void MasterGameMode::StartRoundEndTimer()
{
    if (GetShowdownTime() != 0)
        return;

    CountdownInfo* countdown = new CountdownInfo();
    countdown->message = ClientboundChatPacket::eChatPacketMessage_SecondsToReachEnd;

    const int duration =
        Minecraft::GetInstance()->GetMiniGame()->getTimerSpec(eTimerID_TimeToReachEnd).TotalDurationS();
    const int glidePlayerCount = static_cast<int>(GetGlidePlayers().size());
    const int reduction = Mth::clamp(((FULL_ROUND_END_TIMER_PLAYERS - glidePlayerCount) * duration) / 3, 0,
                                     (duration * 2) / 3);
    mShowdownStartTime -= reduction * 1000;
    StartTimer(eTimerID_TimeToReachEnd, countdown, duration - reduction);
}

void MasterGameMode::OnShowdownStart()
{
    if (GetMiniGame()->GetId() == eMiniGameId_Tumble
        && (GetMiniGame()->GetGameVariant() == 0
            || (GetMiniGame()->IsCustomGame() && GetMiniGame()->GetLootSetId() == 0)))
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
             playerIt != players->end(); ++playerIt)
        {
            boost::shared_ptr<ServerPlayer> player = *playerIt;
            player->setSlot(player->mInventory->m_selected,
                            not_null_ptr<ItemInstance>(new ItemInstance(Items::snowball, 1)));
        }
    }

    std::vector<PlayerTeam*>* teams = gMinecraftServer->getLevel(0)->GetScoreboard()->getPlayerTeams();
    for (std::vector<PlayerTeam*>::iterator teamIt = teams->begin(); teamIt != teams->end(); ++teamIt)
    {
        if ((*teamIt)->getName() != SPECTATOR_TEAM_NAME)
        {
            (*teamIt)->setNameTagVisibility(Team::ALWAYS);
            (*teamIt)->syncChanges();
        }
    }
    delete teams;

    BroadcastGameModePacket(GameModePacket::eMessage_Showdown, GameModePacket::EmptyData());

    if (GetMiniGame()->GetId() == eMiniGameId_Glide)
        StartRoundEndTimer();
}

std::vector<PlayerTeam*> MasterGameMode::GetActiveTeams()
{
    Scoreboard* scoreboard = gMinecraftServer->getLevel(0)->GetScoreboard();
    std::vector<boost::shared_ptr<ServerPlayer> > players = GetPlayers(true, false);
    std::vector<PlayerTeam*> teams;
    for (unsigned int index = 0; index < players.size(); ++index)
    {
        PlayerTeam* team = scoreboard->getPlayersTeam(players[index]->mXuid);
        if (std::find(teams.begin(), teams.end(), team) == teams.end())
            teams.push_back(team);
    }
    return teams;
}

bool MasterGameMode::CanEndPostGame()
{
    GameCondition* postGameConditions = GetMiniGame()->GetPostGameConditions();
    if (postGameConditions == NULL)
        return true;
    postGameConditions->UpdateMessage(this, IDS_MG_START);
    return postGameConditions->IsSatisfied(this);
}

bool MasterGameMode::ShouldPlayAnotherRound()
{
    const int nextRound = mGameStats.mCurrentRound + 1;
    switch (GetMiniGame()->GetId())
    {
    case eMiniGameId_Battle:
    case eMiniGameId_Glide:
        return nextRound < static_cast<int>(GetMiniGame()->GetRoundCount())
               && static_cast<unsigned int>(GetPlayerCount(false, false))
                      >= static_cast<unsigned int>(GetMinimumPlayersToStart());
    case eMiniGameId_Tumble:
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
        if (players != NULL)
        {
            for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
                 playerIt != players->end(); ++playerIt)
            {
                if (mGameStats.GetRoundsWon((*playerIt)->getSUID()) >= TUMBLE_ROUNDS_TO_WIN)
                    return false;
            }
        }
        return static_cast<unsigned int>(GetPlayerCount(false, false))
               >= static_cast<unsigned int>(GetMinimumPlayersToStart());
    }
    default:
        return false;
    }
}

void MasterGameMode::ReturnPlayersToLobby()
{
    PlayerList* playerList = ServerPlayers();
    PlayerChunkMap* chunkMap = gMinecraftServer->getLevel(0)->getChunkMap();
    gMinecraftServer->texturePackId = 0;
    mKillboxes.clear();

    Minecraft::GetInstance()->SetupMiniGameInstance(*MiniGameDef::GetCustomGameModeById(eMiniGameId_Lobby, true), 0);
    memset(mPlayerSpawnSlots.data, EMPTY_SPAWN_SLOT, mPlayerSpawnSlots.length);
    memset(mSpectatorSpawnSlots.data, EMPTY_SPAWN_SLOT, mSpectatorSpawnSlots.length);

    playerList->broadcastAll(GameModePacket::Create(GameModePacket::eMessage_ChangeCustomGameMode,
                                                    GameModePacket::ChangeCustomGameMode(GetMiniGame()->GetId(), -1)));
    playerList->broadcastAll(
        GameModePacket::Create(GameModePacket::eMessage_Transition,
                               GameModePacket::TransitionData(mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ)));
    chunkMap->removeOutsideAreaForAllPlayers(mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ);

    File baseSaveFile(mLobbyLevelGenOptions->getBaseSaveRoot() + mLobbyLevelGenOptions->getBaseSavePath());
    mLobbyLevelGenOptions->setBaseSaveData(baseSaveFile);
    gApp.setLevelGenerationOptions(mLobbyLevelGenOptions);
    gMinecraftServer->getLevel(0)->refreshFromLevelGen();
    gRegionFileCache._clear();

    std::vector<NamedAreaRuleDefinition*> boundsVolumes;
    mLobbyLevelGenOptions->getRequiredGameRules()->getPlayerBoundsVolumes(&boundsVolumes);

    std::vector<boost::shared_ptr<ServerPlayer> > players = *ServerPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players.begin(); playerIt != players.end();
         ++playerIt)
    {
        (*playerIt)->setGameMode(GetMiniGame()->getPlayerGameType());
        (*playerIt)->mConnection->send(
            GameModePacket::Create(GameModePacket::eMessage_ClearAreaBounds, GameModePacket::EmptyData()));

        for (std::vector<NamedAreaRuleDefinition*>::iterator volumeIt = boundsVolumes.begin();
             volumeIt != boundsVolumes.end(); ++volumeIt)
        {
            AABB* area = (*volumeIt)->getArea();
            GameModePacket::AddAreaBoundsData bounds(static_cast<int>(area->mMinX), static_cast<int>(area->mMinY),
                                                     static_cast<int>(area->mMinZ), static_cast<int>(area->mMaxX),
                                                     static_cast<int>(area->mMaxY), static_cast<int>(area->mMaxZ));
            (*playerIt)->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_AddAreaBounds, bounds));
        }

        (*playerIt)->mChangedDimension = true;
        gMinecraftServer->getPlayers()->respawn(*playerIt, 0, false);
    }

    ServerChunkCache* chunkCache = static_cast<ServerChunkCache*>(gMinecraftServer->getLevel(0)->getChunkSource());
    chunkCache->invalidateChunksOutsideArea(mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ);
    playerList->resetAllChunkFlagsOutsideArea(mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ, 0);
    static_cast<ServerChunkCache*>(gMinecraftServer->getLevel(0)->getChunkSource())
        ->setOverrideBoundFunction(NULL, NULL);

    mActiveChunkAreas.clear();
    AABB* activeArea = mLobbyLevelGenOptions->getRequiredGameRules()->getNamedArea(L"ActiveArea");
    if (activeArea != NULL)
        mActiveChunkAreas.push_back(activeArea);
    mActiveChunkBounds.DeactivateCache();
    mActiveChunkBounds.ClearCache();
    chunkMap->reloadAllPlayers(mLobbyMinX, mLobbyMinZ, mLobbyMaxX, mLobbyMaxZ);

    ResetReadyStates(false);
    EmptyPlayerInventory(ServerPlayers()->getPlayers(), true);
}

void MasterGameMode::Tick()
{
    CommonMasterGameMode::Tick();

    if (GetMiniGame()->GetId() != eMiniGameId_Glide
        || !GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) || !gMinecraftServer->IsServerPaused())
    {
        TickTimers();
    }
    CheckTutorialUpdate();

    std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (IsInKillBox(player))
            player->hurt(DamageSource::CREATIVE_PLAYER, KILLBOX_DAMAGE);
    }

    if (mBlockRemover != NULL)
    {
        if (!mBlockRemover->Finished())
        {
            mBlockRemover->Tick();
        }
        else
        {
            delete mBlockRemover;
            mBlockRemover = NULL;
        }
    }

    switch (GetState())
    {
    case eInternalGameModeState_Initialising:
        if (!AreLevelChunksLoaded())
            break;

        mTexturePackId = Minecraft::GetInstance()->mTexturePackRepository->getSelected()->getId();
        mPlayedRuleGroups.clear();
        mRemainingRuleGroups.clear();
        mRemainingRuleGroups.insert(mRemainingRuleGroups.end(), mPlaylist.begin(), mPlaylist.end());
        std::random_shuffle(mRemainingRuleGroups.begin(), mRemainingRuleGroups.end());
        ChooseNextGameRules(true);
        mLobbyChestsPopulated = false;
        SetState(eInternalGameModeState_Lobby);
        sendTutorialUpdate(eTutorial_GameMode_Lobby);
        break;

    case eInternalGameModeState_LobbyCountdown:
        CheckReadyStates();
        if (!HasTimer(eTimerID_LobbyInterval))
        {
            CountdownInfo* countdown = new CountdownInfo();
            countdown->message = ClientboundChatPacket::eChatPacketMessage_TimeToGameChangeCountdown;
            countdown->completeCallback = OnGameChange;
            AddTimer(eTimerID_LobbyInterval, OnTimerCountdown, countdown,
                     GetMiniGame()->getTimerSpec(eTimerID_LobbyInterval));
        }
        else if (GetMiniGame()->CanAutoStart()
                 && static_cast<unsigned int>(GetPlayerCount(false, false))
                        < static_cast<unsigned int>(GetMinimumPlayersToStart()))
        {
            KillTimer(eTimerID_LobbyInterval);
            SetState(eInternalGameModeState_Lobby);
            mLobbyChestsPopulated = true;
        }
        break;

    case eInternalGameModeState_Lobby:
        if (AreLevelChunksLoaded() && !mLobbyChestsPopulated)
        {
            PopulateChests();
            mLobbyChestsPopulated = true;
        }

        if (GetMiniGame()->CanAutoStart())
        {
            const unsigned int minimumPlayers = GetMinimumPlayersToStart();
            const unsigned int playerCount = GetPlayerCount(false, false);
            if (playerCount < minimumPlayers)
            {
                std::vector<int> arguments;
                arguments.push_back(minimumPlayers - playerCount);
                BroadcastChat(ClientboundChatPacket::eChatPacketMessage_MorePlayersRequired, arguments);
            }
            else
            {
                SetState(eInternalGameModeState_LobbyCountdown);
            }
        }
        break;

    case eInternalGameModeState_SelectingGameRules:
        BroadcastChatMessage(ClientboundChatPacket::eChatPacketMessage_SelectingMode);
        SelectNewGameRules();
        mGameRulesChanged = true;
        SetState(eInternalGameModeState_LoadingGameRules);
        break;

    case eInternalGameModeState_LoadingGameRules:
        if (mNextLevelGenOptions != NULL)
        {
            mRoundInfo.reset();
            if (LoadNewGameRules())
            {
                SetLockPlayerPositions(true);
                SetState(eInternalGameModeState_LoadingRound);
                if (GetMiniGame()->GetId() == eMiniGameId_Tumble || GetMiniGame()->GetId() == eMiniGameId_Build)
                {
                    UpdateGeneratorSeed();
                    RestartMapGenerator();
                }
                else
                {
                    RemoveMapGenerator();
                }
            }
        }
        else
        {
            SetState(eInternalGameModeState_SelectingGameRules);
        }
        break;

    case eInternalGameModeState_LoadingRound:
    case eInternalGameModeState_LoadingSubRound:
        if (AreLevelChunksLoaded() && !mWaitingForChunkLoading)
        {
            const bool newRound = GetState() == eInternalGameModeState_LoadingRound;
            PositionPlayersAtStartingPosition();
            RemoveAllEntities(false);
            if (newRound)
            {
                mRoundInfo.startNewRound();
                gMinecraftServer->getLevel(0)->GetScoreboard()->ResetPlayerScores();
                mGameStats.Clear();
                mGameStats.OnGameStart();
            }
            else
            {
                mRoundInfo.startNewSubRound();
            }

            mWaitingForChunkLoading = true;
            mPlayersLoading = static_cast<int>(gMinecraftServer->getPlayers()->getPlayers()->size());
            mChunkLoadStartTime = System::processTimeInSeconds();
            mChunkLoadStartTick = mTickCount;
            PlayerChunkMap* chunkMap = static_cast<ServerLevel*>(GeneratorTargetLevel())->getChunkMap();
            chunkMap->PauseBlockUpdatePackets(true);
            std::vector<boost::shared_ptr<ServerPlayer> >* loading = ServerPlayers()->getPlayers();
            for (unsigned int index = 0; index < loading->size(); ++index)
            {
                boost::shared_ptr<ServerPlayer> player = loading->at(index);
                const int pending = chunkMap->getBlockRegionPacketsToBeSent(player);
#ifdef MINECRAFT_PC
                const std::wstring wideName = player->getName();
                const std::string name(wideName.begin(), wideName.end());
                printf("[pc] mini game: %s waits for %d region updates of the new map, placed at %.1f %.1f %.1f\n",
                       name.c_str(), pending, player->mX, player->mY, player->mZ);
                fflush(stdout);
#endif
                player->mConnection->send(
                    GameModePacket::Create(GameModePacket::eMessage_WaitForChunkLoading, GameModePacket::IntData(pending)));
            }
        }

        {
            const bool playersLoaded = mPlayersLoading <= 0;
            const bool loadTimedOut = static_cast<long long>(System::processTimeInSeconds() - mChunkLoadStartTime)
                                      > CHUNK_LOAD_TIMEOUT_SECONDS;
            if (mWaitingForChunkLoading && mTickCount == mChunkLoadStartTick + 1)
                static_cast<ServerLevel*>(GeneratorTargetLevel())->getChunkMap()->PauseBlockUpdatePackets(false);

            if (mWaitingForChunkLoading && (playersLoaded || loadTimedOut))
            {
                mWaitingForChunkLoading = false;
                SetState(eInternalGameModeState_PreparingRound);
                GameModePacket::RoundStartGameModeData roundStart(mRoundInfo.getRoundId(), mRoundInfo.getSubRoundId());
                BroadcastGameModePacket(GameModePacket::eMessage_RoundStart, roundStart);
                static_cast<ServerLevel*>(GeneratorTargetLevel())->getChunkMap()->PauseBlockUpdatePackets(false);
            }
        }
        break;

    case eInternalGameModeState_PreparingRound:
        if (GeneratorSpawn() != NULL)
            PositionPlayersAtStartingPosition();
        if (mRoundInfo.getCurrentSubRoundNumber() > 0)
            SetupTeams();
        AddPlayersToTeams();
        SetState(eInternalGameModeState_WaitingToStart);
        AddTimer(eTimerID_RoundPreparing, OnWaitingToStartCountdown, NULL,
                 GetMiniGame()->getTimerSpec(eTimerID_RoundPreparing));
        if (mKillboxes.empty() && mCurrentLevelGenOptions != NULL)
            GetKillBoxes(mCurrentLevelGenOptions, mKillboxes);
        break;

    case eInternalGameModeState_WaitingToStart:
    {
        if (!CanRoundStart())
            break;

        SetState(eInternalGameModeState_RoundStarting);
        sendTutorialUpdate(eTutorial_GameMode_RoundStarting);
        mGameStats.AddPlayers(ServerPlayers()->getPlayers());
        KillTimer(eTimerID_RoundPreparing);
        BroadcastChatMessage(ClientboundChatPacket::eChatPacketMessage_RoundStarting);

        mShowdownStartTime = 0;
        SetGameMode(GetMiniGame()->getPlayerGameType());
        PreparePlayersForGamemode(ServerPlayers()->getPlayers());
        ResetGameRules();
        SetPlayersInvulnerable(true);

        CountdownInfo* countdown = new CountdownInfo();
        countdown->message = ClientboundChatPacket::eChatPacketMessage_TimeToStartCountdown;
        countdown->completeCallback = OnGameStart;
        MiniGameDef::TimerSpec specification = GetMiniGame()->getTimerSpec(eTimerID_RoundStartDelay);
        if (GetMiniGame()->GetId() == eMiniGameId_Glide
            && GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial))
            specification.SetDurationS(static_cast<int>(specification.TotalDurationS()) / 3);
        AddTimer(eTimerID_RoundStartDelay, OnTimerCountdown, countdown, specification);
        mGameRulesChanged = false;

        const int duration = std::max(static_cast<int>(specification.TotalDurationS()), 0);
        mRoundStartTime = GetCurrentTimeMillis() + duration * 1000;
        mBeaconCheckpointIndex = 0;
        mPostRoundStartTime = 0;
        ResetMapDistances();
        mActiveChunkBounds.ActivateCache();
        break;
    }

    case eInternalGameModeState_RoundPlaying:
        Tick_Round_Playing();
        if (CanRoundEnd())
        {
            ServerPlayers()->broadcastAll(boost::shared_ptr<ClientboundChatPacket>(
                new ClientboundChatPacket(ClientboundChatPacket::eChatPacketMessage_RoundEnd, -1)));
            KillTimer(eTimerID_GracePeriod);
            KillTimer(eTimerID_RoundLimit);
            KillTimer(eTimerID_Showdown);
            KillTimer(eTimerID_TimeSincePlayers);
            KillTimer(eTimerID_ChestRefill);
            KillTimer(eTimerID_TimeToReachEnd);
            SetPlayersInvulnerable(true);
            mKillboxes.clear();
            mPostRoundTeamIndex = -1;
            mPostRoundStartTime = GetCurrentTimeMillis();
            SetState(eInternalGameModeState_PostRound);
            for (int playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex)
                ResetLastKnownPosition(playerIndex);

            if (mMapGenerator != NULL && GetMiniGame()->GetId() == eMiniGameId_Build)
            {
                std::vector<AABB*> internalWalls;
                static_cast<WallGenerator*>(mMapGenerator)->GetInternalWalls(&internalWalls);
                mBlockRemover = new DissolveBlockRemover(gMinecraftServer->getLevel(0), internalWalls,
                                                         WALL_DISSOLVE_PASSES, WALL_DISSOLVE_TICKS_PER_PASS);
            }
        }
        else if (mShowdownStartTime == 0 && CanStartShowdown())
        {
            mShowdownStartTime = GetCurrentTimeMillis();
            OnShowdownStart();
            sendTutorialUpdate(eTutorial_GameMode_Showdown);
        }
        break;

    case eInternalGameModeState_PostRound:
    {
        const std::vector<PlayerTeam*> teams = GetActiveTeams();
        const bool showTeamAreas = Minecraft::InMiniGame(eMiniGameId_Build, true)
                                   && GetMiniGame()->GetGameVariant() == 0;
        if (showTeamAreas && (mPostRoundTeamIndex < 0 || CanEndPostGame()))
        {
            ++mPostRoundTeamIndex;
            if (mPostRoundTeamIndex < static_cast<int>(teams.size()))
            {
                AABB* teamArea = GetTeamArea(teams.at(mPostRoundTeamIndex));
                std::vector<boost::shared_ptr<ServerPlayer> >* roundPlayers = ServerPlayers()->getPlayers();
                TumbleGenerator* tumbleGenerator = mMapGenerator == NULL
                                                       ? NULL
                                                       : dynamic_cast<TumbleGenerator*>(mMapGenerator);
                GameModeUtilities::SpreadPlayers(roundPlayers, teamArea, TEAM_AREA_SPREAD_DISTANCE,
                                                 gMinecraftServer->getLevel(0), false, tumbleGenerator);

                for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = roundPlayers->begin();
                     playerIt != roundPlayers->end(); ++playerIt)
                {
                    boost::shared_ptr<ServerPlayer> player = *playerIt;
                    if (teamArea != NULL)
                    {
                        GameModePacket::AddAreaBoundsData bounds(
                            static_cast<int>(teamArea->mMinX), static_cast<int>(teamArea->mMinY),
                            static_cast<int>(teamArea->mMinZ), static_cast<int>(teamArea->mMaxX),
                            static_cast<int>(teamArea->mMaxY), static_cast<int>(teamArea->mMaxZ));
                        player->mConnection->send(
                            GameModePacket::Create(GameModePacket::eMessage_SetAreaBounds, bounds));
                    }
                    else
                    {
                        player->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_ClearAreaBounds,
                                                                         GameModePacket::EmptyData()));
                    }
                }
                mPostRoundStartTime = GetCurrentTimeMillis();
            }
        }

        const bool allTeamsShown = mPostRoundTeamIndex >= static_cast<int>(teams.size());
        if (CanEndPostGame() && (!showTeamAreas || allTeamsShown))
        {
            std::vector<boost::shared_ptr<ServerPlayer> >* roundPlayers = ServerPlayers()->getPlayers();
            mGameStats.CalculateStats(roundPlayers, false);
            if (!Minecraft::InMiniGame(eMiniGameId_Glide, true))
            {
                boost::shared_ptr<ServerPlayer> winner = GetRoundWinner();
                for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = roundPlayers->begin();
                     playerIt != roundPlayers->end(); ++playerIt)
                {
                    boost::shared_ptr<ServerPlayer> player = *playerIt;
                    if (winner != NULL && player->getName() != winner->getName())
                    {
                        player->setGameMode(GameType::SPECTATOR);
                        player->SetCamera(winner);
                        UpdateClientsCameras(player);
                    }
                }

                std::vector<PlayerTeam*>* playerTeams =
                    gMinecraftServer->getLevel(0)->GetScoreboard()->getPlayerTeams();
                for (std::vector<PlayerTeam*>::iterator teamIt = playerTeams->begin(); teamIt != playerTeams->end();
                     ++teamIt)
                {
                    PlayerTeam* team = *teamIt;
                    if (team->getName().compare(SPECTATOR_TEAM_NAME) != 0)
                        team->setNameTagVisibility(Team::NEVER);
                }
                delete playerTeams;
            }

            CountdownInfo* countdown = new CountdownInfo();
            countdown->message = ClientboundChatPacket::eChatPacketMessage_TimeToRoundEndCountdown;
            countdown->completeCallback = OnGameEnd;
            MiniGameDef::TimerSpec specification = GetMiniGame()->getTimerSpec(eTimerID_RoundEndDelay);
            if (ShouldPlayAnotherRound())
                specification.SetDurationS(NEXT_ROUND_DELAY_SECONDS);
            AddTimer(eTimerID_RoundEndDelay, OnTimerCountdown, countdown, specification);
            SetState(eInternalGameModeState_RoundEnding);
        }
        break;
    }

    case eInternalGameModeState_ReturningToLobby:
    {
        std::vector<boost::shared_ptr<ServerPlayer> >* roundPlayers = ServerPlayers()->getPlayers();
        mGameStats.CalculateStats(roundPlayers, true);
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = roundPlayers->begin();
             playerIt != roundPlayers->end(); ++playerIt)
        {
            boost::shared_ptr<ServerPlayer> player = *playerIt;
            player->SetCamera(boost::shared_ptr<Entity>());
            UpdateClientsCameras(player);
        }

        RemoveMapGenerator();
        mGameStats.OnGameEnd();
        mCheckpoints.clear();
        RemoveAllEntities(true);
        ChooseNextGameRules(true);
        ReturnPlayersToLobby();
        mLobbyChestsPopulated = false;
        SetState(eInternalGameModeState_Lobby);
        mRoundInfo.reset();
        break;
    }

    case eInternalGameModeState_NextSubRound:
    {
        SetLockPlayerPositions(true);
        std::vector<boost::shared_ptr<ServerPlayer> >* roundPlayers = ServerPlayers()->getPlayers();
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = roundPlayers->begin();
             playerIt != roundPlayers->end(); ++playerIt)
        {
            boost::shared_ptr<ServerPlayer> player = *playerIt;
            player->SetCamera(boost::shared_ptr<Entity>());
            UpdateClientsCameras(player);
        }

        for (std::vector<ChestRefillInfo>::iterator chestIt = mChestRefills.begin(); chestIt != mChestRefills.end();
             ++chestIt)
        {
            boost::shared_ptr<Container> container = boost::dynamic_pointer_cast<Container>(
                gMinecraftServer->getLevel(0)->getBlockEntity(chestIt->pos));
            if (container != NULL)
                container->clearContent();
        }

        if (GetMiniGame()->GetId() == eMiniGameId_Tumble || GetMiniGame()->GetId() == eMiniGameId_Build)
        {
            CleanUpMapGenerator();
            UpdateGeneratorSeed();
            RestartMapGenerator();
        }

        mGameStats.OnRoundEnd();
        ResetPlayers();
        SetState(eInternalGameModeState_LoadingSubRound);
        break;
    }

    default:
        break;
    }

    ++mTickCount;
}

void MasterGameMode::SetupServerSettings(Settings* settings)
{
    settings->setBooleanAndSave(eServerSetting_SpawnAnimals, false);
    settings->setBooleanAndSave(eServerSetting_SpawnNpcs, false);
    settings->setBooleanAndSave(eServerSetting_SpawnMonsters, false);
    settings->setBooleanAndSave(eServerSetting_Pvp, true);
    settings->setBooleanAndSave(eServerSetting_DisableSaving, true);
    settings->setBooleanAndSave(eServerSetting_AllowNether, false);
}

const GameType* MasterGameMode::InitialGameType(boost::shared_ptr<ServerPlayer> player)
{
    switch (GetState())
    {
    case eInternalGameModeState_WaitingToStart:
    case eInternalGameModeState_RoundStarting:
    case eInternalGameModeState_RoundPlaying:
    case eInternalGameModeState_RoundEnding:
        return GameType::SPECTATOR;
    case eInternalGameModeState_Initialising:
    case eInternalGameModeState_LobbyCountdown:
    case eInternalGameModeState_Lobby:
    case eInternalGameModeState_SelectingGameRules:
    case eInternalGameModeState_LoadingGameRules:
    case eInternalGameModeState_LoadingRound:
    case eInternalGameModeState_LoadingSubRound:
    case eInternalGameModeState_PreparingRound:
    case eInternalGameModeState_PostRound:
    case eInternalGameModeState_ReturningToLobby:
    case eInternalGameModeState_NextSubRound:
        return GameType::LOBBY;
    default:
        return GameType::NOT_SET;
    }
}

bool MasterGameMode::IsInRound()
{
    bool inRound = false;
    switch (GetState())
    {
    case eInternalGameModeState_RoundStarting:
    case eInternalGameModeState_RoundPlaying:
    case eInternalGameModeState_RoundEnding:
        inRound = true;
        break;
    default:
        break;
    }
    return inRound;
}

void MasterGameMode::PlaceSpectators(std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (!GetMiniGame()->ForceSpectateModeFollow() && ruleset != NULL)
    {
        AABB* spectatorSpawn = ruleset->getNamedArea(L"SpectatorSpawn");
        if (spectatorSpawn != NULL)
        {
            for (unsigned int index = 0; index < players->size(); ++index)
                GameModeUtilities::SpawnWithinArea(players->at(index), spectatorSpawn);
        }
        else
        {
            std::vector<GameRuleDefinition*> updatePlayerRules;
            ruleset->getChildren(&updatePlayerRules, ConsoleGameRules::eGameRuleType_UpdatePlayer);
            if (updatePlayerRules.empty())
            {
                for (unsigned int index = 0; index < players->size(); ++index)
                    ruleset->postProcessPlayer(players->at(index));
            }
            else
            {
                GameModeUtilities::DistributePlayers(players, &updatePlayerRules, &mSpectatorSpawnSlots);
            }
        }
    }
    else
    {
        boost::shared_ptr<ServerPlayer> leadingPlayer = mGameStats.GetLeadingPlayer();
        if (leadingPlayer == NULL)
        {
            std::vector<boost::shared_ptr<ServerPlayer> > roundPlayers = GetPlayers(true, false);
            if (!roundPlayers.empty())
                leadingPlayer = roundPlayers.front();
        }

        if (leadingPlayer != NULL)
        {
            for (unsigned int index = 0; index < players->size(); ++index)
                players->at(index)->SetCamera(leadingPlayer);
        }
    }

    if (Minecraft::GetInstance()->GetSpectateMode() != eSpectateMode_FloatingHead)
        return;

    for (unsigned int index = 0; index < players->size(); ++index)
    {
        boost::shared_ptr<ServerPlayer> player = players->at(index);
        if (!player->SpectateAsSkull() || !player->getItemSlot(EquipmentSlot::HEAD)->isEmpty())
            continue;

        player->mSpectatorSkullEquipped = true;
        const bool wearPumpkin = SharedConstants::IS_HALLOWEEN_SEASON
                                 || (SharedConstants::IS_CHRISTMAS
                                     && ruleset->getGenerationOptions()->getTexturePackId()
                                            == PUMPKIN_SPECTATOR_TEXTURE_PACK_ID);
        if (wearPumpkin)
            player->setItemSlot(EquipmentSlot::HEAD,
                                not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carved_pumpkin)));
        else
            player->setItemSlot(EquipmentSlot::HEAD, not_null_ptr<ItemInstance>(new ItemInstance(Items::skull)));
    }
}

void MasterGameMode::OnSpawn(boost::shared_ptr<ServerPlayer> player, boost::shared_ptr<ServerPlayer> owner,
                             bool joining)
{
    if (!IsInRound())
    {
        std::vector<boost::shared_ptr<ServerPlayer> > spawningPlayers;
        spawningPlayers.push_back(player);
        PlacePlayers(&spawningPlayers, false, false);
    }
    else
    {
        if (gApp.getGameRuleDefinitions() != NULL)
        {
            const unsigned int livesPerRound = GetMiniGame()->GetLivesPerRound();
            if (livesPerRound != 0 && livesPerRound <= player->mGameMode->GetDeaths())
            {
                Scoreboard* scoreboard = gMinecraftServer->getLevel(0)->GetScoreboard();
                scoreboard->addPlayerToTeam(player->mXuid, scoreboard->getPlayerTeam(SPECTATOR_TEAM_NAME));
                std::vector<boost::shared_ptr<ServerPlayer> > spawningPlayers;
                spawningPlayers.push_back(player);
                PlaceSpectators(&spawningPlayers);
            }
            else
            {
                std::vector<boost::shared_ptr<ServerPlayer> > spawningPlayers;
                spawningPlayers.push_back(player);
                PlacePlayers(&spawningPlayers, false, true);
                ResetPlayersFlyingState(&spawningPlayers);
                ResetPlayersEquipmentAndInventory(&spawningPlayers);
            }
        }

        player->SetCamera(boost::shared_ptr<Entity>());
        boost::shared_ptr<ServerPlayer> winner = GetRoundWinner();
        if (winner != NULL && player != winner)
            player->setGameMode(GameType::SPECTATOR);
    }

    UpdateClientsCameras(player);
}

void MasterGameMode::OnSpawned(boost::shared_ptr<ServerPlayer> player, bool joining)
{
    if (!IsInRound())
        return;

    if (gApp.getGameRuleDefinitions() != NULL)
    {
        const unsigned int livesPerRound = GetMiniGame()->GetLivesPerRound();
        if (livesPerRound == 0 || livesPerRound > player->mGameMode->GetDeaths())
        {
            std::vector<boost::shared_ptr<ServerPlayer> > spawnedPlayers;
            spawnedPlayers.push_back(player);
            UpdatePlayersAbilities(&spawnedPlayers);
        }
    }

    boost::shared_ptr<ServerPlayer> winner = GetRoundWinner();
    if (winner != NULL && player != winner)
    {
        player->SetCamera(winner);
        UpdateClientsCameras(player);
    }

    const StatsUID suid = player->getSUID();
    if (mGameStats.GetPlayerStats(suid) != NULL)
        player->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(suid, mGameStats.GetDeathCount(suid));
}

void MasterGameMode::SendPlaylistInfo(boost::shared_ptr<ServerPlayer> player)
{
    player->mConnection->send(boost::shared_ptr<Packet>(new MapSelectInfoPacket(mPlaylistSyncData, false)));
}

void MasterGameMode::SetLockPlayerPosition(boost::shared_ptr<ServerPlayer> player, bool locked)
{
    player->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_LockPlayerLocation,
                                                     GameModePacket::LockPlayerLocation(locked)));
    player->SetPositionLocked(locked);
}

void MasterGameMode::OnJoin(boost::shared_ptr<ServerPlayer> player)
{
    int themeUniqueId = -1;
    if (!gApp.getLevelGenerationOptions()->isGameModeLobby())
        themeUniqueId = mCurrentLevelGenOptions->getUniqueId();
    GameModePacket::ChangeCustomGameMode changeGameMode(GetMiniGame()->GetId(), themeUniqueId);
    player->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_ChangeCustomGameMode, changeGameMode));
    player->mConnection->send(GetMiniGame()->GetPlayerSettingsPacket());

    ServerScoreboard* scoreboard = player->getLevel()->GetServerScoreboard();
    mGameStats.RemovePlayerStats(player);
    std::vector<boost::shared_ptr<ServerPlayer> > players = GetPlayers(false, false);
    scoreboard->SyncPlayerScores(&players, player);
    SendPlaylistInfo(player);
    UpdateClientState(player);

    for (int playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex)
        player->mConnection->send(
            boost::shared_ptr<Packet>(new PlayerReadyPacket(playerIndex, mPlayerReady[playerIndex])));
    mVoteables->syncAllVoteablesToPlayer(player);

    bool joinsRound;
    switch (GetState())
    {
    case eInternalGameModeState_LoadingRound:
    case eInternalGameModeState_LoadingSubRound:
    case eInternalGameModeState_PreparingRound:
    case eInternalGameModeState_WaitingToStart:
    case eInternalGameModeState_RoundStarting:
    case eInternalGameModeState_ReturningToLobby:
    case eInternalGameModeState_NextSubRound:
        joinsRound = true;
        break;
    default:
        joinsRound = false;
        break;
    }

    if (player->GetGameType() == GameType::SPECTATOR)
    {
        GameModePacket::UpdateStartTimeData startTime(GetCurrentTimeMillis() - mRoundStartTime);
        player->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_UpdateStartTime, startTime));
        player->sendMessage(ClientboundChatPacket::eChatPacketMessage_Spectating, NULL, 0, NULL, 0);

        std::vector<NamedAreaRuleDefinition*> boundsVolumes;
        gApp.getLevelGenerationOptions()->getRequiredGameRules()->getPlayerBoundsVolumes(&boundsVolumes);
        for (std::vector<NamedAreaRuleDefinition*>::iterator volumeIt = boundsVolumes.begin();
             volumeIt != boundsVolumes.end(); ++volumeIt)
        {
            AABB* area = (*volumeIt)->getArea();
            GameModePacket::AddAreaBoundsData bounds(static_cast<int>(area->mMinX), static_cast<int>(area->mMinY),
                                                     static_cast<int>(area->mMinZ), static_cast<int>(area->mMaxX),
                                                     static_cast<int>(area->mMaxY), static_cast<int>(area->mMaxZ));
            player->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_AddAreaBounds, bounds));
        }

        Scoreboard* levelScoreboard = gMinecraftServer->getLevel(0)->GetScoreboard();
        levelScoreboard->addPlayerToTeam(player->mXuid, levelScoreboard->getPlayerTeam(SPECTATOR_TEAM_NAME));
        std::vector<boost::shared_ptr<ServerPlayer> > joiningPlayers;
        joiningPlayers.push_back(player);
        PlaceSpectators(&joiningPlayers);
    }
    else if (joinsRound)
    {
        std::vector<boost::shared_ptr<ServerPlayer> > joiningPlayers;
        joiningPlayers.push_back(player);
        PlacePlayers(&joiningPlayers, false, false);
        PreparePlayersForGamemode(&joiningPlayers);
        mGameStats.AddPlayerStats(player);
        SetPlayerInvulnerable(player, true);
        SetLockPlayerPosition(player, true);
        GameModeUtilities::DistributePlayersInTeams(&joiningPlayers, gMinecraftServer->getLevel(0)->GetScoreboard());
    }
    else
    {
        std::vector<boost::shared_ptr<ServerPlayer> > joiningPlayers;
        joiningPlayers.push_back(player);
        PlacePlayers(&joiningPlayers, false, false);
    }

    const int themeId = mRoundInfo.getThemeId();
    if (themeId >= 0)
    {
        const int themeSubId = mRoundInfo.getThemeSubId();
        if (themeSubId >= 0)
        {
            GameModePacket::UpdateThemeIdData theme(themeId, themeSubId);
            player->mConnection->send(GameModePacket::Create(GameModePacket::eMessage_UpdateThemeId, theme));
        }
    }
    UpdateClientsCameras(player);

    for (std::vector<CheckpointRuleDefinition*>::iterator checkpointIt = mCheckpoints.begin();
         checkpointIt != mCheckpoints.end(); ++checkpointIt)
        (*checkpointIt)->processBeacons(gMinecraftServer->getLevel(0));

    if (Minecraft::InMiniGame(eMiniGameId_Battle, true))
    {
        const unsigned int networkPlayerCount = CGameNetworkManager::sInstance.GetPlayerCount();
        const unsigned int mapSize = Minecraft::GetInstance()->GetMiniGame()->GetMapSizeOverride();
        if ((mapSize == LevelGenerationOptions::eMapSize_Medium || mapSize == LevelGenerationOptions::eMapSize_Large)
            && networkPlayerCount > CENTRAL_SPAWN_MAX_PLAYERS)
        {
            MiniGamePlayerSettings settings;
            MiniGameDef::GetPlayerSettings(settings);
            if (settings.mHostOptions.Get(eCustomGameModeHostOption_CentreSpawn))
            {
                settings.mHostOptions.Set(eCustomGameModeHostOption_CentreSpawn, 0);
                settings.mCentralSpawnDisabled = true;
                MiniGameDef::SetPlayerSettings(settings);
                ServerPlayers()->broadcastAll(Minecraft::GetInstance()->GetMiniGame()->GetPlayerSettingsPacket());
            }
        }
    }
}

void MasterGameMode::OnFollow(boost::shared_ptr<ServerPlayer> player, bool follow, bool previous)
{
    if (!follow)
    {
        if (player->IsSpectatingOtherPlayer())
            player->StopSpectatingPlayer();
    }
    else if (player->SpectatePlayerChange(previous) == NULL)
    {
        player->StopSpectatingPlayer();
    }
    UpdateClientsCameras(player);
}

void MasterGameMode::OnLeave(boost::shared_ptr<ServerPlayer> player)
{
    const unsigned int playerIndex = player->mPlayerIndex;
    for (unsigned int slot = 0; slot < mPlayerSpawnSlots.length; ++slot)
    {
        if (mPlayerSpawnSlots.data[slot] == playerIndex)
            mPlayerSpawnSlots.data[slot] = EMPTY_SPAWN_SLOT;
    }
    for (unsigned int slot = 0; slot < mSpectatorSpawnSlots.length; ++slot)
    {
        if (mSpectatorSpawnSlots.data[slot] == playerIndex)
            mSpectatorSpawnSlots.data[slot] = EMPTY_SPAWN_SLOT;
    }

    if (mPlayersLoading > 0)
        --mPlayersLoading;

    Scoreboard* scoreboard = gMinecraftServer->getLevel(0)->GetScoreboard();
    scoreboard->removePlayerFromTeam(player->mXuid);
    const StatsUID suid = player->getSUID();
    if (mGameStats.GetPlayerStats(suid) != NULL)
        player->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(suid, mGameStats.GetDeathCount(suid));

    mPlayerReady[playerIndex] = false;
    ServerPlayers()->broadcastAll(boost::shared_ptr<Packet>(new PlayerReadyPacket(playerIndex, false)));

    mVoteables->getVoteable(Voteables::eVoteable_Map)->setIncludedVotesAndSync(playerIndex, Voteable::NO_VOTE);
    mVoteables->getVoteable(Voteables::eVoteable_Kick)
        ->setIncludedVotesAndSync(playerIndex, playerIndex, Voteable::NO_VOTE);
    if (Minecraft::InMiniGame(eMiniGameId_Build, true))
    {
        mVoteables->getVoteable(Voteables::eVoteable_BuildOffScoring)
            ->setIncludedVotesAndSync(playerIndex, playerIndex, Voteable::NO_VOTE);
        mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme)
            ->setIncludedVotesAndSync(playerIndex, Voteable::NO_VOTE);
    }

    mGameStats.RemovePlayerStats(player);

    std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> otherPlayer = *playerIt;
        if (otherPlayer != NULL && otherPlayer->GetCamera() == player && otherPlayer != player)
            OnFollow(otherPlayer, true, false);
    }
}

void MasterGameMode::OnDeath(boost::shared_ptr<ServerPlayer> player)
{
    if (GetMiniGame()->GetLivesPerRound() != 0)
    {
        int livesRemaining = GetMiniGame()->GetLivesPerRound() - player->mGameMode->GetDeaths();
        if (livesRemaining < 1)
        {
            player->closeContainer();
            player->setGameMode(GameType::SPECTATOR);
        }
        else
        {
            player->sendMessage(ClientboundChatPacket::eChatPacketMessage_LivesRemaining, &livesRemaining, 1, NULL, 0);
        }
    }

    if (GetMiniGame()->GetId() == eMiniGameId_Glide)
        static_cast<ServerPlayerGameMode*>(player->GetGameMode())->ResetTempPoints();

    BroadcastGameModePacket(GameModePacket::eMessage_DeathPosition,
                            GameModePacket::SoundPositionData(static_cast<float>(player->mX),
                                                              static_cast<float>(player->mY),
                                                              static_cast<float>(player->mZ)));
    player->SetGlideCollisionDamage(false);
    player->SetCamera(boost::shared_ptr<Entity>());
    UpdateClientsCameras(player);
}

void MasterGameMode::StopAchievementExploit(const std::wstring& kickedPlayerName)
{
    int activePlayerCount = 0;
    bool kickedPlayerActive = false;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        const boost::shared_ptr<ServerPlayer>& player = *playerIt;
        if (!player->isAlive() || player->isSpectator())
            continue;

        ++activePlayerCount;
        if (!kickedPlayerActive && kickedPlayerName == player->getNetworkName())
            kickedPlayerActive = true;
    }

    if (kickedPlayerActive && activePlayerCount == ACHIEVEMENT_EXPLOIT_PLAYER_COUNT)
        mGameStats.mAchievementExploitDetected = true;
}

void MasterGameMode::OnKickReqested(int voterIndex, int targetIndex)
{
    const int votes = mVoteables->getVoteable(Voteables::eVoteable_Kick)->getTotalledVotes()[targetIndex];
    INetworkPlayer* voter = getNetworkPlayerFromIndex(voterIndex);
    INetworkPlayer* target = getNetworkPlayerFromIndex(targetIndex);
    const int requiredVotes = std::min(GetPlayerCount(false, false) - 1, MAX_KICK_VOTES_REQUIRED);

    if (voter != NULL && target != NULL)
    {
        boost::shared_ptr<ClientboundChatPacket> packet(
            new ClientboundChatPacket(ClientboundChatPacket::eChatPacketMessage_PlayerVotedToKick, -1));
        packet->strings.push_back(voter->GetDisplayName());
        packet->strings.push_back(target->GetDisplayName());
        packet->ints.push_back(votes);
        packet->ints.push_back(requiredVotes);
        ServerPlayers()->broadcastAll(packet);
    }

    if (votes >= requiredVotes)
    {
        if (target != NULL)
        {
            StopAchievementExploit(target->GetOnlineName());
            gMinecraftServer->getPlayers()->kickPlayerByShortId(target->GetSmallId());
        }
        mVoteables->getVoteable(Voteables::eVoteable_Kick)
            ->setIncludedVotesAndSync(targetIndex, targetIndex, Voteable::NO_VOTE);
    }
}

void MasterGameMode::OnStartSpectating(boost::shared_ptr<ServerPlayer> player)
{
    player->setGameMode(GameType::SPECTATOR);
    if (!Minecraft::InMiniGame(eMiniGameId_Glide, true))
        return;

    boost::shared_ptr<ServerPlayer> trailingPlayer = mGameStats.GetTrailingPlayer(player);
    if (trailingPlayer != NULL)
    {
        player->SetCamera(trailingPlayer);
        UpdateClientsCameras(player);
    }
}

void MasterGameMode::OnServerResume(long long suspendedTime)
{
    if (GetMiniGame()->GetId() != eMiniGameId_Glide && GetMiniGame()->GetId() != eMiniGameId_Lobby)
        return;

    if (!GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial))
        return;

    if (!IsRoundPlaying())
    {
        if (HasTimer(eTimerID_RoundStartDelay))
            RestartTimer(eTimerID_RoundStartDelay,
                         GetTimerRemainingTime(eTimerID_RoundStartDelay) + static_cast<int>(suspendedTime));
    }
    else
    {
        mGameStats.increasePlayerRoundStartTimeMs(suspendedTime);
        mRoundStartTime += suspendedTime;
    }

    GhostController* ghostController = Minecraft::GetInstance()->GetGhostController();
    if (ghostController != NULL)
    {
        ghostController->mPlaybackStartTime += suspendedTime;
        ghostController->mSuspendedTime += suspendedTime;
    }

    Minecraft::GetInstance()->GetClientMasterGameMode()->SetLocalRoundTimeStartOverride(mGameStats.GetStartTime());
    if (HasTimer(eTimerID_LobbyInterval))
        RestartTimer(eTimerID_LobbyInterval,
                     GetTimerRemainingTime(eTimerID_LobbyInterval) + static_cast<int>(suspendedTime));
    if (HasTimer(eTimerID_RoundEndDelay))
        RestartTimer(eTimerID_RoundEndDelay,
                     GetTimerRemainingTime(eTimerID_RoundEndDelay) + static_cast<int>(suspendedTime));

    const std::vector<boost::shared_ptr<ServerPlayer> > roundPlayers = GetPlayers(true, false);
    std::vector<boost::shared_ptr<Player> > players(roundPlayers.begin(), roundPlayers.end());
    CheckTargets(players);
    CheckCheckpoints(players);
}

void MasterGameMode::PlayerFinishedLoadingLocally()
{
    --mPlayersLoading;
}

void MasterGameMode::OnLevelLoad(ServerLevel* level)
{
    if (level->mDimension->getType() != DimensionType::OVERWORLD)
        return;

    delete mBlockChangeListener;
    mBlockChangeListener = new BlockChangeListener(level);
    level->addListener(mBlockChangeListener);
}

IChunkHistory* MasterGameMode::GetChunkHistory() const
{
    return mBlockChangeListener;
}

int MasterGameMode::GetRoundTime()
{
    return static_cast<unsigned int>(floorf(static_cast<float>(GetCurrentTimeMillis() - mRoundStartTime) / 1000.0f));
}

int MasterGameMode::GetPostGameTime()
{
    if (mPostRoundStartTime == 0)
        return 0;
    return static_cast<unsigned int>(GetCurrentTimeMillis() - mPostRoundStartTime) / 1000;
}

int MasterGameMode::GetMaxKills()
{
    unsigned int maxKills = 0;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
    for (unsigned int index = 0; index < players->size(); ++index)
        maxKills = std::max(maxKills, players->at(index)->mGameMode->GetKills());
    return maxKills;
}

int MasterGameMode::GetActiveTeamCount()
{
    return static_cast<int>(GetActiveTeams().size());
}

Vec3* MasterGameMode::GetLastKnownPosition(boost::shared_ptr<Player> player)
{
    return mLastKnownPositions[player->mPlayerIndex];
}

AABB* MasterGameMode::GetSweptVolumeFromLastKnownPosition(boost::shared_ptr<Player> player)
{
    Vec3* lastPosition = GetLastKnownPosition(player);
    if (lastPosition == NULL)
        return player->getSweptVolume();

    Vec3* position = player->getPos(1.0f);
    return AABB::newTemp(std::min(lastPosition->x, position->x), std::min(lastPosition->y, position->y),
                         std::min(lastPosition->z, position->z), std::max(lastPosition->x, position->x),
                         std::max(lastPosition->y, position->y), std::max(lastPosition->z, position->z));
}

unsigned int MasterGameMode::GetLastCheckpointID(const StatsUID& uid)
{
    return getScoreboard()->GetLastCheckpointID(uid, -1);
}

void MasterGameMode::CatchUpCheckpointCheck(boost::shared_ptr<ServerPlayer> player, AABB* sweptVolume)
{
    unsigned int checkpointIndex = 0;
    for (std::vector<CheckpointRuleDefinition*>::iterator checkpointIt = mCheckpoints.begin();
         checkpointIt != mCheckpoints.end(); ++checkpointIt, ++checkpointIndex)
    {
        AABB* area = (*checkpointIt)->getArea();
        if (area->intersects(sweptVolume->mMinX, sweptVolume->mMinY, sweptVolume->mMinZ, sweptVolume->mMaxX,
                             sweptVolume->mMaxY, sweptVolume->mMaxZ))
        {
            OnCheckpointEnter(player, checkpointIndex, *checkpointIt);
        }
    }
}

void MasterGameMode::KillTimers()
{
    KillTimer(eTimerID_GameInitialise);
    KillTimer(eTimerID_RoundPreparing);
    KillTimer(eTimerID_RoundStartDelay);
    KillTimer(eTimerID_GracePeriod);
    KillTimer(eTimerID_RoundEndDelay);
    KillTimer(eTimerID_ChestRefill);
    KillTimer(eTimerID_LobbyInterval);
    KillTimer(eTimerID_RoundLimit);
    KillTimer(eTimerID_Showdown);
    KillTimer(eTimerID_PostGame);
    KillTimer(eTimerID_TimeSincePlayers);
    KillTimer(eTimerID_TimeToReachEnd);
}

void MasterGameMode::PositionPlayersInSpectatorLobby()
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset == NULL)
        return;

    AABB* spectatorSpawn = gApp.getLevelGenerationOptions()->getRequiredGameRules()->getNamedArea(L"SpectatorSpawn");
    std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
         playerIt != players->end(); ++playerIt)
    {
        boost::shared_ptr<ServerPlayer> player = *playerIt;
        if (spectatorSpawn != NULL)
        {
            GameModeUtilities::SpawnWithinArea(player, spectatorSpawn);
        }
        else
        {
            ruleset->postProcessPlayer(player);
            player->mConnection->teleport(player->mX, player->mY, player->mZ, player->mYRot, player->mXRot, true, true);
        }
    }
}

void MasterGameMode::ResetPlayer(boost::shared_ptr<ServerPlayer> player)
{
    std::vector<boost::shared_ptr<ServerPlayer> > players;
    players.push_back(player);
    PreparePlayersForGamemode(&players);
}

void MasterGameMode::ReduceShowdownTime(int reduction, int minimumRemaining)
{
    bool reduced = false;
    if (HasTimer(eTimerID_TimeToReachEnd))
    {
        const int remaining = GetTimerRemainingTime(eTimerID_TimeToReachEnd);
        if (remaining > minimumRemaining)
        {
            if (remaining - reduction < minimumRemaining)
            {
                reduction = remaining - minimumRemaining;
                RestartTimer(eTimerID_TimeToReachEnd, minimumRemaining);
            }
            else
            {
                RestartTimer(eTimerID_TimeToReachEnd, remaining - reduction);
            }
            reduced = true;
        }
    }

    if (HasTimer(eTimerID_Showdown))
    {
        const int remaining = GetTimerRemainingTime(eTimerID_Showdown);
        if (remaining > minimumRemaining)
        {
            if (remaining - reduction < minimumRemaining)
            {
                reduction = remaining - minimumRemaining;
                RestartTimer(eTimerID_Showdown, minimumRemaining);
            }
            else
            {
                RestartTimer(eTimerID_Showdown, remaining - reduction);
            }
            reduced = true;
        }
    }

    if (mShowdownStartTime != 0 && reduced)
        mShowdownStartTime -= reduction;
}

void MasterGameMode::UpdateRulePlayList(std::vector<LevelRules::LevelRulesetGroup*>* playlist)
{
    mPlaylist.clear();
    mPlaylist.insert(mPlaylist.end(), playlist->begin(), playlist->end());
    std::sort(mPlaylist.begin(), mPlaylist.end(), LevelRules::LevelRulesetGroup::SortMapRules);
    mPlayedRuleGroups.clear();
    mRemainingRuleGroups.clear();
    mRemainingRuleGroups.insert(mRemainingRuleGroups.end(), mPlaylist.begin(), mPlaylist.end());
}

bool MasterGameMode::IsPlayerReady(unsigned int playerIndex)
{
    return mPlayerReady[playerIndex];
}

void MasterGameMode::SetPlayerReady(unsigned int playerIndex, bool ready)
{
    const EInternalGameModeState state = GetState();
    if (state != eInternalGameModeState_Lobby && state != eInternalGameModeState_LobbyCountdown)
        return;

    mPlayerReady[playerIndex] = ready;
    ServerPlayers()->broadcastAll(boost::shared_ptr<Packet>(new PlayerReadyPacket(playerIndex, ready)));
    if (mNextLevelGenOptions != NULL)
        mNextLevelGenOptions->getRequiredGameRules()->getRuleType();
}

bool MasterGameMode::IsChunkWithinLobbyRegionOfRoundLevel(int chunkX, int chunkZ)
{
    return gApp.getLevelGenerationOptions() != mLobbyLevelGenOptions && chunkX >= mLobbyMinX && chunkX <= mLobbyMaxX
           && chunkZ >= mLobbyMinZ && chunkZ <= mLobbyMaxZ;
}

void MasterGameMode::GetMaxActiveBounds(int& minX, int& minZ, int& maxX, int& maxZ)
{
    if (mActiveChunkAreas.empty())
        return;

    std::vector<AABB*>::iterator areaIt = mActiveChunkAreas.begin();
    minX = static_cast<int>((*areaIt)->mMinX);
    minZ = static_cast<int>((*areaIt)->mMinZ);
    maxX = static_cast<int>((*areaIt)->mMaxX);
    maxZ = static_cast<int>((*areaIt)->mMaxZ);
    for (++areaIt; areaIt != mActiveChunkAreas.end(); ++areaIt)
    {
        minX = std::min(minX, static_cast<int>((*areaIt)->mMinX));
        minZ = std::min(minZ, static_cast<int>((*areaIt)->mMinZ));
        maxX = std::max(maxX, static_cast<int>((*areaIt)->mMaxX));
        maxZ = std::max(maxZ, static_cast<int>((*areaIt)->mMaxZ));
    }
}

void MasterGameMode::BlockDestroyedAt(const BlockPos& pos)
{
    if (mMapGenerator == NULL || GetState() != eInternalGameModeState_RoundPlaying)
        return;

    AABB* layerBounds = mMapGenerator->GetLayerBB(mSpawnLayerIndex);
    if (layerBounds == NULL || !layerBounds->contains(pos))
        return;

    const int blocksDestroyed = mLayerBlocksDestroyed++;
    if (static_cast<int>(mMapGenerator->GetBlockCountOnLayer(mSpawnLayerIndex))
        < blocksDestroyed + SPAWN_LAYER_REMAINING_BLOCKS)
    {
        ++mSpawnLayerIndex;
        mLayerBlocksDestroyed = 0;
        const unsigned int layerCount = mMapGenerator->GetNumberOfLayers();
        if (layerCount <= static_cast<unsigned int>(mSpawnLayerIndex))
            mSpawnLayerIndex = layerCount - 1;
    }
}

void MasterGameMode::ResetSpawnLayers()
{
    mLayerBlocksDestroyed = 0;
    mSpawnLayerIndex = 0;
}

void MasterGameMode::RegenTumbleLayers()
{
    if (mMapGenerator == NULL)
        return;

    CleanUpMapGenerator();
    UpdateGeneratorSeed();
    RestartMapGenerator();
}

long long MasterGameMode::RestartMapGenerator()
{
    ResetSpawnLayers();
    long long seed = CommonMasterGameMode::RestartMapGenerator();
    BroadcastGameModePacket(GameModePacket::eMessage_UpdateSeed,
                            GameModePacket::UpdateSeedGameModeData(seed, mGeneratorSeedScale));
    return seed;
}

EGameModeState MasterGameMode::GetGameModeState() const
{
    return sStateMap[mState];
}

Level* MasterGameMode::GeneratorTargetLevel() const
{
    return gMinecraftServer->getLevel(0);
}

void MasterGameMode::setChosenThemeWordId(int themeWordId)
{
    mChosenThemeWordId = themeWordId;
    GameModePacket::UpdateThemeGameModeData data(themeWordId);
    BroadcastGameModePacket(GameModePacket::eMessage_UpdateTheme, data);
}

bool MasterGameMode::IsRoundPlaying() const
{
    return mState == eInternalGameModeState_RoundPlaying;
}

std::vector<AABB*>* MasterGameMode::getActiveViewAreas()
{
    return &mActiveViewAreas;
}

Scoreboard* MasterGameMode::getScoreboard()
{
    return gMinecraftServer->getLevel(0)->GetScoreboard();
}

void MasterGameMode::OnCheckpointEnter(const boost::shared_ptr<Player>& player, unsigned int checkpointIndex,
                                       CheckpointRuleDefinition* checkpoint)
{
    CommonMasterGameMode::OnCheckpointEnter(player, checkpointIndex, checkpoint);
    mLastKnownPositionPlayers.push_back(player->getSUID());
}

void MasterGameMode::PlayReachedEndSFX(const std::vector<boost::shared_ptr<ServerPlayer> >& players,
                                       boost::shared_ptr<ServerPlayer> player)
{
    if (!GetMiniGame()->HasFeatureFinishLineSoundEffect())
        return;

    unsigned int finishedCount = 0;
    std::vector<boost::shared_ptr<ServerPlayer> >* serverPlayers = gMinecraftServer->getPlayers()->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = serverPlayers->begin();
         playerIt != serverPlayers->end(); ++playerIt)
    {
        PlayerStats* stats = mGameStats.GetPlayerStats((*playerIt)->getSUID());
        if (stats != NULL && stats->getReachedEnd(mGameStats.mCurrentRound))
            ++finishedCount;
    }

    const SoundEvent* sound = SoundEvents::MG03_FINISH_LINE;
    if (finishedCount == 1)
        sound = SoundEvents::MG03_FINISH_LINE_1ST;
    else if (finishedCount == 2)
        sound = SoundEvents::MG03_FINISH_LINE_2ND;
    else if (finishedCount == 3)
        sound = SoundEvents::MG03_FINISH_LINE_3RD;

    PlayerStats* playerStats = mGameStats.GetPlayerStats(player->getSUID());
    if (GetMiniGame()->GetGameVariant() == MiniGameDef::eGlideVariant_TimeAttack)
    {
        const bool fastestTime = playerStats->getLifeSpan(mGameStats.mCurrentRound)
                                 <= mGameStats.GetFastestPlayerTime();
        if (fastestTime && finishedCount == 1)
            sound = SoundEvents::MG03_FINISH_LINE_1ST_BEST_TIME;
        else if (fastestTime && finishedCount == 2)
            sound = SoundEvents::MG03_FINISH_LINE_2ND_BEST_TIME;
        else if (fastestTime && finishedCount == 3)
            sound = SoundEvents::MG03_FINISH_LINE_3RD_BEST_TIME;
    }
    else
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(mGameStats.mCurrentRound);
        const unsigned int score = roundStats->mPoints + roundStats->mUnbankedPoints;
        const bool bestScore = score >= mGameStats.GetHighestScoringPlayerScore();
        if (bestScore && finishedCount == 1)
            sound = SoundEvents::MG03_FINISH_LINE_1ST_BEST_SCORE;
        else if (bestScore && finishedCount == 2)
            sound = SoundEvents::MG03_FINISH_LINE_2ND_BEST_SCORE;
        else if (bestScore && finishedCount == 3)
            sound = SoundEvents::MG03_FINISH_LINE_3RD_BEST_SCORE;
    }

    boost::shared_ptr<Packet> packet(new ClientboundSoundPacket(
        ClientboundSoundPacket::eSoundInstances_FollowLocalPlayer, sound, 1.0f, 1.0f, false, player->getId()));
    ServerPlayers()->broadcast(player->mX, player->mY, player->mZ, FINISH_LINE_SOUND_RANGE, player->mDimension, packet);
}

void MasterGameMode::OnPrimaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* target)
{
    std::vector<boost::shared_ptr<ServerPlayer> > players = GetPlayers(true, false);
    boost::shared_ptr<ServerPlayer> serverPlayer = boost::static_pointer_cast<ServerPlayer>(player);
    if (mGameStats.RecordEndZoneEntered(serverPlayer->getSUID()))
    {
        ReduceShowdownTime(END_ZONE_SHOWDOWN_REDUCTION, END_ZONE_SHOWDOWN_MINIMUM);
        mLastKnownPositionPlayers.push_back(player->getSUID());
        PlayReachedEndSFX(players, serverPlayer);
    }
}

void MasterGameMode::OnSecondaryTargetEnter(const boost::shared_ptr<Player>& player, TargetAreaRuleDefinition* target)
{
    ServerPlayerGameMode* playerGameMode = static_cast<ServerPlayerGameMode*>(player->GetGameMode());
    if (playerGameMode == NULL)
        return;

    if (playerGameMode->RecordSecondaryTargetReached(
            target->getName(), target->getPointValue(),
            static_cast<GlideRingGenerator::eGlideRingSize>(target->getSize())))
    {
        mLastKnownPositionPlayers.push_back(player->getSUID());
    }
}

void MasterGameMode::OnPowerupEnter(const boost::shared_ptr<Player>& player, unsigned int powerupIndex,
                                    PowerupRuleDefinition* powerup)
{
    CommonMasterGameMode::OnPowerupEnter(player, powerupIndex, powerup);
    mLastKnownPositionPlayers.push_back(player->getSUID());
}

const std::vector<int>& MasterGameMode::GetOptionsList(EMiniGameId miniGame, eCustomGameModeHostOption option)
{
    static std::vector<int> sNoOptions;
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetOptionsList(option);
}

unsigned int MasterGameMode::GetOptionMin(EMiniGameId miniGame, eCustomGameModeHostOption option, bool includeRandom)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetOptionMin(option, includeRandom);
}

int MasterGameMode::GetOptionMax(EMiniGameId miniGame, eCustomGameModeHostOption option)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetOptionMax(option);
}

int MasterGameMode::GetOptionsCount(EMiniGameId miniGame, eCustomGameModeHostOption option)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetOptionsCount(option);
}

std::wstring MasterGameMode::GetValueText(EMiniGameId miniGame, eCustomGameModeHostOption option, int value)
{
    return MiniGameDef::GetCustomGameModeById(miniGame, true)->GetValueText(option, value);
}

void MasterGameMode::StaticCtor()
{
    sStateMap[eInternalGameModeState_Initialising] = eGameModeState_LobbyCountdown;
    sStateMap[eInternalGameModeState_LobbyCountdown] = eGameModeState_LobbyCountdown;
    sStateMap[eInternalGameModeState_Lobby] = eGameModeState_Lobby;
    sStateMap[eInternalGameModeState_SelectingGameRules] = eGameModeState_SelectingGameRules;
    sStateMap[eInternalGameModeState_LoadingGameRules] = eGameModeState_SelectingGameRules;
    sStateMap[eInternalGameModeState_LoadingRound] = eGameModeState_LoadingRound;
    sStateMap[eInternalGameModeState_LoadingSubRound] = eGameModeState_LoadingSubRound;
    sStateMap[eInternalGameModeState_PreparingRound] = eGameModeState_LoadingRound;
    sStateMap[eInternalGameModeState_WaitingToStart] = eGameModeState_WaitingToStart;
    sStateMap[eInternalGameModeState_RoundStarting] = eGameModeState_RoundStarting;
    sStateMap[eInternalGameModeState_RoundPlaying] = eGameModeState_RoundPlaying;
    sStateMap[eInternalGameModeState_PostRound] = eGameModeState_PostRound;
    sStateMap[eInternalGameModeState_RoundEnding] = eGameModeState_RoundEnding;
    sStateMap[eInternalGameModeState_ReturningToLobby] = eGameModeState_RoundEnding;
    sStateMap[eInternalGameModeState_NextSubRound] = eGameModeState_LoadingSubRound;
}

void MasterGameMode::OnGameChange(MasterGameMode* gameMode, void* context)
{
    BroadcastChatMessage(ClientboundChatPacket::eChatPacketMessage_RoundChanging);
    gameMode->SetState(eInternalGameModeState_SelectingGameRules);
}

void MasterGameMode::OnTimerCountdown(ETimerID timer, MasterGameMode* gameMode, void* context)
{
    gameMode->TimerCountdown(timer, static_cast<CountdownInfo*>(context));
}

void MasterGameMode::OnWaitingToStartCountdown(ETimerID timer, MasterGameMode* gameMode, void* context)
{
    GameCondition* startConditions = Minecraft::GetInstance()->GetMiniGame()->GetStartConditions();
    if (startConditions != NULL)
        startConditions->UpdateMessage(gameMode, IDS_MG_START);
}

void MasterGameMode::OnRefillChestTimer(MasterGameMode* gameMode, void* context)
{
    gameMode->sendTutorialUpdate(eTutorial_GameMode_ChestRefill);

    const MiniGameDef* miniGame = GetMiniGame();
    if (!miniGame->IsChestRefillEnabled())
        return;

    std::vector<ChestRefillInfo>& chestRefills = gameMode->mChestRefills;
    ServerLevel* level = gMinecraftServer->getLevel(0);
    const int centralChance = miniGame->getCentralRefillChance();
    const int outerChance = miniGame->getOuterRefillChance();
    const int hvChance = miniGame->getHVRefillChance();
    const int chestsToRefill = miniGame->getNumberOfChestsToRefill();
    const int minimumDistance = miniGame->getMinPlayerDistanceToEffectChance();
    const int maximumDistance = miniGame->getMaxPlayerDistanceToEffectChance();
    if (level == NULL || chestRefills.size() == 0)
        return;

    std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
    for (unsigned int playerIndex = 0; playerIndex < players->size(); ++playerIndex)
    {
        boost::shared_ptr<ServerPlayer> player = (*players)[playerIndex];
        if (player->isSpectator())
            continue;

        for (int chestIndex = 0; chestIndex < chestRefills.size(); ++chestIndex)
        {
            const BlockPos pos = chestRefills[chestIndex].pos;
            const float distance = static_cast<float>(
                Vec3::newTemp(player->mX - pos.x, player->mY - pos.y, player->mZ - pos.z)->length());
            if (chestRefills[chestIndex].distance == UNMEASURED_CHEST_DISTANCE
                || distance < chestRefills[chestIndex].distance)
            {
                const float cappedDistance = distance - maximumDistance >= 0.0 ? maximumDistance : distance;
                float chance = cappedDistance - minimumDistance >= 0.0 ? cappedDistance : minimumDistance;
                chestRefills[chestIndex].distance = distance;

                const std::wstring itemSet = chestRefills[chestIndex].itemSet;
                if (equalsIgnoreCase(itemSet, L"StartItems"))
                    chance *= centralChance;
                else if (equalsIgnoreCase(itemSet, L"HVItems"))
                    chance *= hvChance;
                else if (equalsIgnoreCase(itemSet, L"OuterItems"))
                    chance *= outerChance;
                chestRefills[chestIndex].chance = chance;
            }
        }
    }

    float totalChance = 0.0f;
    for (unsigned int chestIndex = 0; chestIndex < chestRefills.size(); ++chestIndex)
        totalChance += chestRefills[chestIndex].chance;

    Random random;
    std::vector<BlockPos> refilledPositions;
    std::vector<boost::shared_ptr<Container> > refilledContainers;
    std::vector<std::wstring> refilledLootGroups;
    int refilledCount = 0;
    for (int attempt = 0; attempt < chestsToRefill * CHEST_REFILL_ATTEMPTS_PER_CHEST && refilledCount < chestsToRefill;
         ++attempt)
    {
        float roll = random.nextFloat() * totalChance;
        int chosenIndex = -1;
        for (unsigned int chestIndex = 0; chestIndex < chestRefills.size(); ++chestIndex)
        {
            roll -= chestRefills[chestIndex].chance;
            if (roll <= 0.0f)
            {
                chosenIndex = chestIndex;
                break;
            }
        }
        if (chosenIndex < 0)
            return;

        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(chestRefills[chosenIndex].pos);
        if (blockEntity == NULL)
            return;

        boost::shared_ptr<ChestBlockEntity> chest = boost::dynamic_pointer_cast<ChestBlockEntity>(blockEntity);
        if (chest == NULL || chest->isFull())
            continue;

        ++refilledCount;
        refilledPositions.push_back(chestRefills[chosenIndex].pos);
        refilledContainers.push_back(boost::dynamic_pointer_cast<Container>(chest));
        refilledLootGroups.push_back(chestRefills[chosenIndex].lootGroup);
    }

    for (int index = 0; index < refilledContainers.size(); ++index)
    {
        std::vector<boost::shared_ptr<Container> > containers;
        containers.push_back(refilledContainers[index]);
        const std::wstring lootGroup = refilledLootGroups[index];
        miniGame->PopulateContainers(&containers, lootGroup);
    }

    if (refilledPositions.size() != 0)
        BroadcastGameModePacket(GameModePacket::eMessage_ChestRefill, GameModePacket::ChestRefill(refilledPositions));
}

void MasterGameMode::OnGracePeriodEnd(MasterGameMode* gameMode, void* context)
{
    {
        PlayerList* players = ServerPlayers();
        boost::shared_ptr<Packet> packet(
            new ClientboundChatPacket(ClientboundChatPacket::eChatPacketMessage_InvulnerabilityOff, -1));
        players->broadcastAll(packet);
    }
    gameMode->SetPlayersInvulnerable(false);
}

void MasterGameMode::OnGameStart(MasterGameMode* gameMode, void* context)
{
    const MiniGameDef* miniGame = GetMiniGame();
    LevelRuleset* rules = gApp.getLevelGenerationOptions()->getRequiredGameRules();
    {
        PlayerList* players = ServerPlayers();
        boost::shared_ptr<Packet> packet(
            new ClientboundChatPacket(ClientboundChatPacket::eChatPacketMessage_RoundStart, -1));
        players->broadcastAll(packet);
    }
    gameMode->PopulateChests();

    if (GetMiniGame()->GetId() == eMiniGameId_Glide)
    {
        ServerLevel* level = gMinecraftServer->getLevel(0);
        if (miniGame->ArePowerupsActive())
        {
            std::vector<PowerupRuleDefinition*> powerups;
            rules->getPowerups(&powerups);
            for (std::vector<PowerupRuleDefinition*>::iterator powerupIt = powerups.begin();
                 powerupIt != powerups.end(); ++powerupIt)
            {
                std::vector<boost::shared_ptr<Entity> >* crystals = level->getEntitiesOfClass(typeid(EnderCrystal),
                                                                                              (*powerupIt)->getArea());
                if (crystals->empty())
                {
                    boost::shared_ptr<EnderCrystal> crystal(new EnderCrystal(
                        level, (*powerupIt)->getArea()->getCenter()->x, (*powerupIt)->getArea()->getCenter()->y,
                        (*powerupIt)->getArea()->getCenter()->z));
                    crystal->setShowBottom(false);
                    level->addEntity(crystal);
                }
                delete crystals;
            }
        }

        std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
             playerIt != players->end(); ++playerIt)
        {
            boost::shared_ptr<ServerPlayer> player = *playerIt;
            not_null_ptr<ItemInstance> chestItem = player->getItemSlot(EquipmentSlot::CHEST);
            if (chestItem->getItem() == Items::elytra && ElytraItem::isFlyEnabled(chestItem))
            {
                AABB* jumpOffPoint = rules->getNamedArea(L"JumpOffPoint");
                if (jumpOffPoint == NULL || !jumpOffPoint->containsIncludingLowerBound(player->getPos(1.0f)))
                    player->startFallFlying();
            }
        }

        gameMode->setCheckpoints(*rules->getCheckpoints());
        for (std::vector<CheckpointRuleDefinition*>::iterator checkpointIt = gameMode->mCheckpoints.begin();
             checkpointIt != gameMode->mCheckpoints.end(); ++checkpointIt)
        {
            (*checkpointIt)->processBeacons(level);
        }

        gameMode->mPrimaryTargetAreas.clear();
        std::vector<TargetAreaRuleDefinition*> targetAreas;
        rules->getTargetAreas(&targetAreas);
        for (std::vector<TargetAreaRuleDefinition*>::iterator targetIt = targetAreas.begin();
             targetIt != targetAreas.end(); ++targetIt)
        {
            if ((*targetIt)->mPrimary)
                gameMode->mPrimaryTargetAreas.push_back((*targetIt)->getArea());
        }
    }

    gameMode->SetLockPlayerPositions(false);

    {
        std::vector<NamedAreaRuleDefinition*> boundsVolumes;
        rules->getPlayerBoundsVolumes(&boundsVolumes);
        std::vector<boost::shared_ptr<ServerPlayer> >* players = ServerPlayers()->getPlayers();
        for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator playerIt = players->begin();
             playerIt != players->end(); ++playerIt)
        {
            (*playerIt)->mConnection->send(
                GameModePacket::Create(GameModePacket::eMessage_ClearAreaBounds, GameModePacket::EmptyData()));

            Team* team = (*playerIt)->getTeam();
            if (miniGame->GetId() == eMiniGameId_Build && team != NULL)
            {
                AABB* teamArea = gameMode->GetTeamArea(team);
                if (teamArea != NULL)
                {
                    AABB* bounds = teamArea->grow(0.0, TEAM_AREA_BOUNDS_HEIGHT_MARGIN, 0.0);
                    GameModePacket::AddAreaBoundsData boundsData(
                        static_cast<int>(bounds->mMinX), static_cast<int>(bounds->mMinY),
                        static_cast<int>(bounds->mMinZ), static_cast<int>(bounds->mMaxX),
                        static_cast<int>(bounds->mMaxY), static_cast<int>(bounds->mMaxZ));
                    (*playerIt)->mConnection->send(
                        GameModePacket::Create(GameModePacket::eMessage_AddAreaBounds, boundsData));
                }
            }
            else
            {
                for (std::vector<NamedAreaRuleDefinition*>::iterator volumeIt = boundsVolumes.begin();
                     volumeIt != boundsVolumes.end(); ++volumeIt)
                {
                    AABB* area = (*volumeIt)->getArea();
                    GameModePacket::AddAreaBoundsData boundsData(
                        static_cast<int>(area->mMinX), static_cast<int>(area->mMinY), static_cast<int>(area->mMinZ),
                        static_cast<int>(area->mMaxX), static_cast<int>(area->mMaxY), static_cast<int>(area->mMaxZ));
                    (*playerIt)->mConnection->send(
                        GameModePacket::Create(GameModePacket::eMessage_AddAreaBounds, boundsData));
                }
            }
        }
    }

    gameMode->SetState(eInternalGameModeState_RoundPlaying);
    gameMode->sendTutorialUpdate(eTutorial_GameMode_RoundPlaying);

    if (miniGame->GetId() == eMiniGameId_Battle)
    {
        const int itemRespawnTime = static_cast<int>(miniGame->GetItemRespawnTime());
        if (itemRespawnTime >= 0)
        {
            CountdownInfo* itemRespawn = new CountdownInfo();
            itemRespawn->message = ClientboundChatPacket::eChatPacketMessage_Text;
            itemRespawn->tickCallback = OnRefillChestTimer;
            gameMode->AddTimer(eTimerID_ChestRefill, OnTimerCountdown, itemRespawn, -1, itemRespawnTime * 1000, -1, -1);
        }
    }

    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (ruleset != NULL)
    {
        DegradationSequenceRuleDefinition* degradation = static_cast<DegradationSequenceRuleDefinition*>(
            ruleset->getRuleByType(ConsoleGameRules::eGameRuleType_DegradationSequence));
        if (degradation != NULL)
            gameMode->mBlockDegradeRoutine = new BlockDegradeRoutine(gMinecraftServer->getLevel(0), degradation);
    }

    CountdownInfo* gracePeriod = new CountdownInfo();
    gracePeriod->completeCallback = OnGracePeriodEnd;
    gracePeriod->message = ClientboundChatPacket::eChatPacketMessage_InvulnerabilityCountdown;
    if (!gameMode->StartTimer(eTimerID_GracePeriod, gracePeriod))
    {
        delete gracePeriod;
        gameMode->SetPlayersInvulnerable(false);
    }
    else if (gameMode->GetTimerRemainingTime(eTimerID_GracePeriod) > 0)
    {
        gameMode->sendTutorialUpdate(eTutorial_GameMode_GracePeriod);
    }
    gameMode->mGameStats.OnRoundStart();
}

void MasterGameMode::OnGameEnd(MasterGameMode* gameMode, void* context)
{
    {
        PlayerList* players = ServerPlayers();
        boost::shared_ptr<Packet> packet(
            new ClientboundChatPacket(ClientboundChatPacket::eChatPacketMessage_RoundEnding, -1));
        players->broadcastAll(packet);
    }
    for (std::vector<CheckpointRuleDefinition*>::iterator checkpointIt = gameMode->mCheckpoints.begin();
         checkpointIt != gameMode->mCheckpoints.end(); ++checkpointIt)
    {
        (*checkpointIt)->deactivateBeacons(gMinecraftServer->getLevel(0));
    }
    gameMode->SetState(gameMode->ShouldPlayAnotherRound() ? eInternalGameModeState_NextSubRound
                                                          : eInternalGameModeState_ReturningToLobby);
}

void MasterGameMode::ResetLobbyTimerAfterSuspend()
{
    sLobbyTimerNeedsReset = true;
}

void MasterGameMode::SetDefaults(LaunchMoreOptionsMenuInitData& data)
{
    data.mPvP = true;
    data.mDoDaylightCycle = false;
    data.mDisableSaving = true;
    data.mFireSpreads = false;
    data.mPlayerSettings = MiniGameDef::GetCustomGameModeById(data.mMiniGameId, true)->GetDefaultPlayerSettings();
}

void MasterGameMode::SaveDefaults(const LaunchMoreOptionsMenuInitData& data)
{
    const unsigned int value = data.mPlayerSettings.mHostOptions.Get(eCustomGameModeHostOption_All);
    MiniGameDef::GetCustomGameModeById(data.mMiniGameId, true)->SetHostOption(eCustomGameModeHostOption_All, value);
    GameSettings::SetGameSettings(data.mPad, eCustomGameModeHostOption_All, value);
}
