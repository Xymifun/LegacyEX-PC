#include "game/entity/ai/NonTameRandomTargetGoal.hpp"

#include "game/entity/mob/TamableAnimal.hpp"

NonTameRandomTargetGoal::NonTameRandomTargetGoal(TamableAnimal* tamableAnimal, const std::type_info& targetType,
                                                 bool mustSee, const Predicate<boost::shared_ptr<Entity> >* selector,
                                                 bool includeRemoved)
    : NearestAttackableTargetGoal(tamableAnimal, targetType, 10, mustSee, false, selector, true, includeRemoved),
      mTamableAnimal(tamableAnimal)
{
}

bool NonTameRandomTargetGoal::canUse()
{
    return !mTamableAnimal->isTame() && NearestAttackableTargetGoal::canUse();
}

bool NonTameRandomTargetGoal::canContinueToUse()
{
    bool result = false;
    if (!mTamableAnimal->isTame())
    {
        result = NearestAttackableTargetGoal::canContinueToUse();
    }
    return result;
}
