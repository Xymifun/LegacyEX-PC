#ifndef GAME_ENTITY_AI_FOLLOWBOATGOAL_HPP
#define GAME_ENTITY_AI_FOLLOWBOATGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Dolphin;
class LivingEntity;

class FollowBoatGoal : public Goal
{
public:
    FollowBoatGoal(Dolphin* dolphin);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual bool canInterrupt();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName();

private:
    int mSearchDistance;
    int mMaxDistance;
    int mCooldown;
    boost::weak_ptr<LivingEntity> mTarget;
    int mState;
    Dolphin* mDolphin;
};

#endif
