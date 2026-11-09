#ifndef GAME_ENTITY_AI_RIVERFOLLOWINGGOAL_HPP
#define GAME_ENTITY_AI_RIVERFOLLOWINGGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/util/Vec3.hpp"

class Mob;

class RiverFollowingGoal : public Goal
{
public:
    RiverFollowingGoal(Mob* mob, float distance, float speed);
    virtual ~RiverFollowingGoal();

    virtual bool canUse();
    virtual void start();
    virtual void stop();
    void determineSteerDirection();
    virtual void tick();
    virtual bool canContinueToUse();
    virtual std::wstring getName() { return L"RiverFollowing"; }

private:
    Mob* mMob;
    Vec3* mSteerDirection;
    float mDistance;
    float mSpeed;
};

#endif
