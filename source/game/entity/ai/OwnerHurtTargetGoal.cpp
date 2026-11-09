#include "game/entity/ai/OwnerHurtTargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/TamableAnimal.hpp"

OwnerHurtTargetGoal::OwnerHurtTargetGoal(TamableAnimal* tamable) : TargetGoal(tamable, false)
{
    mTamable = tamable;
    setRequiredControlFlags(1);
    mTimestamp = 0;
}

bool OwnerHurtTargetGoal::canUse()
{
    if (!mTamable->isTame())
        return false;

    boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(mTamable->getOwner());
    if (!owner)
        return false;

    mOwnerLastHurt = boost::weak_ptr<LivingEntity>(owner->getLastHurtMob());
    int timestamp = owner->getLastHurtMobTimestamp();
    boost::shared_ptr<LivingEntity> lastHurt = mOwnerLastHurt.lock();
    return timestamp != mTimestamp && canAttack(lastHurt, false) && mTamable->wantsToAttack(lastHurt, owner);
}

void OwnerHurtTargetGoal::start()
{
    mMob->setTarget(mOwnerLastHurt.lock());

    boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(mTamable->getOwner());
    if (owner)
    {
        mTimestamp = owner->getLastHurtMobTimestamp();
        TargetGoal::start();
    }
}
