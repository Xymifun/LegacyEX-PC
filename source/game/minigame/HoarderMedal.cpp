#include "game/minigame/HoarderMedal.hpp"

#include <map>

#include "game/world/generation/PlayerStats.hpp"

float HoarderMedal::GetScore(PlayerStats* playerStats) const
{
    std::map<int, int> items;
    playerStats->getAllItems(items);
    return (float)items.size();
}
