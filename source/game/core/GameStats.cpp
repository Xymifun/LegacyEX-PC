#include "game/core/GameStats.hpp"

#include <algorithm>
#include <climits>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <map>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CombatEntry.hpp"
#include "game/core/DamageEvent.hpp"
#include "game/core/DamageReceivedStatTracker.hpp"
#include "game/core/DamageDealtStatTracker.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/DeadshotStatTracker.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/KillsWithBowStatTracker.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Scoreboard.hpp"
#include "game/core/System.hpp"
#include "game/core/Team.hpp"
#include "game/core/Voteable.hpp"
#include "game/core/Voteables.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/DistributeItemsRuleDefinition.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionItem.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/minigame/GameMedal.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/MiniGameMedals.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerScoreboard.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/LastPlayerDamageStatTracker.hpp"
#include "game/world/generation/LastPlayerFatalDamageStatTracker.hpp"
#include "game/world/generation/MaxSnowballHitsWithSnowballOnPlayerStatTracker.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerTeam.hpp"

namespace
{
const float MILLISECONDS_PER_SECOND = 1000.0f;
const double MILLISECONDS_PER_SECOND_EXACT = 1000.0;
const int DEATH_TIME_PENALTY_SECONDS = 4;
const int LAST_PLAYER_DAMAGE_GRACE_TICKS = 40;
const double END_ZONE_PROGRESS = 100.0;
const double NO_PROGRESS = -1.0;
const unsigned int SCRATCH_DAMAGE = 100;
const float SCRATCH_DAMAGE_MARGIN = 100.0f;
const unsigned int CUPID_BOW_KILLS = 2;
const unsigned int TENACIOUS_BALL_HITS = 25;
const int SUCK_MY_SNOWBALLS_KILLS = 3;
const unsigned int BACK_FROM_THE_DEAD_WIN_STREAK = 2;
const unsigned int BACK_FROM_THE_DEAD_RIVAL_WINS = 1;
const int MOUNTAIN_TOP_LAYER = 0;
const int WILL_LIKE_NO_OTHER_LAYER = 2;
const unsigned int MEDAL_POOL_CAPACITY = 65;
}

GameStats::GameStats()
{
    mPlayerStats = StatsMap();
    mRoundStartTime = 0;
    mRoundEndTime = 0;
    mRoundWinner = boost::shared_ptr<ServerPlayer>();
    mFarthestCheckpointReached = 0;
    mCurrentRound = 0;
    mLandingRecorded = false;
    mAchievementExploitDetected = false;
    mRoundDeathCount = 0;
}

GameStats::~GameStats()
{
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
        delete it->second;
}

PlayerStats* GameStats::GetPlayerStats(const StatsUID& uid)
{
    StatsMap::iterator found = mPlayerStats.find(uid);
    if (found != mPlayerStats.end())
        return found->second;
    return NULL;
}

boost::shared_ptr<ServerPlayer> GameStats::GetFirstPlace()
{
    return mFirstPlace.lock();
}

unsigned int GameStats::GetShortestLifespan(unsigned int round)
{
    unsigned int shortest = INT_MAX;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        const unsigned int lifeSpan = it->second->getLifeSpan(round);
        if (lifeSpan < shortest)
            shortest = lifeSpan;
    }
    return shortest;
}

GameStats::StatsMap* GameStats::GetPlayerStats()
{
    return &mPlayerStats;
}

unsigned int GameStats::GetDeathCount(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    unsigned int deaths = 0;
    if (stats != NULL)
        deaths = stats->getRoundStats(mCurrentRound)->mDeaths;
    return deaths;
}

void GameStats::RecordBlockPresent(const StatsUID& uid, const BlockPos& pos, int blockId)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    writeBlockInteractionData(*round, round->mBlocksPlaced, mRoundStartTime, pos, blockId);
}

unsigned int GameStats::GetRoundTime()
{
    if (mRoundStartTime > 0)
    {
        if (mRoundEndTime > 0)
            return Math::round(static_cast<float>(mRoundEndTime - mRoundStartTime) / MILLISECONDS_PER_SECOND);
        return Math::round(static_cast<float>(System::processTimeInMilliSecs() - mRoundStartTime)
                           / MILLISECONDS_PER_SECOND);
    }
    return 0;
}

void GameStats::AwardMG01Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    std::vector<boost::shared_ptr<ServerPlayer> > survivors;
    std::vector<boost::shared_ptr<ServerPlayer> > mostKills;
    std::vector<boost::shared_ptr<ServerPlayer> > mostDamageDealt;
    std::vector<boost::shared_ptr<ServerPlayer> > leastDamageTaken;
    float leastTaken = FLT_MAX;
    float mostDealt = 0.0f;
    int highestKills = 0;
    const int livesPerRound = CommonMasterGameMode::GetMiniGame()->GetLivesPerRound();

    mFirstPlace.reset();
    mSecondPlace.reset();
    mThirdPlace.reset();

    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end();
         ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        PlayerStats* stats = GetPlayerStats(player->getSUID());
        if (stats == NULL || !stats->hasRoundStats(mCurrentRound))
            continue;

        int livesRemaining = INT_MAX;
        if (livesPerRound > 0)
            livesRemaining = livesPerRound - stats->getDeaths(mCurrentRound);

        if ((player->isAlive() || livesRemaining > 0) && !player->isSpectator())
        {
            stats->getRoundStats(mCurrentRound)->mParticipated = true;
            survivors.push_back(player);

            if (stats->getRoundStats(mCurrentRound)->mKills >= highestKills)
            {
                if (stats->getRoundStats(mCurrentRound)->mKills > highestKills)
                {
                    highestKills = stats->getRoundStats(mCurrentRound)->mKills;
                    mostKills.clear();
                }
                mostKills.push_back(player);
            }

            const float dealt = stats->mDamageDealt->getValue();
            if (dealt >= mostDealt)
            {
                if (dealt > mostDealt)
                {
                    mostDealt = dealt;
                    mostDamageDealt.clear();
                }
                mostDamageDealt.push_back(player);
            }

            const float taken = stats->mDamageReceived->getValue();
            if (taken <= leastTaken)
            {
                if (taken < leastTaken)
                {
                    leastTaken = taken;
                    leastDamageTaken.clear();
                }
                leastDamageTaken.push_back(player);
            }
        }

        if (stats->getRoundStats(mCurrentRound)->mLifeSpan == 0)
            stats->getRoundStats(mCurrentRound)->mLifeSpan =
                GetRoundTime() - stats->getRoundStats(mCurrentRound)->mDeaths * DEATH_TIME_PENALTY_SECONDS;
    }

    if (survivors.size() == 1)
        mRoundWinner = survivors.front();
    else if (mostKills.size() == 1)
        mRoundWinner = mostKills.front();
    else if (mostDamageDealt.size() == 1)
        mRoundWinner = mostDamageDealt.front();
    else if (leastDamageTaken.size() == 1)
        mRoundWinner = leastDamageTaken.front();

    if (mAchievementExploitDetected)
    {
        mRoundWinner.reset();
        mAchievementExploitDetected = false;
    }

    if (mRoundWinner)
    {
        PlayerStats* winnerStats = GetPlayerStats(mRoundWinner->getSUID());
        if (winnerStats != NULL)
        {
            winnerStats->getRoundStats(mCurrentRound)->mWon = true;
            mFirstPlace = mRoundWinner;
        }
    }
}

