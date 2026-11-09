#ifndef GAME_ENTITY_AI_RANDOMHOPGOAL_HPP
#define GAME_ENTITY_AI_RANDOMHOPGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class PathfinderMob;

class RandomHopGoal : public Goal
{
public:
    RandomHopGoal(PathfinderMob* mob, double speedModifier);
    RandomHopGoal(PathfinderMob* mob, double speedModifier, int interval);

    void _init(PathfinderMob* mob, double speedModifier, int interval);

    virtual ~RandomHopGoal();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual std::wstring getName() { return L"RandomHop"; }

    PathfinderMob* mMob;
    double mWantedX;
    double mWantedY;
    double mWantedZ;
    double mSpeedModifier;
    int mInterval;
};

#endif
