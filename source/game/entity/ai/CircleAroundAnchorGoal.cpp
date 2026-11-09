#include "game/entity/ai/CircleAroundAnchorGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Mth.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

CircleAroundAnchorGoal::CircleAroundAnchorGoal(Mob* mob, float speedModifier, Goal::FloatRange distanceRange,
                                               int distanceInterval, Goal::FloatRange heightRange,
                                               Goal::FloatRange verticalRange, int verticalInterval, float minDistance)
    : Goal(),
      mMob(mob),
      mSpeedModifier(speedModifier),
      mDistanceRange(distanceRange),
      mDistanceInterval(distanceInterval),
      mHeightRange(heightRange),
      mVerticalRange(verticalRange),
      mVerticalInterval(verticalInterval),
      mMinDistanceSquared(minDistance * minDistance),
      mAnchor(NULL),
      mCurrentDistance(0.0f),
      mCurrentHeight(0.0f),
      mDirection(1.0f),
      mAngle(0.0f)
{
    setRequiredControlFlags(3);
}

CircleAroundAnchorGoal::~CircleAroundAnchorGoal()
{
    delete mAnchor;
}

float CircleAroundAnchorGoal::calculateHeightOffset() const
{
    float height = mVerticalRange.min;
    if (mVerticalRange.max - height > 0.0f)
    {
        height += mMob->mLevel->mRandom->nextFloat(mVerticalRange.max - height);
    }
    return height;
}

void CircleAroundAnchorGoal::setAnchorAboveTarget()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
    {
        return;
    }

    Vec3* position = target->getPos(0.0f)->add(0.0, (double)mHeightRange.min, 0.0);
    float range = mHeightRange.max - mHeightRange.min;
    if (range > 0.0f)
    {
        position = position->add(0.0, (double)mMob->mLevel->mRandom->nextFloat(range), 0.0);
    }
    int seaLevel = mMob->mLevel->mSeaLevel;
    if (position->y <= (double)seaLevel)
    {
        position->y = (double)(seaLevel + 1);
    }
    delete mAnchor;
    mAnchor = Vec3::newPermanent(position);
}

void CircleAroundAnchorGoal::selectNext()
{
    Vec3* anchor = mAnchor;
    if (anchor == NULL)
    {
        anchor = Vec3::newPermanent(mMob->getPos(0.0f));
        mAnchor = anchor;
        if (anchor->y + 20.0 < 256.0)
        {
            anchor->y += 20.0;
        }
        else
        {
            anchor->y = 236.0;
        }
    }

    mAngle += mDirection * 30.0f * Mth::DEG_TO_RAD;
    Vec3* wanted = anchor->add((double)(mCurrentDistance * Mth::cos(mAngle)), (double)mCurrentHeight,
                               (double)(mCurrentDistance * Mth::sin(mAngle)));
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
    {
        navigation->moveTo(wanted->x, wanted->y, wanted->z, mSpeedModifier);
    }
    else
    {
        mMob->getMoveControl()->setWantedPosition(wanted->x, wanted->y, wanted->z, mSpeedModifier);
    }
}

void CircleAroundAnchorGoal::start()
{
    Random* random = mMob->mLevel->mRandom;
    mCurrentDistance = mDistanceRange.min + random->nextFloat() * (mDistanceRange.max - mDistanceRange.min);
    mCurrentHeight = calculateHeightOffset();
    mDirection = random->nextBoolean() ? 1.0f : -1.0f;
    if (mMob->getTarget())
    {
        setAnchorAboveTarget();
    }
    selectNext();
}

void CircleAroundAnchorGoal::stop()
{
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
    {
        navigation->stop();
    }
    else
    {
        mMob->getMoveControl()->wait();
    }
}

bool CircleAroundAnchorGoal::canUse()
{
    return true;
}

bool CircleAroundAnchorGoal::touchingTarget() const
{
    PathNavigation* navigation = mMob->getNavigation();
    if (navigation != NULL)
    {
        return navigation->isDone();
    }

    MoveControl* moveControl = mMob->getMoveControl();
    Vec3* position = mMob->getPos(0.0f);
    Vec3* wanted = Vec3::newTemp(moveControl->getWantedX(), moveControl->getWantedY(), moveControl->getWantedZ());
    return wanted->distanceToSqr(position) < (double)mMinDistanceSquared;
}

void CircleAroundAnchorGoal::tick()
{
    Random* random = mMob->mLevel->mRandom;
    if (mMob->getTarget())
    {
        setAnchorAboveTarget();
    }

    if (mVerticalRange.max > mVerticalRange.min && (mVerticalInterval == 0 || random->nextInt(mVerticalInterval) == 0))
    {
        mCurrentHeight = calculateHeightOffset();
    }

    if (mDistanceRange.max > mDistanceRange.min && (mDistanceInterval == 0 || random->nextInt(mDistanceInterval) == 0))
    {
        if (mCurrentDistance + 1.0f <= mDistanceRange.max)
        {
            mCurrentDistance += 1.0f;
        }
        else
        {
            mCurrentDistance = mDistanceRange.min;
            mDirection = -mDirection;
        }
    }

    Vec3* position = mMob->getPos(0.0f);
    double distanceToWanted = phys::Vec2::newTemp((float)mMob->getMoveControl()->getWantedX(),
                                                  (float)mMob->getMoveControl()->getWantedZ())
                                  ->distanceToSqr(phys::Vec2::newTemp((float)position->x, (float)position->z));

    if (distanceToWanted < (double)mMinDistanceSquared)
    {
        if (mMob->getMoveControl()->getWantedY() < mMob->getPos(0.0f)->y)
        {
            if (!mMob->mLevel->isEmptyBlock(BlockPos(mMob->getPos(0.0f)).below(1)))
            {
                mCurrentHeight = mCurrentHeight > 1.0f ? mCurrentHeight : 1.0f;
                selectNext();
            }
        }
    }

    if (distanceToWanted < (double)mMinDistanceSquared)
    {
        if (mMob->getMoveControl()->getWantedY() > mMob->getPos(0.0f)->y)
        {
            if (!mMob->mLevel->isEmptyBlock(BlockPos(mMob->getPos(0.0f)).above(1)))
            {
                mCurrentHeight = mCurrentHeight < -1.0f ? mCurrentHeight : -1.0f;
                selectNext();
            }
        }
    }

    if (touchingTarget())
    {
        selectNext();
    }
}