bool GameStats::SORT_MG02_STATS(PlayerStats* first, PlayerStats* second)
{
    if (static_cast<unsigned int>(first->getTotalRoundsWon()) > static_cast<unsigned int>(second->getTotalRoundsWon()))
        return true;
    if (static_cast<unsigned int>(first->getTotalRoundsWon()) >= static_cast<unsigned int>(second->getTotalRoundsWon()))
    {
        if (static_cast<unsigned int>(first->getTotalBlocksDestroyed())
            > static_cast<unsigned int>(second->getTotalBlocksDestroyed()))
            return true;
        if (static_cast<unsigned int>(first->getTotalBlocksDestroyed())
            < static_cast<unsigned int>(second->getTotalBlocksDestroyed()))
            return false;
    }
    return false;
}

boost::weak_ptr<ServerPlayer> GameStats::getPlayerFromStatsVec(const std::vector<PlayerStats*>& stats,
                                                               unsigned int index)
{
    if (index < stats.size())
    {
        boost::shared_ptr<ServerPlayer> player = (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)
                                                     ->getPlayerFromStatsUID(stats.at(index)->mStatsUID);
        return player;
    }
    return boost::weak_ptr<ServerPlayer>();
}

void GameStats::AwardMG02Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    std::vector<boost::shared_ptr<ServerPlayer> > survivors;
    std::vector<boost::shared_ptr<ServerPlayer> > fewestDeaths;
    int lowestDeaths = INT_MAX;

    mRoundWinner.reset();
    const int livesPerRound = CommonMasterGameMode::GetMiniGame()->GetLivesPerRound();

    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end();
         ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        PlayerStats* stats = GetPlayerStats(player->getSUID());
        if (stats == NULL)
            continue;

        int livesRemaining = INT_MAX;
        if (livesPerRound > 0)
            livesRemaining = livesPerRound - stats->getDeaths(mCurrentRound);

        if ((player->isAlive() || livesRemaining > 0) && !player->isSpectator())
        {
            survivors.push_back(player);

            if (stats->getRoundStats(mCurrentRound)->mDeaths <= lowestDeaths)
            {
                if (stats->getRoundStats(mCurrentRound)->mDeaths < lowestDeaths)
                {
                    lowestDeaths = stats->getRoundStats(mCurrentRound)->mDeaths;
                    fewestDeaths.clear();
                }
                fewestDeaths.push_back(player);
            }

            if (stats->getRoundStats(mCurrentRound)->mLifeSpan == 0)
                stats->getRoundStats(mCurrentRound)->mLifeSpan =
                    GetRoundTime() - stats->getRoundStats(mCurrentRound)->mDeaths * DEATH_TIME_PENALTY_SECONDS;
        }
    }

    if (survivors.size() == 1)
        mRoundWinner = survivors.front();
    else if (fewestDeaths.size() == 1)
        mRoundWinner = fewestDeaths.front();

    if (mRoundWinner)
    {
        PlayerStats* winnerStats = GetPlayerStats(mRoundWinner->getSUID());
        if (winnerStats != NULL)
        {
            winnerStats->getRoundStats(mCurrentRound)->mParticipated = true;
            if (!mAchievementExploitDetected)
            {
                winnerStats->getRoundStats(mCurrentRound)->mWon = true;
            }
            else
            {
                mRoundWinner.reset();
                mAchievementExploitDetected = false;
            }
        }
    }

    std::vector<PlayerStats*> ranking;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end();
         ++it)
    {
        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats != NULL)
            ranking.push_back(stats);
    }

    std::sort(ranking.begin(), ranking.end(), SORT_MG02_STATS);
    mFirstPlace = getPlayerFromStatsVec(ranking, 0);
    mSecondPlace = getPlayerFromStatsVec(ranking, 1);
    mThirdPlace = getPlayerFromStatsVec(ranking, 2);
}

bool GameStats::SORT_MG03_STATS_FALLBACK(PlayerStats* first, PlayerStats* second)
{
    if (first->getBestLifeSpan(false, false) != second->getBestLifeSpan(false, false))
        return first->getBestLifeSpan(false, false) < second->getBestLifeSpan(false, false);
    if (first->getBestPoints(true, false) != second->getBestPoints(true, false))
        return first->getBestPoints(true, false) > second->getBestPoints(true, false);
    if (first->getTotalRoundsWon() != second->getTotalRoundsWon())
        return static_cast<unsigned int>(first->getTotalRoundsWon())
               > static_cast<unsigned int>(second->getTotalRoundsWon());
    if (first->getTotalDeaths() != second->getTotalDeaths())
        return first->getTotalDeaths() < second->getTotalDeaths();
    return first < second;
}

bool GameStats::SORT_MG03_STATS_POINTS(PlayerStats* first, PlayerStats* second)
{
    if (first->getBestPoints(true, true) != second->getBestPoints(true, true))
        return first->getBestPoints(true, true) > second->getBestPoints(true, true);
    return SORT_MG03_STATS_FALLBACK(first, second);
}

bool GameStats::SORT_MG03_STATS_TIME(PlayerStats* first, PlayerStats* second)
{
    if (first->getBestLifeSpan(false, true) != second->getBestLifeSpan(false, true))
        return first->getBestLifeSpan(false, true) < second->getBestLifeSpan(false, true);
    return SORT_MG03_STATS_FALLBACK(first, second);
}

bool GameStats::SORT_MG03_STATS_POINTS_CURRENT_ROUND(PlayerStats* first, PlayerStats* second)
{
    const unsigned int round = gMinecraftServer->mMasterGameMode->mGameStats.mCurrentRound;
    if (first->getReachedEnd(round) != second->getReachedEnd(round))
        return first->getReachedEnd(round);
    if (first->getPoints(round) != second->getPoints(round))
        return static_cast<unsigned int>(first->getPoints(round)) > static_cast<unsigned int>(second->getPoints(round));
    return SORT_MG03_STATS_FALLBACK(first, second);
}

bool GameStats::SORT_MG03_STATS_TIME_CURRENT_ROUND(PlayerStats* first, PlayerStats* second)
{
    const unsigned int round = gMinecraftServer->mMasterGameMode->mGameStats.mCurrentRound;
    if (first->getReachedEnd(round) != second->getReachedEnd(round))
        return first->getReachedEnd(round);
    if (first->getLifeSpan(round) != second->getLifeSpan(round))
        return first->getLifeSpan(round) < second->getLifeSpan(round);
    return SORT_MG03_STATS_FALLBACK(first, second);
}

void GameStats::AwardMG03Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    mRoundWinner.reset();

    std::vector<PlayerStats*> ranking;
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end();
         ++it)
    {
        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats != NULL)
            ranking.push_back(stats);
    }
    if (ranking.empty())
        return;

    const bool scoreAttack = Minecraft::GetInstance()->GetMiniGame()->GetGameVariant()
                             == MiniGameDef::eGlideVariant_ScoreAttack;
    std::sort(ranking.begin(), ranking.end(), scoreAttack ? SORT_MG03_STATS_POINTS : SORT_MG03_STATS_TIME);
    mFirstPlace = getPlayerFromStatsVec(ranking, 0);
    mSecondPlace = getPlayerFromStatsVec(ranking, 1);
    mThirdPlace = getPlayerFromStatsVec(ranking, 2);
    mRoundWinner = mFirstPlace.lock();
    ranking.front()->getRoundStats(mCurrentRound)->mWon = true;

    const bool roundScoreAttack = Minecraft::GetInstance()->GetMiniGame()->GetGameVariant()
                                  == MiniGameDef::eGlideVariant_ScoreAttack;
    std::sort(ranking.begin(), ranking.end(),
              roundScoreAttack ? SORT_MG03_STATS_POINTS_CURRENT_ROUND : SORT_MG03_STATS_TIME_CURRENT_ROUND);
    for (unsigned int position = 0; position < ranking.size(); ++position)
    {
        ranking[position]->getRoundStats(mCurrentRound)->mFinishPosition = position;
        ranking[position]->getRoundStats(mCurrentRound)->mFinishPlayerCount = ranking.size();
    }
}

