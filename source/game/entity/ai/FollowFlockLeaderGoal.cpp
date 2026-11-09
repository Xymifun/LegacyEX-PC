#include "game/entity/ai/FollowFlockLeaderGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

static int sIntervalOffset = 0;

FollowFlockLeaderGoal::FollowFlockLeaderGoal(Mob* mob, bool inWater)
    : Goal(),
      mInterval(++sIntervalOffset + 10),
      mMob(mob),
      mLeader(),
      mTimeToRecalcPath(mInterval),
      mInWater(inWater)
{
    if (sIntervalOffset > 10)
    {
        sIntervalOffset = 0;
    }
}

bool FollowFlockLeaderGoal::canUse()
{
    if (mMob->isLeader() || mMob->followsLeader() || mInWater != mMob->isInWaterOrBubble())
    {
        return false;
    }

    std::vector<boost::shared_ptr<Entity> >* entities = mMob->mLevel->getEntitiesOfClass(typeid(*mMob),
                                                                                         mMob->mBoundingBox->grow(8.0));
    if (entities->size() <= 1)
    {
        delete entities;
        return false;
    }

    boost::shared_ptr<Mob> self = boost::static_pointer_cast<Mob>(mMob->shared_from_this());

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Mob> candidate = boost::static_pointer_cast<Mob>(*it);
        if (candidate->shouldFollow() && candidate != self)
        {
            ++candidate->mFlockSize;
            mLeader = candidate;
            delete entities;
            return true;
        }
    }

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Mob> candidate = boost::static_pointer_cast<Mob>(*it);
        if (candidate != self && !candidate->followsLeader() && !candidate->isLeader())
        {
            candidate->setLeader(true);
            ++candidate->mFlockSize;
            mLeader = candidate;
            delete entities;
            return true;
        }
    }

    delete entities;
    return false;
}

bool FollowFlockLeaderGoal::canContinueToUse()
{
    if (!mLeader || mInWater != mMob->isInWaterOrBubble())
    {
        return false;
    }

    if (!mLeader->isAlive() || !mLeader->isLeader())
    {
        return false;
    }

    double distance = mMob->distanceToSqr(mLeader);
    return distance <= 121.0;
}

void FollowFlockLeaderGoal::start()
{
    mMob->setFollowsLeader(true);
    mTimeToRecalcPath = 0;
}

void FollowFlockLeaderGoal::stop()
{
    mMob->setFollowsLeader(false);
    if (mLeader)
    {
        --mLeader->mFlockSize;
        mLeader = boost::shared_ptr<Mob>();
    }
}

BlockPos FollowFlockLeaderGoal::GetTargetPosition(boost::shared_ptr<Mob> leader)
{
    Vec3* side = leader->getLookAngle()->cross(Vec3::newTemp(0.0, 1.0, 0.0))->normalize();
    Vec3* mobPos = mMob->getPos(0.0f);
    Vec3* delta = mobPos->subtract(leader->getPos(0.0f))->normalize();
    double sideAmount = side->dot(delta);

    Vec3* center = Vec3::newTemp(Mth::floor(leader->getPos(0.0f)->x), Mth::floor(leader->getPos(0.0f)->y),
                                 Mth::floor(leader->getPos(0.0f)->z))
                       ->add(0.5, 0.5, 0.5);
    Vec3* position = center->add(leader->getLookAngle()->multiply(6.0))->add(side->multiply(sideAmount * 3.0));

    BlockPos target(position);
    if (leader->getNavigation()->isRiverFollowing())
    {
        return target;
    }

    return target.offset(0, -1, 0);
}

void FollowFlockLeaderGoal::tick()
{
    if (!mLeader || !mLeader->isMoving())
    {
        mTimeToRecalcPath = mInterval;
        return;
    }

    if (mTimeToRecalcPath - 1 > 0)
    {
        --mTimeToRecalcPath;
        return;
    }
    mTimeToRecalcPath = mInterval;
    boost::shared_ptr<Mob> self = boost::static_pointer_cast<Mob>(mMob->shared_from_this());
    BlockPos target = GetTargetPosition(mLeader);
    self->getNavigation()->moveTo((double)target.x, (double)target.y, (double)target.z, 1.0);
}
