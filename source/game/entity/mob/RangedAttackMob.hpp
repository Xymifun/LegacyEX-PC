#ifndef GAME_ENTITY_MOB_RANGEDATTACKMOB_HPP
#define GAME_ENTITY_MOB_RANGEDATTACKMOB_HPP

#include <boost/shared_ptr.hpp>

class LivingEntity;

class RangedAttackMob
{
public:
    virtual ~RangedAttackMob() {}

    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor) = 0;
    virtual bool isAggressive() = 0;
    virtual void setAggressive(bool aggressive) = 0;
};

#endif
