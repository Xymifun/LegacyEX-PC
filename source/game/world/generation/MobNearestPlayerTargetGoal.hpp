#ifndef GAME_WORLD_GENERATION_MOBNEARESTPLAYERTARGETGOAL_HPP
#define GAME_WORLD_GENERATION_MOBNEARESTPLAYERTARGETGOAL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/world/generation/CanAttackPlayerPredicate.hpp"

class Entity;
class LivingEntity;
class Mob;

class MobNearestPlayerTargetGoal : public Goal
{
public:
    MobNearestPlayerTargetGoal(Mob* mob);

    double getFollowDistance();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual std::wstring getName() { return L"MobNearestPlayerTarget"; }

    Mob* mMob;

private:
    CanAttackPlayerPredicate mTargetSelector;
    NearestAttackableTargetGoal::DistComp mSorter;
    boost::shared_ptr<LivingEntity> mTarget;
};

#endif
