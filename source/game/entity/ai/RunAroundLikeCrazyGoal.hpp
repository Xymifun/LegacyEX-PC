#ifndef GAME_ENTITY_AI_RUNAROUNDLIKECRAZYGOAL_HPP
#define GAME_ENTITY_AI_RUNAROUNDLIKECRAZYGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class AbstractHorse;

class RunAroundLikeCrazyGoal : public Goal
{
public:
    RunAroundLikeCrazyGoal(AbstractHorse* horse, double speedModifier);
    virtual ~RunAroundLikeCrazyGoal();
    virtual bool canUse();
    virtual void start();
    virtual bool canContinueToUse();
    virtual void tick();
    virtual std::wstring getName() { return L"RunAroundLikeCrazy"; }

private:
    AbstractHorse* mHorse;
    double mSpeedModifier;
    double mWantedX;
    double mWantedY;
    double mWantedZ;
};

#endif