void GameStats::AwardMG04Medals(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    mRoundWinner.reset();
    mFirstPlace.reset();
    mSecondPlace.reset();
    mThirdPlace.reset();
}

void GameStats::CalculateStats(const std::vector<boost::shared_ptr<ServerPlayer> >* players, bool newRound)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end();
         ++it)
    {
        PlayerStats* stats = GetPlayerStats((*it)->getSUID());
        if (stats == NULL)
            continue;
        for (int category = 0; category < eTimeSpentCategory_Count; ++category)
            stats->getRoundStats(mCurrentRound)
                ->MedalTimestamp(static_cast<eTimeSpentCategory>(category), false,
                                 System::processTimeInMilliSecs() - mRoundStartTime);
    }

    MasterGameMode* gameMode = gMinecraftServer->mMasterGameMode;
    if (Minecraft::InMiniGame(eMiniGameId_Build, true))
    {
        Voteable* scoring = gameMode->mVoteables->getVoteable(Voteables::eVoteable_BuildOffScoring);
        if (scoring != NULL)
        {
            for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
                 it != players->end(); ++it)
            {
                std::map<int, int> scoresGiven = scoring->getVotesFromPlayer((*it)->mPlayerIndex);
                std::map<int, int> scoresReceived = scoring->getVotesToVoteable((*it)->getTeam()->getTeamIndex());
                PlayerStats::RoundStats* round = GetPlayerStats((*it)->getSUID())->getRoundStats(mCurrentRound);
                for (int score = 0; score < PlayerStats::RoundStats::BUILD_SCORE_COUNT; ++score)
                {
                    round->mScoresGiven[score] = 0;
                    round->mScoresReceived[score] = 0;
                }
                for (std::map<int, int>::const_iterator vote = scoresGiven.begin(); vote != scoresGiven.end(); ++vote)
                    ++round->mScoresGiven[vote->second];
                for (std::map<int, int>::const_iterator vote = scoresReceived.begin(); vote != scoresReceived.end();
                     ++vote)
                    ++round->mScoresReceived[vote->second];
            }
        }
    }

    if (Minecraft::InMiniGame(eMiniGameId_Build, true))
    {
        ServerLevel* level = gMinecraftServer->mLevels.data[0];
        std::vector<PlayerTeam*>* teams = level->GetServerScoreboard()->getPlayerTeams();
        for (std::vector<PlayerTeam*>::iterator teamIt = teams->begin(); teamIt != teams->end(); ++teamIt)
        {
            boost::unordered_set<PlayerUID, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >* members =
                (*teamIt)->getPlayers();
            AABB* area = gameMode->GetTeamArea(*teamIt);
            BlockPos pos(0, 0, 0);
            for (boost::unordered_set<PlayerUID, boost::hash<PlayerUID>, std::equal_to<PlayerUID> >::iterator member =
                     members->begin();
                 member != members->end(); ++member)
            {
                boost::shared_ptr<ServerPlayer> builder = boost::dynamic_pointer_cast<ServerPlayer>(
                    level->getPlayer(*member));
                for (int x = static_cast<int>(area->mMinX); x < static_cast<int>(area->mMaxX); ++x)
                {
                    for (int y = static_cast<int>(area->mMinY); y < static_cast<int>(area->mMaxY); ++y)
                    {
                        for (int z = static_cast<int>(area->mMinZ); z < static_cast<int>(area->mMaxZ); ++z)
                        {
                            pos.x = x;
                            pos.y = y;
                            pos.z = z;
                            const int blockId = level->getBlockId(x, y, z);
                            if (blockId != 0)
                                RecordBlockPresent(builder->getSUID(), pos, blockId);
                        }
                    }
                }
            }
        }
        delete teams;
    }

    if (Minecraft::InMiniGame(eMiniGameId_Build, true))
    {
        Voteable* themes = gameMode->mVoteables->getVoteable(Voteables::eVoteable_BuildOffTheme);
        if (themes != NULL)
        {
            int winningVotes = Voteable::NO_VOTE;
            themes->getWinningVote(winningVotes);
            mWinningThemeChoice = gameMode->getThemeWordEnglish(gameMode->mChosenThemeWordId);
            for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
                 it != players->end(); ++it)
            {
                PlayerStats::RoundStats* round = GetPlayerStats((*it)->getSUID())->getRoundStats(mCurrentRound);
                int themeVote = Voteable::NO_VOTE;
                themes->getOnlyVote((*it)->mPlayerIndex, &themeVote);
                round->mThemeVote = gameMode->getThemeWordEnglish(themeVote);
            }
        }
    }

    if (!newRound)
        mRoundEndTime = System::processTimeInMilliSecs();

    std::vector<const GameMedal*> medalPool;
    medalPool.reserve(MEDAL_POOL_CAPACITY);
    switch (Minecraft::GetInstance()->GetMiniGame()->GetId())
    {
    case eMiniGameId_Battle:
        AwardMG01Medals(players);
        medalPool = gBattleMedals;
        break;
    case eMiniGameId_Tumble:
        AwardMG02Medals(players);
        medalPool = gTumbleMedals;
        break;
    case eMiniGameId_Glide:
        AwardMG03Medals(players);
        medalPool = gGlideMedals;
        break;
    case eMiniGameId_Build:
        AwardMG04Medals(players);
        medalPool = gBuildMedals;
        break;
    default:
        break;
    }

    if (Minecraft::InMiniGame(eMiniGameId_Build, true))
    {
        const int roundSeconds = static_cast<unsigned int>(
            std::floor(static_cast<double>(static_cast<long long>(System::processTimeInMilliSecs() - mRoundStartTime)))
            / MILLISECONDS_PER_SECOND_EXACT);
        for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end();
             ++it)
            GetPlayerStats((*it)->getSUID())->getRoundStats(mCurrentRound)->mLifeSpan = roundSeconds;
    }

    if (mRoundWinner)
    {
        PlayerStats* winnerStats = GetPlayerStats(mRoundWinner->getSUID());
        if (winnerStats != NULL)
        {
            const double winnerY = static_cast<int>(mRoundWinner->mY);
            winnerStats->getRoundStats(mCurrentRound)->mFinalLayer = gameMode->GetLayer(winnerY);
        }
    }

    if (Minecraft::InMiniGame(eMiniGameId_Tumble, true) && mRoundWinner)
    {
        const bool snowballAwards = Minecraft::GetInstance()->GetMiniGame()->HasFeatureSnowballAwards();
        PlayerStats* winnerStats = GetPlayerStats(mRoundWinner->getSUID());
        if (newRound)
        {
            unsigned int winStreak = 0;
            for (int round = mCurrentRound; round >= 0 && winnerStats->getRoundStats(round)->mWon; --round)
                ++winStreak;

            unsigned int bestRivalWins = 0;
            for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin();
                 it != players->end(); ++it)
            {
                PlayerStats* stats = GetPlayerStats((*it)->getSUID());
                if (*it != mRoundWinner && stats != NULL
                    && bestRivalWins < static_cast<unsigned int>(stats->getTotalRoundsWon()))
                    bestRivalWins = stats->getTotalRoundsWon();
            }

            if (winStreak > BACK_FROM_THE_DEAD_WIN_STREAK && bestRivalWins > BACK_FROM_THE_DEAD_RIVAL_WINS)
                mRoundWinner->awardStat(GenericStats::backFromTheDead(), GenericStats::param_backFromTheDead());
        }

        if (snowballAwards)
        {
            PlayerStats::RoundStats* winnerRound = winnerStats->getRoundStats(mCurrentRound);
            if (winnerRound->mFinalLayer == MOUNTAIN_TOP_LAYER)
                mRoundWinner->awardStat(GenericStats::mountainTop(), GenericStats::param_mountainTop());
            else if (winnerRound->mFinalLayer == WILL_LIKE_NO_OTHER_LAYER)
                mRoundWinner->awardStat(GenericStats::willLikeNoOther(), GenericStats::param_willLikeNoOther());

            if (winnerStats->getRoundStats(mCurrentRound)->mSnowballsThrown == 0)
                mRoundWinner->awardStat(GenericStats::equipYourFists(), GenericStats::param_equipYourFists());
        }
    }

    if (newRound)
    {
        for (std::vector<const GameMedal*>::const_iterator medalIt = medalPool.begin(); medalIt != medalPool.end();
             ++medalIt)
        {
            const GameMedal* medal = *medalIt;
            if (!medal->BestOnly())
            {
                for (unsigned int index = 0; index < players->size(); ++index)
                {
                    const boost::shared_ptr<ServerPlayer>& player = players->at(index);
                    PlayerStats* stats = GetPlayerStats(player->getSUID());
                    if (stats != NULL && medal->CanAward(player->getSUID(), this))
                        stats->mMedals.push_back(medal);
                }
                continue;
            }

            const int best = medal->FindBest(players, this);
            if (best < 0)
                continue;
            PlayerStats* bestStats = GetPlayerStats(players->at(best)->getSUID());
            if (bestStats != NULL)
                bestStats->mMedals.push_back(medal);
        }

        Random random;
        for (unsigned int index = 0; index < players->size(); ++index)
        {
            boost::shared_ptr<ServerPlayer> player = players->at(index);
            PlayerStats* stats = GetPlayerStats(player->getSUID());
            if (stats == NULL)
                continue;

            int totalWeight = 0;
            for (std::vector<const GameMedal*>::iterator medalIt = stats->mMedals.begin();
                 medalIt != stats->mMedals.end(); ++medalIt)
                totalWeight += (*medalIt)->GetWeight();
            stats->mMedalWeight = totalWeight;

            const GameMedal* chosen = NULL;
            while (!stats->mMedals.empty() && stats->mAwardedMedal == NULL)
            {
                int weight = 0;
                for (std::vector<const GameMedal*>::iterator medalIt = stats->mMedals.begin();
                     medalIt != stats->mMedals.end(); ++medalIt)
                    weight += (*medalIt)->GetWeight();

                int roll = random.nextInt(weight);
                for (std::vector<const GameMedal*>::iterator medalIt = stats->mMedals.begin();
                     medalIt != stats->mMedals.end(); ++medalIt)
                {
                    roll -= (*medalIt)->GetWeight();
                    if (roll < 0)
                    {
                        chosen = *medalIt;
                        break;
                    }
                }

                std::vector<const GameMedal*>::iterator pooled = std::find(medalPool.begin(), medalPool.end(), chosen);
                if (pooled == medalPool.end())
                {
                    stats->mMedals.erase(std::find(stats->mMedals.begin(), stats->mMedals.end(), chosen));
                    continue;
                }

                stats->mAwardedMedal = chosen;
                if (!(*pooled)->CanMultiAward())
                    medalPool.erase(pooled);
            }
        }
    }

    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end();
         ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        if (GetPlayerStats(player->getSUID()) == NULL)
            continue;
        const unsigned int deathCount = GetDeathCount(player->getSUID());
        player->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(player->getSUID(), deathCount);
    }
}

