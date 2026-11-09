#ifndef GAME_ENTITY_AI_LEAPATTARGETGOAL_HPP
#define GAME_ENTITY_AI_LEAPATTARGETGOAL_HPP

#include <string>

#include <boost/weak_ptr.hpp>

#include "game/entity/ai/Goal.hpp"

class LivingEntity;
class Mob;

class LeapAtTargetGoal : public Goal
{
public:
    LeapAtTargetGoal(Mob* mob, float yd);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual std::wstring getName() { return L"LeapAtTarget"; }

private:
    Mob* mMob;
    boost::weak_ptr<LivingEntity> mTarget;
    float mYd;
};

#endif
