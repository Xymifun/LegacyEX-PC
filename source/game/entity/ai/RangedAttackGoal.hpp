#ifndef GAME_ENTITY_AI_RANGEDATTACKGOAL_HPP
#define GAME_ENTITY_AI_RANGEDATTACKGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"

class LivingEntity;
class Mob;

class RangedAttackGoal : public Goal
{
public:
    RangedAttackGoal(RangedAttackMob* rangedAttackMob, Mob* mob, double speedModifier, int attackInterval,
                     float attackRadius);
    RangedAttackGoal(RangedAttackMob* rangedAttackMob, Mob* mob, double speedModifier, int attackIntervalMin,
                     int attackIntervalMax, float attackRadius);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"RangedAttack"; }
    void _init(RangedAttackMob* rangedAttackMob, Mob* mob, double speedModifier, int attackIntervalMin,
               int attackIntervalMax, float attackRadius);

private:
    Mob* mMob;
    RangedAttackMob* mRangedAttackMob;
    int mAttackTime;
    int mUnknown;
    double mSpeedModifier;
    int mSeeTime;
    int mAttackIntervalMin;
    int mAttackIntervalMax;
    float mAttackRadius;
    float mAttackRadiusSqr;
    boost::weak_ptr<LivingEntity> mTarget;
};

#endif
