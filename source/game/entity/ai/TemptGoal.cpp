#include "game/entity/ai/TemptGoal.hpp"

#include <algorithm>
#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

TemptGoal::TemptGoal(PathfinderMob* mob, double speedModifier, arrayWithLength<Item*> items, bool canScare,
                     arrayWithLength<int> itemAuxValues)
{
    mTargetX = mTargetY = mTargetZ = mTargetXRot = mTargetYRot = 0.0;
    mPlayer = boost::weak_ptr<Player>();
    mField0x61 = false;
    mRunning = false;
    mSpeedModifier = speedModifier;
    mMob = mob;
    mDelayTemptCounter = 0;
    mItems = items;
    mItemAuxValues = itemAuxValues;
    mCanScare = canScare;
    setRequiredControlFlags(3);
}

bool TemptGoal::shouldFollowItem(not_null_ptr<ItemInstance> item)
{
    bool follow = false;
    for (unsigned int i = 0; i < mItems.length; ++i)
    {
        if (item->getItem() == mItems.data[i]
            && (mItemAuxValues.length == 0 || item->getAuxValue() == mItemAuxValues.data[i]))
        {
            follow = true;
            break;
        }
    }

    return follow;
}

bool TemptGoal::canUse()
{
    if (mDelayTemptCounter > 0)
    {
        --mDelayTemptCounter;
        return false;
    }

    mPlayer = boost::weak_ptr<Player>(mMob->mLevel->getNearestPlayer(mMob->shared_from_this(), 10.0, -1.0));
    if (!mPlayer.lock())
    {
        return false;
    }

    mMob->setDespawnProtected();
    return shouldFollowItem(mPlayer.lock()->getMainHandItem()) || shouldFollowItem(mPlayer.lock()->getOffhandItem());
}

bool TemptGoal::canContinueToUse()
{
    if (mCanScare)
    {
        boost::shared_ptr<Player> player = mPlayer.lock();
        if (!player)
        {
            return false;
        }

        if (mMob->distanceToSqr(player) < 36.0)
        {
            if (player->distanceToSqr(mTargetX, mTargetY, mTargetZ) > 0.010000000000000002)
            {
                mDelayTemptCounter = 100;
                return false;
            }

            double xRotDelta = fabs((double)player->mXRot - mTargetXRot);
            double yRotDelta = fabs((double)player->mYRot - mTargetYRot);
            if (std::min(xRotDelta, 360.0 - xRotDelta) > 5.0 || std::min(yRotDelta, 360.0 - yRotDelta) > 5.0)
            {
                mDelayTemptCounter = 100;
                return false;
            }
        }
        else
        {
            mTargetX = player->mX;
            mTargetY = player->mY;
            mTargetZ = player->mZ;
        }

        mTargetXRot = (double)player->mXRot;
        mTargetYRot = (double)player->mYRot;
    }

    return canUse();
}

void TemptGoal::start()
{
    boost::shared_ptr<Player> player = mPlayer.lock();
    mTargetX = player->mX;
    mTargetY = player->mY;
    mTargetZ = player->mZ;
    mRunning = true;
    mTargetXRot = (double)player->mXRot;
    mTargetYRot = (double)player->mYRot;
}

void TemptGoal::stop()
{
    mPlayer = boost::weak_ptr<Player>();
    mMob->getNavigation()->stop();
    mRunning = false;
}

void TemptGoal::tick()
{
    mMob->getLookControl()->setLookAt(mPlayer.lock(), (float)mMob->getMaxHeadYRot() + 20.0f,
                                      (float)mMob->getMaxHeadXRot());

    if (mMob->distanceToSqr(mPlayer.lock()) < 6.25)
    {
        mMob->getNavigation()->stop();
    }
    else
    {
        mMob->getNavigation()->moveTo(mPlayer.lock(), mSpeedModifier);
    }
}

bool TemptGoal::isRunning()
{
    return mRunning;
}
