#ifndef GAME_ENTITY_AI_RANGEDBOWATTACKGOAL_HPP
#define GAME_ENTITY_AI_RANGEDBOWATTACKGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Monster;

class RangedBowAttackGoal : public Goal
{
public:
    RangedBowAttackGoal(Monster* mob, double speedModifier, int attackInterval, float attackRadius);
    virtual ~RangedBowAttackGoal();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"RangedBowAtack"; }
    void setMinAttackInterval(int attackInterval);
    bool isHoldingBow();

private:
    Monster* mMob;
    double mSpeedModifier;
    int mAttackInterval;
    float mAttackRadiusSqr;
    int mAttackTime;
    int mSeeTime;
    bool mStrafingClockwise;
    bool mStrafingBackwards;
    int mStrafingTime;
};

#endif