void GameStats::AddPlayerStats(const boost::shared_ptr<ServerPlayer>& player)
{
    if (mPlayerStats.find(player->getSUID()) == mPlayerStats.end())
    {
        PlayerStats* stats = new PlayerStats(player->getSUID());
        stats->getRoundStats(mCurrentRound);
        stats->mName = player->getDisplayName();
        stats->mStatsUID = player->getSUID();
        PlayerStats*& slot = mPlayerStats[player->getSUID()];
        slot = stats;
    }
}

void GameStats::AddPlayers(const std::vector<boost::shared_ptr<ServerPlayer> >* players)
{
    for (std::vector<boost::shared_ptr<ServerPlayer> >::const_iterator it = players->begin(); it != players->end();
         ++it)
        AddPlayerStats(*it);
}

void GameStats::RemovePlayerStats(const boost::shared_ptr<ServerPlayer>& player)
{
    StatsMap::iterator found = mPlayerStats.find(player->getSUID());
    if (found != mPlayerStats.end())
    {
        delete found->second;
        mPlayerStats.erase(found);
        player->getLevel()->GetServerScoreboard()->OnPlayerStatsRemoved(player->getSUID(), player->getNetworkName());
    }
}

void GameStats::Clear()
{
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
        delete it->second;
    mPlayerStats.clear();
    mRoundStartTime = 0;
    mRoundEndTime = 0;
    mRoundDeathCount = 0;
    mLandingRecorded = false;
    mFarthestCheckpointReached = 0;
    mRoundWinner = boost::shared_ptr<ServerPlayer>();
}

void GameStats::OnRoundStart()
{
    mRoundStartTime = System::processTimeInMilliSecs();
    mRoundEndTime = 0;
    mRoundDeathCount = 0;
    mLandingRecorded = false;
    mFarthestCheckpointReached = 0;

    if (gMinecraftServer->mMasterGameMode->GetLayerCount() <= 1)
    {
        for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
        {
            PlayerStats* stats = it->second;
            if (stats == NULL)
                continue;
            stats->getRoundStats(mCurrentRound)->mDeathLayer = PlayerStats::RoundStats::LAYERS_UNUSED;
            stats->getRoundStats(mCurrentRound)->mLapsCompleted = 0;
        }
    }
}

void GameStats::OnRoundEnd()
{
    ++mCurrentRound;
    mRoundWinner = boost::shared_ptr<ServerPlayer>();

    std::vector<boost::shared_ptr<ServerPlayer> >* players =
        (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        if (GetPlayerStats(player->getSUID()) != NULL)
        {
            const unsigned int deathCount = GetDeathCount(player->getSUID());
            player->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(player->getSUID(), deathCount);
        }
    }
}

void GameStats::OnGameStart()
{
    mCurrentRound = 0;
}

void GameStats::OnGameEnd()
{
    mCurrentRound = 0;
}

unsigned int GameStats::GetDamageTakenThisRound(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
    {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        return static_cast<unsigned int>(round->mDamageReceived->getValue());
    }
    return 0;
}

unsigned int GameStats::GetKillsWithBowThisRound(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        return stats->getRoundStats(mCurrentRound)->mKillsWithBow->getValue();
    return 0;
}

unsigned int GameStats::GetMaxSnowballHitsOnAnyPlayer(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        return stats->getRoundStats(mCurrentRound)->mMaxSnowballHits->getValue();
    return 0;
}

