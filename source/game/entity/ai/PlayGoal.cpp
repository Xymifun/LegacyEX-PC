#include "game/entity/ai/PlayGoal.hpp"

#include <cfloat>
#include <typeinfo>
#include <vector>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villager.hpp"

PlayGoal::PlayGoal(Villager* villager, double speedModifier)
{
    mTarget = boost::weak_ptr<Villager>();
    mPlayPosition.x = mPlayPosition.y = mPlayPosition.z = 0.0;
    mPlayTime = 0;
    mVillager = villager;
    mSpeedModifier = speedModifier;
    setRequiredControlFlags(1);
}

bool PlayGoal::canUse()
{
    if (mVillager->getAge() >= 0)
        return false;

    if (mVillager->getRandom()->nextInt(400) != 0)
        return false;

    std::vector<boost::shared_ptr<Entity> >* entities = mVillager->mLevel->getEntitiesOfClass(
        typeid(Villager), mVillager->mBoundingBox->grow(6.0, 3.0, 6.0));
    double closestDistance = DBL_MAX;
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if (entity.get() == mVillager)
            continue;

        boost::shared_ptr<Villager> candidate = boost::dynamic_pointer_cast<Villager>(entity);
        if (candidate->isChasing())
            continue;
        if (candidate->getAge() >= 0)
            continue;

        double distance = candidate->distanceToSqr(mVillager->shared_from_this());
        if (distance > closestDistance)
            continue;

        closestDistance = distance;
        mTarget = boost::weak_ptr<Villager>(candidate);
    }

    delete entities;

    if (!mTarget.lock())
    {
        Vec3* position = RandomPos::getPos(boost::dynamic_pointer_cast<PathfinderMob>(mVillager->shared_from_this()),
                                           16, 3, -1);
        if (position == 0)
            return false;
    }
    return true;
}

bool PlayGoal::canContinueToUse()
{
    return mPlayTime > 0 && mTarget.lock();
}

void PlayGoal::start()
{
    if (mTarget.lock())
        mVillager->setChasing(true);
    mPlayTime = 1000;
}

void PlayGoal::stop()
{
    mVillager->setChasing(false);
    mTarget = boost::weak_ptr<Villager>();
}

void PlayGoal::tick()
{
    --mPlayTime;

    if (mTarget.lock())
    {
        if (mVillager->distanceToSqr(mTarget.lock()) > 4.0)
            mVillager->getNavigation()->moveTo(mTarget.lock(), mSpeedModifier);
    }
    else if (mVillager->getNavigation()->isDone())
    {
        Vec3* position = RandomPos::getPos(boost::dynamic_pointer_cast<PathfinderMob>(mVillager->shared_from_this()),
                                           16, 3, -1);
        if (position == 0)
            return;

        mVillager->getNavigation()->moveTo(position->x, position->y, position->z, mSpeedModifier);
    }
}
