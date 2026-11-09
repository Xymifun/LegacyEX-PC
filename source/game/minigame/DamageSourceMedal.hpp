#ifndef GAME_MINIGAME_DAMAGESOURCEMEDAL_HPP
#define GAME_MINIGAME_DAMAGESOURCEMEDAL_HPP

#include <vector>

#include "game/core/Scoreboard.hpp"
#include "game/minigame/GameMedal.hpp"

class DamageSourceMedal : public GameMedal
{
public:
    DamageSourceMedal(int name, float target, unsigned int weight, Scoreboard::EDamageSource source);
    DamageSourceMedal(int name, float target, unsigned int weight, std::vector<Scoreboard::EDamageSource>* sources);
    virtual ~DamageSourceMedal();

    virtual float GetScore(PlayerStats* playerStats) const;

private:
    std::vector<Scoreboard::EDamageSource>* mSources;
};

#endif
