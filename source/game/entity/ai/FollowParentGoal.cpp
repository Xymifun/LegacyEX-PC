#include "game/entity/ai/FollowParentGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

FollowParentGoal::FollowParentGoal(Animal* animal, double speedModifier)
{
    mTimeToRecalcPath = 0;
    mAnimal = animal;
    mSpeedModifier = speedModifier;
}

bool FollowParentGoal::canUse()
{
    if (mAnimal->getAge() >= 0)
    {
        return false;
    }

    Animal* animal = mAnimal;
    std::vector<boost::shared_ptr<Entity> >* animals = animal->mLevel->getEntitiesOfClass(
        typeid(*animal), animal->mBoundingBox->grow(8.0, 4.0, 8.0));

    boost::shared_ptr<Entity> parent;
    double closestDistance = 1.7976931348623157e+308;

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = animals->begin(); it != animals->end(); ++it)
    {
        Animal* candidate = static_cast<Animal*>(it->get());
        if (candidate->getAge() >= 0)
        {
            double distance = mAnimal->distanceToSqr(BlockPos(candidate));
            if (distance <= closestDistance)
            {
                closestDistance = distance;
                parent = *it;
            }
        }
    }

    delete animals;

    if (!parent)
    {
        return false;
    }

    if (closestDistance < 9.0)
    {
        return false;
    }

    mParentAnimal = boost::dynamic_pointer_cast<Animal>(parent);
    return true;
}

bool FollowParentGoal::canContinueToUse()
{
    if (mAnimal->getAge() >= 0)
    {
        return false;
    }

    if (!mParentAnimal.lock() || !mParentAnimal.lock()->isAlive())
    {
        return false;
    }

    double distance = mAnimal->distanceToSqr(mParentAnimal.lock());
    if (distance < 9.0 || distance > 256.0)
    {
        return false;
    }

    return true;
}

void FollowParentGoal::start()
{
    mTimeToRecalcPath = 0;
}

void FollowParentGoal::stop()
{
    mParentAnimal = boost::weak_ptr<Animal>();
}

void FollowParentGoal::tick()
{
    if (--mTimeToRecalcPath <= 0)
    {
        mTimeToRecalcPath = 10;
        mAnimal->getNavigation()->moveTo(mParentAnimal.lock(), mSpeedModifier);
    }
}
