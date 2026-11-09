#ifndef GAME_ENTITY_AI_SWOOPATTACKGOAL_HPP
#define GAME_ENTITY_AI_SWOOPATTACKGOAL_HPP

#include "game/entity/ai/Goal.hpp"

class Mob;

class SwoopAttackGoal : public Goal
{
public:
    SwoopAttackGoal(Mob* mob, float speedModifier, Goal::FloatRange attackDistance);
    virtual ~SwoopAttackGoal();
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual bool canContinueToUse();
    virtual std::wstring getName() { return L"Swooping Attack!"; }

private:
    Mob* mMob;
    float mSpeedModifier;
    Goal::FloatRange mAttackDistance;
    int mAttackInterval;
    int mSwoopTicks;
    bool mSwooping;
};

#endif
