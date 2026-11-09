#include "game/entity/ai/OwnerHurtByTargetGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/TamableAnimal.hpp"

OwnerHurtByTargetGoal::OwnerHurtByTargetGoal(TamableAnimal* tamableAnimal) : TargetGoal(tamableAnimal, false)
{
    mTamableAnimal = tamableAnimal;
    mTimestamp = 0;
    setRequiredControlFlags(1);
}

bool OwnerHurtByTargetGoal::canUse()
{
    if (!mTamableAnimal->isTame())
        return false;

    boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(mTamableAnimal->getOwner());
    if (!owner)
        return false;

    mOwnerLastHurtBy = boost::weak_ptr<LivingEntity>(owner->getLastHurtByMob());
    int timestamp = owner->getLastHurtByMobTimestamp();
    boost::shared_ptr<LivingEntity> lastHurtBy = mOwnerLastHurtBy.lock();
    return timestamp != mTimestamp && canAttack(lastHurtBy, false) && mTamableAnimal->wantsToAttack(lastHurtBy, owner)
           && !mMob->mHurtByPlayerOnPeaceful;
}

void OwnerHurtByTargetGoal::start()
{
    mMob->setTarget(mOwnerLastHurtBy.lock());

    boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(mTamableAnimal->getOwner());
    if (owner)
        mTimestamp = owner->getLastHurtByMobTimestamp();

    TargetGoal::start();
}
