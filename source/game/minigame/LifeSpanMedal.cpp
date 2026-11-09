#include "game/minigame/LifeSpanMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

LifeSpanMedal::LifeSpanMedal(int name, float target, unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, true, false)
{
}

float LifeSpanMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(static_cast<unsigned int>(playerStats->getTotalLifeSpan())));
}
