#include "game/minigame/MultiConditionMedal.hpp"

#include "game/core/GameStats.hpp"

MultiConditionMedal::MultiConditionMedal(int name, unsigned int weight, MultiCondition* condition)
    : GameMedal(name, weight)
{
    mConditions.push_back(condition);
}

bool MultiConditionMedal::CanAward(const StatsUID& uid, GameStats* gameStats) const
{
    for (unsigned int i = 0; i < mConditions.size(); i++)
    {
        if (!mConditions[i]->canAward(this, uid, gameStats))
        {
            return false;
        }
    }

    return true;
}

float MultiConditionMedal::GetScore(PlayerStats* playerStats) const
{
    return GameMedal::GetScore(playerStats);
}

MultiConditionMedal* MultiConditionMedal::addExtraCondition(MultiCondition* condition)
{
    mConditions.push_back(condition);
    return this;
}

MultiConditionMedal::MultiCondition::MultiCondition(float target, GameMedal::EOperator op, bool bestOnly)
    : GameMedal::Condition(target, op, bestOnly)
{
}

bool MultiConditionMedal::MultiCondition::canAward(const GameMedal* medal, const StatsUID& uid,
                                                   GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats != NULL)
    {
        float score = getScore(medal, playerStats);
        return GameMedal::defaultCanAward(uid, gameStats, score, mTarget, mOperator);
    }

    return false;
}

float MultiConditionMedal::MultiCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    return 0.0f;
}
