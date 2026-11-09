#ifndef GAME_MINIGAME_CONSOLATIONMEDAL_HPP
#define GAME_MINIGAME_CONSOLATIONMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class GameStats;
class StatsUID;

class ConsolationMedal : public GameMedal
{
public:
    ConsolationMedal(int name, float target, unsigned int weight, GameMedal::EOperator op);

    virtual bool CanAward(const StatsUID& uid, GameStats* gameStats) const;
};

#endif
