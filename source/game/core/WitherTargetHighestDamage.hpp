#ifndef GAME_CORE_WITHERTARGETHIGHESTDAMAGE_HPP
#define GAME_CORE_WITHERTARGETHIGHESTDAMAGE_HPP
#include <boost/shared_ptr.hpp>
#include "game/entity/ai/TargetGoal.hpp"
class LivingEntity;
class WitherBoss;
class WitherTargetHighestDamage : public TargetGoal
{
public:
    explicit WitherTargetHighestDamage(WitherBoss*);
    boost::shared_ptr<LivingEntity> getHighestDamageTarget();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual bool canAttack(boost::shared_ptr<LivingEntity>, bool);
};
#endif
