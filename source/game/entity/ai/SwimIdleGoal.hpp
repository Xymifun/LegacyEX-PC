#ifndef GAME_ENTITY_AI_SWIMIDLEGOAL_HPP
#define GAME_ENTITY_AI_SWIMIDLEGOAL_HPP

#include "game/entity/ai/Goal.hpp"

class Mob;
class Vec3;

class SwimIdleGoal : public Goal
{
public:
    SwimIdleGoal(Mob* mob);
    virtual ~SwimIdleGoal();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void tick();
    virtual std::wstring getName() { return L"swim_idle"; }

private:
    Mob* mMob;
    int mTicks;
    Vec3* mStartPos;
};

#endif