void GameStats::RecordDamage(boost::shared_ptr<Entity> entity, CombatEntry* entry,
                             boost::shared_ptr<ClientboundChatPacket> deathMessage)
{
    const int tick = gMinecraftServer->tickCount;
    const Scoreboard::EDamageSource damageSource = Scoreboard::GetDamageSource(
        entry->getSource()->GetBaseDamageSource());
    boost::shared_ptr<Entity> attacker = entry->getSource()->getEntity();

    Scoreboard::Weapon weapon;
    if (damageSource == Scoreboard::DAMAGE_SOURCE_EXPLOSION)
    {
        weapon = Scoreboard::Weapon(Item::getId(Item::byBlock(Blocks::tnt)));
    }
    else if (entry->getSource()->isMagic())
    {
        weapon = Scoreboard::Weapon(Item::getId(Items::splashPotion),
                                    Scoreboard::GetPotionByDamageSource(damageSource));
    }
    else if (attacker != NULL && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, attacker->GetType()))
    {
        not_null_ptr<ItemInstance> heldItem = boost::static_pointer_cast<LivingEntity>(attacker)->getMainHandItem();
        if (heldItem != NULL)
        {
            Item* item = heldItem->getItem();
            if (item != NULL && dynamic_cast<PotionItem*>(item) != NULL)
                weapon = Scoreboard::Weapon(Item::getId(heldItem->getItem()), PotionUtils::getPotion(heldItem));
            else
                weapon = Scoreboard::Weapon(Item::getId(heldItem->getItem()));
        }
    }

    const bool victimIsServerPlayer = eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, entity->GetType());
    if (victimIsServerPlayer && attacker == NULL
        && (damageSource == Scoreboard::DAMAGE_SOURCE_FALL || damageSource == Scoreboard::DAMAGE_SOURCE_LAVA))
    {
        PlayerStats* victimStats = GetPlayerStats(boost::static_pointer_cast<ServerPlayer>(entity)->getSUID());
        if (victimStats != NULL)
        {
            const DamageEvent* lastPlayerDamage =
                victimStats->getRoundStats(mCurrentRound)->mLastPlayerDamage->getValue();
            if (lastPlayerDamage != NULL && lastPlayerDamage->mAttackerEntity != NULL
                && tick - lastPlayerDamage->mTime < entity->mFallTicks + LAST_PLAYER_DAMAGE_GRACE_TICKS)
                attacker = lastPlayerDamage->mAttackerEntity;
        }
    }

    const float damage = entry->getDamage();
    const bool fatal = entry->getHealthAfterDamage() <= 0.0f;
    float fallDistance = 0.0f;
    int fallCount = -1;
    if (attacker != NULL)
    {
        fallDistance = attacker->mFallDistance;
        fallCount = static_cast<unsigned short>(attacker->mFallCount);
    }

    if (deathMessage != NULL)
    {
        switch (deathMessage->getMessage())
        {
        case ClientboundChatPacket::eChatPacketMessage_DeathThrown:
        case ClientboundChatPacket::eChatPacketMessage_DeathIndirectMagic:
        case ClientboundChatPacket::eChatPacketMessage_DeathThorns:
        case ClientboundChatPacket::eChatPacketMessage_DeathFellDoomedBy:
        case ClientboundChatPacket::eChatPacketMessage_DeathFellDoomedByItem:
        case ClientboundChatPacket::eChatPacketMessage_DeathInFireFighting:
        case ClientboundChatPacket::eChatPacketMessage_DeathOnFireFighting:
        case ClientboundChatPacket::eChatPacketMessage_DeathLavaEscaping:
        case ClientboundChatPacket::eChatPacketMessage_DeathDrownEscaping:
        case ClientboundChatPacket::eChatPacketMessage_DeathCactusEscaping:
        case ClientboundChatPacket::eChatPacketMessage_DeathExplosionBy:
        case ClientboundChatPacket::eChatPacketMessage_DeathMobItem:
        case ClientboundChatPacket::eChatPacketMessage_DeathArrowItem:
        case ClientboundChatPacket::eChatPacketMessage_DeathFireballItem:
        case ClientboundChatPacket::eChatPacketMessage_DeathThrownItem:
        case ClientboundChatPacket::eChatPacketMessage_DeathHotFloorEscaping:
        case ClientboundChatPacket::eChatPacketMessage_DeathTridentItem:
        case ClientboundChatPacket::eChatPacketMessage_DeathIndirectMagicItem:
            if (deathMessage->getFirstInt() == EntityType::SERVER_PLAYER && deathMessage->strings.size() > 1)
                attacker = (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)
                               ->getPlayerByName(deathMessage->strings[1]);
            break;
        default:
            break;
        }
    }

    DamageEvent event;
    event.mVictim = StatsUID::EMPTY_SUID();
    if (Entity::instanceof(entity, EntityType::SERVER_PLAYER))
        event.mVictim = boost::static_pointer_cast<ServerPlayer>(entity)->getSUID();
    event.mAttackerEntity = attacker;
    event.mAttacker = StatsUID::EMPTY_SUID();
    if (Entity::instanceof(attacker, EntityType::SERVER_PLAYER))
        event.mAttacker = boost::static_pointer_cast<ServerPlayer>(attacker)->getSUID();
    event.mWeapon = weapon;
    event.mDamage = damage;
    event.mFatal = fatal;
    event.mDamageSource = damageSource;
    event.mDistance = attacker != NULL ? attacker->distanceTo(entity) : 0.0f;
    event.mInWater = attacker != NULL ? attacker->isUnderLiquid(Material::WATER) : false;
    event.mFallDistance = fallDistance;
    event.mFallCount = fallCount;
    event.mTime = tick;

    if (entity != NULL && eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, entity->GetType()))
    {
        PlayerStats* victimStats = GetPlayerStats(boost::static_pointer_cast<ServerPlayer>(entity)->getSUID());
        if (victimStats != NULL)
            victimStats->updateTrackedStats(mCurrentRound, &event);
    }

    if (attacker != NULL && attacker != entity && eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, attacker->GetType()))
    {
        PlayerStats* attackerStats = GetPlayerStats(boost::static_pointer_cast<ServerPlayer>(attacker)->getSUID());
        if (attackerStats != NULL)
            attackerStats->updateTrackedStats(mCurrentRound, &event);
    }

    if (Minecraft::InMiniGame(eMiniGameId_Battle, true) && event.mFatal)
    {
        boost::shared_ptr<ServerPlayer> killer = boost::dynamic_pointer_cast<ServerPlayer>(attacker);
        if (killer != NULL && killer->isStarving() && killer != boost::dynamic_pointer_cast<ServerPlayer>(entity))
            killer->awardStat(GenericStats::youWontLikeMeWhenImHungry(),
                              GenericStats::param_youWontLikeMeWhenImHungry());
    }

    if (!CGameNetworkManager::sInstance.IsInPublicJoinableGame())
        return;

    boost::shared_ptr<ServerPlayer> victim = boost::dynamic_pointer_cast<ServerPlayer>(entity);
    boost::shared_ptr<ServerPlayer> attackingPlayer = boost::dynamic_pointer_cast<ServerPlayer>(attacker);
    if (victim != NULL)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Battle, true))
        {
            const unsigned int damageTaken = GetDamageTakenThisRound(victim->getSUID());
            if (damageTaken >= SCRATCH_DAMAGE && static_cast<float>(damageTaken) - damage < SCRATCH_DAMAGE_MARGIN)
                victim->awardStat(GenericStats::tisButAScratch(), GenericStats::param_tisButAScratch());
        }

        PlayerStats* victimStats = GetPlayerStats(victim->getSUID());
        if (victimStats != NULL && victimStats->mDeadshot->getValue())
            victim->awardStat(GenericStats::deadshot(), GenericStats::param_deadshot());
    }

    if (attackingPlayer != NULL)
    {
        const unsigned int bowKills = GetKillsWithBowThisRound(attackingPlayer->getSUID());
        if (Minecraft::InMiniGame(eMiniGameId_Battle, true) && bowKills == CUPID_BOW_KILLS)
            attackingPlayer->awardStat(GenericStats::cupid(), GenericStats::param_cupid());

        if (Minecraft::InMiniGame(eMiniGameId_Tumble, true)
            && GetMaxSnowballHitsOnAnyPlayer(attackingPlayer->getSUID()) == TENACIOUS_BALL_HITS)
            attackingPlayer->awardStat(GenericStats::tenaciousBall(),
                                       GenericStats::param_tenaciousBall(TENACIOUS_BALL_HITS));
    }
}

