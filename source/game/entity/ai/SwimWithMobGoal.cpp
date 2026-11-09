#include "game/entity/ai/SwimWithMobGoal.hpp"

#include <vector>

#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

SwimWithMobGoal::SwimWithMobGoal(Mob* mob, float speedModifier, float maxDistance, int searchDistance,
                                 arrayWithLength<unsigned long long> entityTypes)
    : mWantedMob(),
      mCooldown(0),
      mState(0),
      mEntityTypes(entityTypes),
      mMob(mob),
      mSpeedModifier(speedModifier),
      mMaxDistance(maxDistance * maxDistance),
      mSearchDistance(searchDistance)
{
    setRequiredControlFlags(3);
}

SwimWithMobGoal::~SwimWithMobGoal()
{
    delete[] mEntityTypes.data;
}

bool SwimWithMobGoal::canSwimWith(boost::shared_ptr<Entity> entity)
{
    if (mEntityTypes.length == 0)
        return true;

    unsigned long long* it;
    for (it = mEntityTypes.data; it != mEntityTypes.data + mEntityTypes.length; ++it)
    {
        if (entity->instanceof(*it))
            return true;
    }

    return false;
}

bool SwimWithMobGoal::setWantedMob()
{
    std::vector<boost::shared_ptr<Entity> >* entities = mMob->mLevel->getEntities(
        mMob->shared_from_this(), mMob->mBoundingBox->grow((double)mSearchDistance), NULL);
    std::vector<boost::shared_ptr<Entity> >* candidates = new std::vector<boost::shared_ptr<Entity> >;

    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if (entity)
        {
            double speedSqr = entity->mXd * entity->mXd + entity->mYd * entity->mYd + entity->mZd * entity->mZd;
            bool ridingAndMoving = entity->isPassenger() && speedSqr > 0.010000000000000002;
            if (entity->isAlive() && (ridingAndMoving || entity->isSwimming()) && canSwimWith(entity))
            {
                candidates->push_back(entity);
            }
        }
    }

    if (!candidates->empty())
    {
        mWantedMob = candidates->at(mMob->getRandom()->nextInt((int)candidates->size()));
        delete candidates;
        return true;
    }

    delete candidates;
    return false;
}

bool SwimWithMobGoal::canUse()
{
    if (mMob->getNavigation() != NULL && mMob->mLevel->mRandom->nextInt(10) == 0)
        return setWantedMob();

    return false;
}

bool SwimWithMobGoal::canContinueToUse()
{
    boost::shared_ptr<Entity> wantedMob = mWantedMob.lock();
    if (!wantedMob || !wantedMob->isAlive())
        return false;

    bool ridingAndMoving = wantedMob->mXd * wantedMob->mXd + wantedMob->mYd * wantedMob->mYd
                                   + wantedMob->mZd * wantedMob->mZd
                               > 0.010000000000000002
                           && wantedMob->isPassenger();
    if (!ridingAndMoving && !wantedMob->isSwimming() && wantedMob->mLevel->mRandom->nextInt(30) == 0)
    {
        return false;
    }

    return mMob->distanceToSqr(wantedMob) < (double)mMaxDistance;
}

bool SwimWithMobGoal::canInterrupt()
{
    return true;
}

void SwimWithMobGoal::start()
{
    mCooldown = 0;
    mState = 0;
}

void SwimWithMobGoal::stop()
{
    mWantedMob.reset();
}

BlockPos SwimWithMobGoal::GetTargetPosition(boost::shared_ptr<Entity> entity, Vec3* direction, double distance,
                                            double offset)
{
    Vec3* position = Vec3::newTemp((double)Mth::floor(entity->getPos(0.0f)->x),
                                   (double)Mth::floor(entity->getPos(0.0f)->y),
                                   (double)Mth::floor(entity->getPos(0.0f)->z));
    position = position->add(0.5, 0.5, 0.5);
    position = position->add(entity->getLookAngle()->multiply(offset))->add(direction->multiply(distance * 3.0));
    BlockPos target = BlockPos(position).offset(0, -1, 0);
    return target;
}

void SwimWithMobGoal::tick()
{
    boost::shared_ptr<Entity> wantedMob = mWantedMob.lock();
    if (!wantedMob || !wantedMob->isAlive())
        return;

    float speed = mSpeedModifier;
    if (mState != 1)
        speed *= 3.0f;

    if (--mCooldown > 0 && !mMob->getNavigation()->isDone())
        return;

    mCooldown = 10;

    Vec3* direction = wantedMob->getLookAngle()->cross(Vec3::newTemp(0.0, 1.0, 0.0))->normalize();
    Vec3* mobPos = mMob->getPos(0.0f);
    Vec3* delta = mobPos->subtract(wantedMob->getPos(0.0f))->normalize();
    double side = direction->dot(delta);
    if (side <= 2.220446049250313e-16 && side >= -2.220446049250313e-16)
        side = 0.2;

    if (mState == 0)
    {
        if (mMob->distanceToSqr(wantedMob) < 6.25)
        {
            mMob->getNavigation()->stop();
            mCooldown = 0;
            mState = 1;
        }
        else
        {
            BlockPos target = GetTargetPosition(wantedMob, direction, side, 2.0);
            mMob->getNavigation()->moveTo((double)target.x, (double)target.y, (double)target.z, (double)(speed + 1.0f));
        }
    }
    else if (mState == 1)
    {
        if (mMob->distanceToSqr(wantedMob) > 144.0)
        {
            mMob->getNavigation()->stop();
            mCooldown = 0;
            mState = 0;
        }
        else
        {
            BlockPos target = GetTargetPosition(wantedMob, direction, side, 6.0);
            mMob->getNavigation()->moveTo((double)target.x, (double)target.y, (double)target.z, (double)(speed + 1.0f));
        }
    }
}
