#ifndef GAME_MINIGAME_RINGSMEDAL_HPP
#define GAME_MINIGAME_RINGSMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class GameStats;
class StatsUID;

class RingsMedal : public GameMedal
{
public:
    enum eRingMedalAmount
    {
        eRingAmountNone,
        eRingAmountAll,
        eRingAmountSome,
        eRingAmountMost
    };

    RingsMedal(int name, unsigned int weight, eRingMedalAmount* amounts, EOperator op);

    virtual bool CanAward(const StatsUID& uid, GameStats* gameStats) const;

private:
    eRingMedalAmount mAmounts[3];
};

#endif
