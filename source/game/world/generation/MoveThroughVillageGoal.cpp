#include "game/world/generation/MoveThroughVillageGoal.hpp"

#include <climits>

#include "game/core/Mth.hpp"
#include "game/core/Path.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"
#include "game/world/block/BlockPos.hpp"

MoveThroughVillageGoal::MoveThroughVillageGoal(PathfinderMob* mob, double speedModifier, bool onlyAtNight)
    : Goal(),
      mDoorInfo(),
      mVisited()
{
    mPath = NULL;
    mDoorInfo = boost::weak_ptr<DoorInfo>();
    mMob = mob;
    mSpeedModifier = speedModifier;
    mOnlyAtNight = onlyAtNight;
    setRequiredControlFlags(1);
}

MoveThroughVillageGoal::~MoveThroughVillageGoal()
{
    delete mPath;
}

void MoveThroughVillageGoal::updateVisited()
{
    if (mVisited.size() > 15)
        mVisited.erase(mVisited.begin());
}

bool MoveThroughVillageGoal::hasVisited(boost::shared_ptr<DoorInfo> door)
{
    for (std::vector<boost::weak_ptr<DoorInfo> >::iterator it = mVisited.begin(); it != mVisited.end();)
    {
        boost::shared_ptr<DoorInfo> visited = it->lock();
        if (visited == NULL)
        {
            it = mVisited.erase(it);
        }
        else
        {
            if (door->getPos().equals(visited->getPos()))
                return true;
            ++it;
        }
    }
    return false;
}

boost::shared_ptr<DoorInfo> MoveThroughVillageGoal::getNextDoorInfo(boost::shared_ptr<Village> village)
{
    boost::shared_ptr<DoorInfo> result;
    int closestDistance = INT_MAX;
    std::vector<boost::shared_ptr<DoorInfo> >* doors = village->getDoorInfos();
    for (std::vector<boost::shared_ptr<DoorInfo> >::iterator it = doors->begin(); it != doors->end(); it++)
    {
        boost::shared_ptr<DoorInfo> door = *it;
        int distance = door->distanceToSqr(Mth::floor(mMob->mX), Mth::floor(mMob->mY), Mth::floor(mMob->mZ));
        if (distance < closestDistance)
        {
            if (hasVisited(door))
                continue;
            closestDistance = distance;
            result = door;
        }
    }
    return result;
}

bool MoveThroughVillageGoal::canUse()
{
    updateVisited();
    if (mOnlyAtNight && mMob->mLevel->isDay())
        return false;

    boost::shared_ptr<Village> village = mMob->mLevel->getVillages()->getClosestVillage(
        BlockPos(mMob->shared_from_this()), 0);
    if (!village)
        return false;

    boost::shared_ptr<DoorInfo> doorInfo = getNextDoorInfo(village);
    if (!doorInfo)
        return false;

    mDoorInfo = doorInfo;

    PathNavigation* navigation = mMob->getNavigation();
    GroundPathNavigation* groundNavigation = static_cast<GroundPathNavigation*>(navigation);
    bool canOpenDoors = groundNavigation->canOpenDoors();
    groundNavigation->setCanOpenDoors(false);
    delete mPath;
    mPath = navigation->createPath(doorInfo->getPos());
    groundNavigation->setCanOpenDoors(canOpenDoors);
    if (mPath != NULL)
        return true;

    Vec3* position = RandomPos::getPosTowards(
        boost::static_pointer_cast<PathfinderMob>(mMob->shared_from_this()), 10, 7,
        Vec3::newTemp((double)doorInfo->getPos().x, (double)doorInfo->getPos().y, (double)doorInfo->getPos().z));
    if (position == NULL)
        return false;

    groundNavigation->setCanOpenDoors(false);
    delete mPath;
    mPath = mMob->getNavigation()->createPath(position->x, position->y, position->z);
    groundNavigation->setCanOpenDoors(canOpenDoors);
    return mPath != NULL;
}

bool MoveThroughVillageGoal::canContinueToUse()
{
    if (mMob->getNavigation()->isDone())
        return false;

    float limit = mMob->mBbWidth + 4.0f;
    boost::shared_ptr<DoorInfo> doorInfo = mDoorInfo.lock();
    if (!doorInfo)
        return false;

    return mMob->distanceToSqr(doorInfo->getPos()) > (double)(limit * limit);
}

void MoveThroughVillageGoal::start()
{
    mMob->getNavigation()->moveTo(mPath, mSpeedModifier);
    mPath = NULL;
}

void MoveThroughVillageGoal::stop()
{
    boost::shared_ptr<DoorInfo> doorInfo = mDoorInfo.lock();
    if (!doorInfo)
        return;

    if (mMob->getNavigation()->isDone() || mMob->distanceToSqr(doorInfo->getPos()) < 16.0)
        mVisited.push_back(mDoorInfo);
}
