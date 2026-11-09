#include "game/world/generation/DefendVillageTargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/VillagerGolem.hpp"

DefendVillageTargetGoal::DefendVillageTargetGoal(VillagerGolem* golem) : TargetGoal(golem, false, true)
{
    mGolem = golem;
    setRequiredControlFlags(1);
}

bool DefendVillageTargetGoal::canUse()
{
    boost::shared_ptr<Village> village = mGolem->getVillage();
    if (village == NULL)
    {
        return false;
    }

    mVillageAggressorTarget = boost::weak_ptr<LivingEntity>(
        village->getClosestAggressor(boost::dynamic_pointer_cast<LivingEntity>(mGolem->shared_from_this())));

    boost::shared_ptr<LivingEntity> target = mVillageAggressorTarget.lock();
    if (target != NULL && eTYPE_DERIVED_FROM(EntityType::CREEPER, target->GetType()))
    {
        return false;
    }
    else if (canAttack(target, false))
    {
        return true;
    }
    else if (mMob->getRandom()->nextInt(20) == 0)
    {
        mVillageAggressorTarget = village->getClosestBadStandingPlayer(
            boost::dynamic_pointer_cast<LivingEntity>(mGolem->shared_from_this()));
        return canAttack(mVillageAggressorTarget.lock(), false);
    }
    else
    {
        return false;
    }
}

void DefendVillageTargetGoal::start()
{
    mGolem->setTarget(mVillageAggressorTarget.lock());
    TargetGoal::start();
}
