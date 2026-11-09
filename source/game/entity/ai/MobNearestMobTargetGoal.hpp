#ifndef GAME_ENTITY_AI_MOBNEARESTMOBTARGETGOAL_HPP
#define GAME_ENTITY_AI_MOBNEARESTMOBTARGETGOAL_HPP

#include <string>
#include <typeinfo>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/Goal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/core/CanAttackPredicate.hpp"

class LivingEntity;
class Mob;

class MobNearestMobTargetGoal : public Goal
{
public:
    MobNearestMobTargetGoal(Mob* mob, const std::type_info& targetType);

    double getFollowDistance();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual std::wstring getName() { return L"MobNearestMobTarget"; }

    Mob* mMob;
    CanAttackPredicate mCanAttack;
    NearestAttackableTargetGoal::DistComp mSorter;
    boost::shared_ptr<LivingEntity> mTarget;
    const std::type_info* mTargetType;
};

#endif
