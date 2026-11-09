#include "game/world/block/BlockTypesUsed.hpp"

#include <set>

float BlockTypesUsed::GetScore(PlayerStats* playerStats) const
{
    std::set<int> blockTypes;

    for (unsigned int round = 0; round < playerStats->getTotalRounds(); round++)
    {
        if (!playerStats->hasRoundStats(round))
        {
            continue;
        }

        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        std::map<int, int>& blocks = roundStats->mBlocksPresent.mByBlock;
        std::map<int, int>::const_iterator it = blocks.begin();
        while (it != blocks.end())
        {
            blockTypes.insert(it->first);
            ++it;
        }
    }

    return (float)blockTypes.size();
}
