#ifndef GAME_ENTITY_AI_FINDMOUNTGOAL_HPP
#define GAME_ENTITY_AI_FINDMOUNTGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Entity;
class Mob;

class FindMountGoal : public Goal
{
public:
    FindMountGoal(Mob& mob, float targetDistance, int startDelay, bool avoidWater, bool targetNeeded,
                  float mountDistance);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName();

private:
    float getAttackReachSqr();

    boost::weak_ptr<Entity> mTarget;
    float mTargetDistance;
    int mTimeToRecalcPath;
    int mStartCounter;
    int mStartDelay;
    bool mAvoidWater;
    bool mTargetNeeded;
    float mMountDistance;
    Mob* mMob;
};

#endif
