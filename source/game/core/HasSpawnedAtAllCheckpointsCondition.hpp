#ifndef GAME_CORE_HASSPAWNEDATALLCHECKPOINTSCONDITION_HPP
#define GAME_CORE_HASSPAWNEDATALLCHECKPOINTSCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class HasSpawnedAtAllCheckpointsCondition : public MultiConditionMedal::MultiCondition
{
public:
    HasSpawnedAtAllCheckpointsCondition();
    virtual ~HasSpawnedAtAllCheckpointsCondition();

    virtual bool canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const;
};

#endif
