#include "game/core/FinishedLastRoundCondition.hpp"

#include "game/core/GameStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/PlayerStats.hpp"

FinishedLastRoundCondition::FinishedLastRoundCondition()
    : MultiConditionMedal::MultiCondition(0.0f, GameMedal::OPERATOR_EQUAL, false)
{
}

FinishedLastRoundCondition::~FinishedLastRoundCondition()
{
}

bool FinishedLastRoundCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats == NULL)
        return false;

    return playerStats->getReachedEnd(Minecraft::GetInstance()->GetMiniGame()->GetFinalRoundIndex());
}
