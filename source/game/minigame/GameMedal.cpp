#include "game/minigame/GameMedal.hpp"

#include <math.h>

#include "game/core/GameStats.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerStats.hpp"

GameMedal::GameMedal(int name, unsigned int weight)
{
    mName = name;
    mWeight = weight;
    mCanMultiAward = false;
    mId = -1;
}

GameMedal::GameMedal(int name, float target, unsigned int weight, EOperator op, bool bestOnly, bool canMultiAward)
{
    mName = name;
    mWeight = weight;
    mId = -1;
    mCanMultiAward = canMultiAward;
    mConditions.push_back(new Condition(target, op, bestOnly));
}

GameMedal::~GameMedal()
{
    for (unsigned int i = 0; i < mConditions.size(); i++)
    {
        delete mConditions[i];
    }
}

bool GameMedal::defaultCanAward(const StatsUID& uid, GameStats* gameStats, float score, float target, EOperator op)
{
    switch (op)
    {
    case OPERATOR_AT_LEAST:
        return score >= target;

    case OPERATOR_AT_MOST:
        return score <= target;

    case OPERATOR_EQUAL:
        return (int)floorf(score + 0.5f) == (int)floorf(target + 0.5f);
    }

    return false;
}

bool GameMedal::CanAward(const StatsUID& uid, GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats == NULL)
    {
        return false;
    }

    float score = GetScore(playerStats);
    return defaultCanAward(uid, gameStats, score, mConditions[0]->mTarget, mConditions[0]->mOperator);
}

int GameMedal::GetName() const
{
    return mName;
}

float GameMedal::GetScore(PlayerStats* playerStats) const
{
    return 0.0;
}

int GameMedal::FindBest(const std::vector<boost::shared_ptr<ServerPlayer> >* players, GameStats* gameStats) const
{
    int best = -2;

    for (unsigned int i = 0; i < mConditions.size(); i++)
    {
        Condition* condition = mConditions[i];
        if (condition->mBestOnly)
        {
            int index = condition->findBest(this, players, gameStats);
            if (best == -2)
            {
                best = index;
            }

            if (index != best)
            {
                return -1;
            }
        }
    }

    if (best >= 0 && best < (int)players->size())
    {
        StatsUID uid = players->at(best)->getSUID();

        for (unsigned int i = 0; i < mConditions.size(); i++)
        {
            Condition* condition = mConditions[i];
            if (!condition->mBestOnly && !condition->canAward(this, uid, gameStats))
            {
                return -1;
            }
        }
    }

    return best;
}

bool GameMedal::BestOnly() const
{
    for (unsigned int i = 0; i < mConditions.size(); i++)
    {
        if (mConditions[i]->mBestOnly)
        {
            return true;
        }
    }

    return false;
}

unsigned int GameMedal::GetWeight() const
{
    return mWeight;
}

void GameMedal::SetCanMultiAward(bool canMultiAward)
{
    mCanMultiAward = canMultiAward;
}

bool GameMedal::CanMultiAward() const
{
    return mCanMultiAward;
}

void GameMedal::SetId(int id)
{
    mId = id;
}

int GameMedal::GetId() const
{
    return mId;
}

GameStats* GameMedal::getGameStats() const
{
    return &gMinecraftServer->mMasterGameMode->mGameStats;
}

float GameMedal::totalOfAllRounds(PlayerStats* playerStats, int (&getRoundValue)(const PlayerStats::RoundStats&))
{
    int total = 0;

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); round++)
    {
        total += getRoundValue(*playerStats->getRoundStats(round));
    }

    return (float)total;
}

GameMedal::Condition::Condition(float target, EOperator op, bool bestOnly)
    : mTarget(target),
      mBestOnly(bestOnly),
      mOperator(op)
{
}

int GameMedal::Condition::findBest(const GameMedal* medal, const std::vector<boost::shared_ptr<ServerPlayer> >* players,
                                   GameStats* gameStats) const
{
    int best = -1;
    double bestScore = 0.0;

    for (unsigned int i = 0; i < players->size(); i++)
    {
        StatsUID uid = players->at(i)->getSUID();

        PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
        if (playerStats == NULL)
        {
            continue;
        }

        double score = getScore(medal, playerStats);

        bool better = false;
        switch (mOperator)
        {
        case OPERATOR_AT_LEAST:
            better = score > bestScore;
            break;

        case OPERATOR_AT_MOST:
            better = score < bestScore;
            break;

        case OPERATOR_EQUAL:
            better = fabsf(score - mTarget) < fabsf(bestScore - mTarget);
            break;
        }

        bool award = canAward(medal, uid, gameStats);

        if ((best == -1 || better) && award)
        {
            best = i;
            bestScore = score;
        }
    }

    return best;
}

bool GameMedal::Condition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    return medal->CanAward(uid, gameStats);
}

float GameMedal::Condition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    return medal->GetScore(playerStats);
}
