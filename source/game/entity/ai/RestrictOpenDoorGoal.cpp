#include "game/entity/ai/RestrictOpenDoorGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/DoorInfo.hpp"
#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"

RestrictOpenDoorGoal::RestrictOpenDoorGoal(PathfinderMob* mob) : Goal(), mDoorInfo()
{
    mMob = mob;
}

bool RestrictOpenDoorGoal::canUse()
{
    if (mMob->mLevel->isDay())
        return false;

    BlockPos pos(mMob->shared_from_this());
    boost::shared_ptr<Village> village = mMob->mLevel->getVillages()->getClosestVillage(pos, 16);
    if (!village)
        return false;

    boost::shared_ptr<DoorInfo> doorInfo = village->getClosestDoorInfo(pos);
    if (!doorInfo)
        return false;

    mDoorInfo = doorInfo;
    return doorInfo->distanceToInsideSqr(pos) < 2.25;
}

bool RestrictOpenDoorGoal::canContinueToUse()
{
    if (mMob->mLevel->isDay())
        return false;

    boost::shared_ptr<DoorInfo> doorInfo = mDoorInfo.lock();
    if (!doorInfo)
        return false;

    return !doorInfo->isRemoved() && doorInfo->isInsideSide(BlockPos(mMob->shared_from_this()));
}

void RestrictOpenDoorGoal::start()
{
    static_cast<GroundPathNavigation*>(mMob->getNavigation())->setCanOpenDoors(false);
    static_cast<GroundPathNavigation*>(mMob->getNavigation())->setCanPassDoors(false);
}

void RestrictOpenDoorGoal::stop()
{
    static_cast<GroundPathNavigation*>(mMob->getNavigation())->setCanOpenDoors(true);
    static_cast<GroundPathNavigation*>(mMob->getNavigation())->setCanPassDoors(true);
    mDoorInfo = boost::weak_ptr<DoorInfo>();
}

void RestrictOpenDoorGoal::tick()
{
    boost::shared_ptr<DoorInfo> doorInfo = mDoorInfo.lock();
    if (doorInfo)
        doorInfo->incBookingCount();
}
