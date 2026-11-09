#ifndef GAME_MINIGAME_SNIPERMEDAL_HPP
#define GAME_MINIGAME_SNIPERMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class SniperMedal : public GameMedal
{
public:
    SniperMedal(int name, float target, unsigned int weight);

    virtual float GetScore(PlayerStats* playerStats) const;
};

#endif
