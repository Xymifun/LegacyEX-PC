#include "game/entity/ai/AvoidEntityGoalEntitySelector.hpp"

#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/core/Sensing.hpp"

AvoidEntityGoalEntitySelector::AvoidEntityGoalEntitySelector(AvoidEntityGoal* avoidEntityGoal)
    : mAvoidEntityGoal(avoidEntityGoal)
{
}

bool AvoidEntityGoalEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return entity->isAlive() && mAvoidEntityGoal->mMob->getSensing()->canSee(entity)
           && !mAvoidEntityGoal->mMob->isAlliedTo(entity);
}

AvoidEntityGoalEntitySelector::~AvoidEntityGoalEntitySelector()
{
}
