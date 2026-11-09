#ifndef GAME_MINIGAME_FALLKILLMEDAL_HPP
#define GAME_MINIGAME_FALLKILLMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class FallKillMedal : public GameMedal
{
public:
    FallKillMedal(int name, float target, unsigned int weight);

    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