const DamageEvent* GameStats::GetPlayerFatalDamage(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        return stats->getRoundStats(mCurrentRound)->mLastPlayerFatalDamage->getValue();
    return NULL;
}

void GameStats::RecordDeath(boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(entity);
    const unsigned int deathCount = GetDeathCount(player->getSUID());
    ++mRoundDeathCount;

    const bool isServerPlayer = eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, entity->GetType());
    if (!isServerPlayer)
        return;

    PlayerStats* stats = GetPlayerStats(player->getSUID());
    if (stats != NULL)
    {
        PlayerStats::RoundStats::PerLifeStats* lifeStats =
            stats->getRoundStats(mCurrentRound)->getLifeStats(deathCount);
        const DamageEvent* fatalDamage = GetPlayerFatalDamage(player->getSUID());
        MasterGameMode* gameMode = gMinecraftServer->mMasterGameMode;
        const unsigned int livesPerRound = CommonMasterGameMode::GetMiniGame()->GetLivesPerRound();

        if (CommonMasterGameMode::GetMiniGame()->GetId() != eMiniGameId_Glide
            && CommonMasterGameMode::GetMiniGame()->GetLivesPerRound() == mRoundDeathCount)
            stats->getRoundStats(mCurrentRound)->mLifeSpan =
                static_cast<unsigned int>(std::floor(static_cast<double>(static_cast<long long>(
                                              System::processTimeInMilliSecs() - mRoundStartTime)))
                                          / MILLISECONDS_PER_SECOND_EXACT)
                - mRoundDeathCount * DEATH_TIME_PENALTY_SECONDS;

        ++stats->getRoundStats(mCurrentRound)->mDeaths;

        if (gameMode->GetLayerCount() > 1)
        {
            if (player->mFallStartPosition != NULL)
            {
                const double fallStartY = player->mFallStartPosition->y;
                stats->getRoundStats(mCurrentRound)->mDeathLayer = gameMode->GetLayer(fallStartY);
            }
            else
            {
                stats->getRoundStats(mCurrentRound)->mDeathLayer = PlayerStats::RoundStats::NO_LAYER;
            }
        }

        stats->getRoundStats(mCurrentRound)->mDeathOrder = mRoundDeathCount;

        bool killedByPlayer = false;
        if (fatalDamage->mAttackerEntity != NULL)
            killedByPlayer = eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, fatalDamage->mAttackerEntity->GetType());
        if (killedByPlayer)
        {
            boost::shared_ptr<ServerPlayer> killer = boost::static_pointer_cast<ServerPlayer>(
                fatalDamage->mAttackerEntity);
            lifeStats->mKillerName = killer->getNetworkName();
            stats->getRoundStats(mCurrentRound)->mFatalWeapon = fatalDamage->mWeapon;

            if (killer != entity)
            {
                PlayerStats* killerStats = GetPlayerStats(killer->getSUID());
                if (killerStats != NULL)
                {
                    ++killerStats->getRoundStats(mCurrentRound)->mKills;
                    killer->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(killer->getSUID(), deathCount);
                    if (Minecraft::InMiniGame(eMiniGameId_Tumble, true)
                        && killerStats->getTotalKills() == SUCK_MY_SNOWBALLS_KILLS)
                        killer->awardStat(GenericStats::suckMySnowballs(),
                                          GenericStats::param_suckMySnowballs(SUCK_MY_SNOWBALLS_KILLS));
                }
            }
        }

        player->getLevel()->GetServerScoreboard()->OnPlayerStatsChanged(player->getSUID(), deathCount);
    }
}

void GameStats::RecordJump(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        ++stats->getRoundStats(mCurrentRound)->mJumpCount;
}

void GameStats::RecordItem(const StatsUID& uid, not_null_ptr<ItemInstance> item)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;

    std::map<int, int>& items = stats->getRoundStats(mCurrentRound)->mItems;
    const int itemId = Item::VALIDgetId(item->getItem());
    if (items.find(itemId) != items.end())
        items[itemId] += item->getCount();
    else
        items.insert(std::make_pair(itemId, item->getCount()));
}

void GameStats::RecordContainer(const StatsUID& uid, int type, int x, int y, int z)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;

    std::vector<ContainerInfo*>& containers = stats->getRoundStats(mCurrentRound)->mContainers;
    for (std::vector<ContainerInfo*>::iterator it = containers.begin(); it != containers.end(); ++it)
    {
        if ((*it)->mX == x && (*it)->mY == y && (*it)->mZ == z)
            return;
    }

    ContainerInfo* container = new ContainerInfo();
    container->mX = x;
    container->mY = y;
    container->mZ = z;
    container->mType = type;
    containers.push_back(container);

    unsigned int containerCount = 0;
    LevelRuleset* rules = gApp.getGameRuleDefinitions();
    if (rules != NULL)
    {
        GameRuleDefinition* initialiseWorld = rules->getRuleByType(ConsoleGameRules::eGameRuleType_OnInitialiseWorld);
        if (initialiseWorld != NULL)
        {
            std::vector<GameRuleDefinition*> distributions;
            initialiseWorld->getChildren(&distributions, ConsoleGameRules::eGameRuleType_DistributeItems);
            for (std::vector<GameRuleDefinition*>::iterator it = distributions.begin(); it != distributions.end(); ++it)
            {
                DistributeItemsRuleDefinition* distribution = dynamic_cast<DistributeItemsRuleDefinition*>(*it);
                if (distribution == NULL)
                    continue;
                std::vector<GameRuleDefinition*> positions;
                distribution->getChildren(&positions);
                containerCount += positions.size();
            }
        }
    }

    if (containerCount != containers.size())
        return;

    boost::shared_ptr<ServerPlayer> player =
        (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->getPlayerFromStatsUID(uid);
    if (player != NULL)
        player->awardStat(GenericStats::mine(), GenericStats::param_mine());
}

void GameStats::RecordWalk(const StatsUID& uid, int distance, bool sprinting, bool sneaking)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;

    if (sprinting)
        stats->getRoundStats(mCurrentRound)->mSprintDistance += distance;
    else if (sneaking)
        stats->getRoundStats(mCurrentRound)->mSneakDistance += distance;
    else
        stats->getRoundStats(mCurrentRound)->mWalkDistance += distance;
}

void GameStats::RecordSwim(const StatsUID& uid, int distance, bool underwater)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;

    if (underwater)
        stats->getRoundStats(mCurrentRound)->mUnderwaterDistance += distance;
    else
        stats->getRoundStats(mCurrentRound)->mSwimDistance += distance;
}

void GameStats::RecordClimb(const StatsUID& uid, int distance)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        stats->getRoundStats(mCurrentRound)->mClimbDistance += distance;
}

