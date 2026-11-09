#include "game/world/generation/TimeSpentOnLayerMedal.hpp"

#include "game/core/MasterGameMode.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerStats.hpp"

TimeSpentOnLayerMedal::TimeSpentOnLayerMedal(ELayerMedal layer, int name, float target, unsigned int weight,
                                             GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, true, false),
      mLayer(layer)
{
}

float TimeSpentOnLayerMedal::GetScore(PlayerStats* playerStats) const
{
    unsigned int total = 0;
    unsigned int layer = 0;

    if (mLayer == LAYER_MEDAL_BOTTOM)
    {
        layer = static_cast<unsigned int>(gMinecraftServer->mMasterGameMode->GetLayerCount() - 1);
    }

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); ++round)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        if (roundStats->mDeathLayer != PlayerStats::RoundStats::LAYERS_UNUSED
            && layer < roundStats->mTimeOnLayer.size())
        {
            unsigned int time = roundStats->mTimeOnLayer.at(layer);
            total += time;
        }
    }

    return static_cast<float>(static_cast<int>(total));
}
