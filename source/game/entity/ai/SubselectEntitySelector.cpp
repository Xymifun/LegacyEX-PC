#include "game/entity/ai/SubselectEntitySelector.hpp"

#include "game/entity/ai/EntitySelector.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"

SubselectEntitySelector::SubselectEntitySelector(NearestAttackableTargetGoal* goal,
                                                 const Predicate<boost::shared_ptr<Entity> >* selector,
                                                 bool ownsSelector)
    : mOwnsSelector(ownsSelector),
      mSelector(selector),
      mGoal(goal)
{
}

SubselectEntitySelector::~SubselectEntitySelector()
{
    if (mOwnsSelector && mSelector != NULL)
        delete mSelector;
}

bool SubselectEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    if (!entity)
        return false;

    if (mSelector != NULL && !mSelector->apply(entity))
        return false;

    if (!EntitySelector::NO_SPECTATORS->apply(entity))
        return false;

    return mGoal->canAttack(boost::dynamic_pointer_cast<LivingEntity>(entity), false);
}