void GameStats::RecordGainPoints(const StatsUID& uid, int points)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        stats->getRoundStats(mCurrentRound)->mPoints += points;
}

void GameStats::ResetTempPoints(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;

    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    round->mLastPointsAdded = 0;
    round->mUnbankedPoints = 0;
    round->mUnbankedTargets.clear();
    std::memset(round->mUnbankedRings, 0, sizeof(round->mUnbankedRings));
}

bool GameStats::RecordSecondaryTargetReached(const StatsUID& uid, const std::wstring& target, int points,
                                             GlideRingGenerator::eGlideRingSize size)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return false;

    std::vector<std::wstring>& banked = stats->getRoundStats(mCurrentRound)->mBankedTargets;
    if (std::find(banked.begin(), banked.end(), target) != banked.end())
        return false;
    std::vector<std::wstring>& unbanked = stats->getRoundStats(mCurrentRound)->mUnbankedTargets;
    if (std::find(unbanked.begin(), unbanked.end(), target) != unbanked.end())
        return false;

    ++stats->getRoundStats(mCurrentRound)->mTargetsReached;
    stats->getRoundStats(mCurrentRound)->mUnbankedTargets.push_back(target);
    stats->getRoundStats(mCurrentRound)->mUnbankedPoints += points;
    stats->getRoundStats(mCurrentRound)->mLastPointsAdded = points;
    ++stats->getRoundStats(mCurrentRound)->mUnbankedRings[size];
    return true;
}

void GameStats::TakeTimestamp(const StatsUID& uid, eTimeSpentCategory category, bool start)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
    {
        long long elapsed = System::processTimeInMilliSecs() - mRoundStartTime;
        stats->getRoundStats(mCurrentRound)->MedalTimestamp(category, start, elapsed);
    }
}

void GameStats::RecordUseItem(const StatsUID& uid, not_null_ptr<ItemInstance> item)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;

    stats->getRoundStats(mCurrentRound)->mItemsUsed.push_back(item->getItem());
    if (item->getItem() == Items::potion)
        ++stats->getRoundStats(mCurrentRound)->mPotionsUsed;
    else if (item->getItem() == Items::splashPotion)
        ++stats->getRoundStats(mCurrentRound)->mSplashPotionsUsed;
    else if (item->getItem() == Items::snowball)
        ++stats->getRoundStats(mCurrentRound)->mSnowballsThrown;
}

void GameStats::RecordTimeOnLayer(const StatsUID& uid, int layer, bool enteredLayer)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL || layer == PlayerStats::RoundStats::NO_LAYER)
        return;

    const int missingLayers = layer + 1 - static_cast<int>(stats->getRoundStats(mCurrentRound)->mTimeOnLayer.size());
    for (int added = 0; added < missingLayers; ++added)
        stats->getRoundStats(mCurrentRound)->mTimeOnLayer.push_back(0);

    ++stats->getRoundStats(mCurrentRound)->mTimeOnLayer.at(layer);
    if (enteredLayer)
        ++stats->getRoundStats(mCurrentRound)->mLayerChanges;
}

void GameStats::RecordSnowballDirectHit(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        ++stats->mSnowballDirectHits;
}

void GameStats::RecordHitBySnowball(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        ++stats->mHitsBySnowball;
}

void GameStats::RecordLanding(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL || mLandingRecorded)
        return;

    mLandingRecorded = true;
    stats->getRoundStats(mCurrentRound)->mLanded = true;
}

void GameStats::RecordBlockPlaced(const StatsUID& uid, const BlockPos& pos, int blockId)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    writeBlockInteractionData(*round, round->mBlocksPlaced, mRoundStartTime, pos, blockId);
}

void GameStats::RecordBlockDestroyed(const StatsUID& uid, const BlockPos& pos, int blockId)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return;
    PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
    writeBlockInteractionData(*round, round->mBlocksDestroyed, mRoundStartTime, pos, blockId);
}

bool GameStats::RecordLastCheckpoint(const StatsUID& uid, int checkpoint, int sequence, bool banked)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL
        && (stats->getRoundStats(mCurrentRound)->mLastCheckpointID != checkpoint
            || stats->getRoundStats(mCurrentRound)->mLastCheckpointSequenceID != sequence))
    {
        stats->getRoundStats(mCurrentRound)->mLastCheckpointID = checkpoint;
        stats->getRoundStats(mCurrentRound)->mLastCheckpointSequenceID = sequence;

        if (banked)
        {
            stats->getRoundStats(mCurrentRound)->mLastBankedCheckpointID = checkpoint;
            const unsigned int elapsedSeconds = static_cast<unsigned int>(
                std::floor(static_cast<double>(System::processTimeInMilliSecs() - mRoundStartTime))
                / MILLISECONDS_PER_SECOND_EXACT);
            stats->getRoundStats(mCurrentRound)->mCheckpointTimes[checkpoint] = elapsedSeconds;
            stats->getRoundStats(mCurrentRound)->mPoints += stats->getRoundStats(mCurrentRound)->mUnbankedPoints;
            stats->getRoundStats(mCurrentRound)->mUnbankedPoints = 0;
            if (stats->getRoundStats(mCurrentRound)->mLastPointsAdded != 0)
                stats->getRoundStats(mCurrentRound)->mLastBankedPoints =
                    stats->getRoundStats(mCurrentRound)->mLastPointsAdded;
            stats->getRoundStats(mCurrentRound)->mLastPointsAdded = 0;

            for (unsigned int target = 0; target < stats->getRoundStats(mCurrentRound)->mUnbankedTargets.size();
                 ++target)
                stats->getRoundStats(mCurrentRound)
                    ->mBankedTargets.push_back(stats->getRoundStats(mCurrentRound)->mUnbankedTargets.at(target));
            stats->getRoundStats(mCurrentRound)->mUnbankedTargets.clear();

            for (int ring = 0; ring < PlayerStats::RoundStats::RING_SIZE_COUNT; ++ring)
                stats->getRoundStats(mCurrentRound)->mBankedRings[ring] +=
                    stats->getRoundStats(mCurrentRound)->mUnbankedRings[ring];
            std::memset(stats->getRoundStats(mCurrentRound)->mUnbankedRings, 0,
                        sizeof(stats->getRoundStats(mCurrentRound)->mUnbankedRings));
        }

        if (static_cast<unsigned int>(checkpoint) < mFarthestCheckpointReached)
            checkpoint = mFarthestCheckpointReached;
        mFarthestCheckpointReached = checkpoint;
        return true;
    }
    return false;
}

void GameStats::RecordLapCompleted(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        ++stats->getRoundStats(mCurrentRound)->mLapsCompleted;
}

unsigned int GameStats::GetLapsCompleted(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    unsigned int laps = 0;
    if (stats != NULL)
        laps = stats->getRoundStats(mCurrentRound)->mLapsCompleted;
    return laps;
}

unsigned int GameStats::GetLastCheckpointID(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    unsigned int checkpoint = 0;
    if (stats != NULL)
        checkpoint = stats->getRoundStats(mCurrentRound)->mLastCheckpointID;
    return checkpoint;
}

unsigned int GameStats::GetLastCheckpointSequenceID(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    unsigned int sequence = 0;
    if (stats != NULL)
        sequence = stats->getRoundStats(mCurrentRound)->mLastCheckpointSequenceID;
    return sequence;
}

void GameStats::RecordThermalEntered(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        ++stats->getRoundStats(mCurrentRound)->mThermalsEntered;
}

