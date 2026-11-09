#ifndef GAME_CORE_FASTESTTIMEANDHIGHTESTSCOREINAROUNDCONDITION_HPP
#define GAME_CORE_FASTESTTIMEANDHIGHTESTSCOREINAROUNDCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class FastestTimeAndHightestScoreInARoundCondition : public MultiConditionMedal::MultiCondition
{
public:
    FastestTimeAndHightestScoreInARoundCondition();
    virtual ~FastestTimeAndHightestScoreInARoundCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
