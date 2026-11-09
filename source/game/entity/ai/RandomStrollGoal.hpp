#ifndef GAME_ENTITY_AI_RANDOMSTROLLGOAL_HPP
#define GAME_ENTITY_AI_RANDOMSTROLLGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class PathfinderMob;
class Vec3;

class RandomStrollGoal : public Goal
{
public:
    RandomStrollGoal(PathfinderMob* mob, double speedModifier);
    RandomStrollGoal(PathfinderMob* mob, double speedModifier, int interval);
    virtual ~RandomStrollGoal() {}
    void _init(PathfinderMob* mob, double speedModifier, int interval);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual Vec3* getPosition();
    void trigger();
    virtual void setInterval(int interval);
    virtual std::wstring getName() { return L"RandomStroll"; }

protected:
    PathfinderMob* mMob;
    double mWantedX;
    double mWantedY;
    double mWantedZ;
    double mSpeedModifier;
    int mInterval;
    bool mForceTrigger;
};

#endif