bool GameStats::RecordProgress(const StatsUID& uid, double sequenceProgress, double nearestDistance, double progress,
                               double speed)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL || stats->getRoundStats(mCurrentRound)->mProgress == progress)
        return false;

    const double previousProgress = stats->getRoundStats(mCurrentRound)->mProgress;
    stats->getRoundStats(mCurrentRound)
        ->MedalTimestamp(eTimeSpentCategory_GoingBackwards, progress < previousProgress,
                         System::processTimeInMilliSecs() - mRoundStartTime);
    stats->getRoundStats(mCurrentRound)->mSequenceProgress = sequenceProgress;
    stats->getRoundStats(mCurrentRound)->mNearestDistance = nearestDistance;
    stats->getRoundStats(mCurrentRound)->mProgress = progress;
    stats->getRoundStats(mCurrentRound)->mTopSpeed = std::max(stats->getRoundStats(mCurrentRound)->mTopSpeed, speed);
    if (!stats->getRoundStats(mCurrentRound)->mReachedEnd)
        stats->getRoundStats(mCurrentRound)->mLifeSpan = static_cast<int>(System::processTimeInMilliSecs()
                                                                          - mRoundStartTime);
    return true;
}

void GameStats::RecordSpawnedAtCheckpoint(const StatsUID& uid, int checkpoint)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
        stats->getRoundStats(mCurrentRound)->mSpawnedCheckpoints.insert(checkpoint);
}

void GameStats::RecordTimeSpentInPosition(bool pointsActive)
{
    double lowest = DBL_MAX;
    double highest = 0.0;
    StatsUID leader = StatsUID::EMPTY_SUID();
    StatsUID trailer = StatsUID::EMPTY_SUID();

    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        PlayerStats::RoundStats* round = it->second->getRoundStats(mCurrentRound);
        const double value = pointsActive ? static_cast<unsigned int>(round->mPoints + round->mUnbankedPoints)
                                          : round->mProgress;
        if (value > highest)
        {
            leader = it->first;
            highest = value;
        }
        else if (value < lowest)
        {
            trailer = it->first;
            lowest = value;
        }
    }

    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        PlayerStats::RoundStats* round = it->second->getRoundStats(mCurrentRound);
        const long long elapsed = System::processTimeInMilliSecs() - mRoundStartTime;
        const bool isLeader = leader == it->first;
        const bool isTrailer = trailer == it->first;
        round->MedalTimestamp(eTimeSpentCategory_Leading, isLeader, elapsed);
        round->MedalTimestamp(eTimeSpentCategory_Trailing, isTrailer, elapsed);
    }
}

bool GameStats::RecordEndZoneEntered(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
    {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        if (!round->mReachedEnd)
        {
            round->mReachedEnd = true;
            round->mProgress = END_ZONE_PROGRESS;
            round->mLifeSpan = static_cast<int>(System::processTimeInMilliSecs() - mRoundStartTime);
            return true;
        }
    }
    return false;
}

bool GameStats::GetEndZoneEntered(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats != NULL)
    {
        PlayerStats::RoundStats* round = stats->getRoundStats(mCurrentRound);
        return round->mReachedEnd;
    }
    return false;
}

boost::shared_ptr<ServerPlayer> GameStats::GetLeadingPlayer()
{
    boost::shared_ptr<ServerPlayer> leader;
    double leaderProgress = NO_PROGRESS;

    std::vector<boost::shared_ptr<ServerPlayer> >* players =
        (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        PlayerStats* stats = GetPlayerStats(player->getSUID());
        if (stats == NULL)
            continue;

        const double progress = stats->getRoundStats(mCurrentRound)->mProgress;
        if (progress > leaderProgress || leader == NULL || (leader->isSpectator() && !player->isSpectator()))
        {
            leader = player;
            leaderProgress = progress;
        }
    }
    return leader;
}

boost::shared_ptr<ServerPlayer> GameStats::GetTrailingPlayer(const boost::shared_ptr<ServerPlayer>& excluded)
{
    boost::shared_ptr<ServerPlayer> trailer;
    double trailerProgress = NO_PROGRESS;

    std::vector<boost::shared_ptr<ServerPlayer> >* players =
        (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->getPlayers();
    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        if (player != NULL && excluded != NULL && player->getXuid() == excluded->getXuid())
            continue;

        PlayerStats* stats = GetPlayerStats(player->getSUID());
        if (stats == NULL)
            continue;

        const double progress = stats->getRoundStats(mCurrentRound)->mProgress;
        if (progress < trailerProgress || trailer == NULL || (trailer->isSpectator() && !player->isSpectator()))
        {
            trailer = player;
            trailerProgress = progress;
        }
    }
    return trailer;
}

void GameStats::increasePlayerRoundStartTimeMs(long long milliseconds)
{
    mRoundStartTime += milliseconds;
    if (mRoundEndTime > 0)
        mRoundEndTime += milliseconds;
}

long long GameStats::GetStartTime()
{
    return mRoundStartTime;
}

boost::shared_ptr<ServerPlayer> GameStats::GetRoundWinner()
{
    return mRoundWinner;
}

boost::shared_ptr<ServerPlayer> GameStats::GetSecondPlace()
{
    return mSecondPlace.lock();
}

boost::shared_ptr<ServerPlayer> GameStats::GetThirdPlace()
{
    return mThirdPlace.lock();
}

std::wstring GameStats::GetWinningThemeChoice() const
{
    return mWinningThemeChoice;
}

unsigned int GameStats::GetRoundsWon(const StatsUID& uid)
{
    PlayerStats* stats = GetPlayerStats(uid);
    if (stats == NULL)
        return 0;
    return stats->getTotalRoundsWon();
}

unsigned int GameStats::GetMostBlocksDestroyed()
{
    unsigned int most = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second != NULL)
        {
            const unsigned int destroyed = it->second->getTotalBlocksDestroyed();
            if (destroyed > most)
                most = destroyed;
        }
    }
    return most;
}

unsigned int GameStats::GetSecondMostBlocksDestroyed()
{
    unsigned int most = 0;
    unsigned int second = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second == NULL)
            continue;
        const unsigned int destroyed = it->second->getTotalBlocksDestroyed();
        if (destroyed > most)
            most = destroyed;
        else if (destroyed > second && destroyed < most)
            second = destroyed;
    }
    return second;
}

unsigned int GameStats::GetThirdMostBlocksDestroyed()
{
    unsigned int most = 0;
    unsigned int second = 0;
    unsigned int third = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(); it != mPlayerStats.end(); ++it)
    {
        if (it->second != NULL)
        {
            const unsigned int destroyed = it->second->getTotalBlocksDestroyed();
            if (destroyed > most)
                most = destroyed;
            else if (destroyed > second && destroyed < most)
                second = destroyed;
            else if (destroyed > third && destroyed < second)
                third = destroyed;
        }
    }
    return third;
}

unsigned int GameStats::GetFastestPlayerTime()
{
    unsigned int fastest = UINT_MAX;
    for (int round = 0; round <= static_cast<int>(mCurrentRound); ++round)
        fastest = std::min(fastest, GetShortestLifespan(round));
    return fastest;
}

unsigned int GameStats::GetHighestScoringPlayerScore()
{
    unsigned int highest = 0;
    for (StatsMap::iterator it = mPlayerStats.begin(), end = mPlayerStats.end(); it != end; it++)
    {
        const unsigned int points = it->second->getBestPoints(true, false);
        if (points > highest)
            highest = points;
    }
    return highest;
}

unsigned int GameStats::GetFarthestCheckpointReached()
{
    return mFarthestCheckpointReached;
}
