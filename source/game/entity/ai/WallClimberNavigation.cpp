#include "game/entity/ai/WallClimberNavigation.hpp"

#include <cmath>

#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

WallClimberNavigation::WallClimberNavigation(Mob* mob, Level* level)
    : GroundPathNavigation(mob, level, false),
      mTargetPos(0)
{
    mPathFinder = GroundPathNavigation::createPathFinder();
}

WallClimberNavigation::~WallClimberNavigation()
{
    delete mTargetPos;
}

void WallClimberNavigation::setPosition(BlockPos* pos)
{
    delete mTargetPos;
    mTargetPos = pos;
}

Path* WallClimberNavigation::createPath(const BlockPos& pos)
{
    setPosition(new BlockPos(pos));
    return GroundPathNavigation::createPath(pos);
}

Path* WallClimberNavigation::createPath(boost::shared_ptr<Entity> entity)
{
    setPosition(new BlockPos(entity));
    return GroundPathNavigation::createPath(entity);
}

bool WallClimberNavigation::moveTo(boost::shared_ptr<Entity> entity, double speed)
{
    Path* path = createPath(entity);
    if (path != 0)
    {
        return moveTo(path, speed);
    }
    else
    {
        setPosition(new BlockPos(entity));
        mSpeedModifier = speed;
        return true;
    }
}

void WallClimberNavigation::tick()
{
    if (isDone())
    {
        if (mTargetPos != 0)
        {
            double widthSquared = mMob->mBbWidth * mMob->mBbWidth;

            if (mMob->distanceSqrToBlockPosCenter(*mTargetPos) < widthSquared
                || (mMob->mY > mTargetPos->getY()
                    && mMob->distanceSqrToBlockPosCenter(
                           BlockPos(mTargetPos->getX(), (int)std::floor(mMob->mY), mTargetPos->getZ()))
                           < widthSquared))
            {
                mTargetPos = 0;
            }
            else
            {
                mMob->getMoveControl()->setWantedPosition(mTargetPos->getX(), mTargetPos->getY(), mTargetPos->getZ(),
                                                          mSpeedModifier);
            }
        }
    }
    else
    {
        PathNavigation::tick();
    }
}
