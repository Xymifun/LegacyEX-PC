#include "game/core/WinnerCondition.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/GameStats.hpp"
#include "game/core/StatsUID.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/Player.hpp"

WinnerCondition::WinnerCondition() : MultiConditionMedal::MultiCondition(-1.0f, GameMedal::OPERATOR_EQUAL, false)
{
}

bool WinnerCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    boost::shared_ptr<ServerPlayer> firstPlace = gameStats->GetFirstPlace();
    return uid == firstPlace->getSUID();
}
