#include "game/entity/ai/BegGoal.hpp"

#include "game/core/InteractionHand.hpp"
#include "game/core/Wolf.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

BegGoal::BegGoal(Wolf* wolf, float distance)
{
    mPlayer = boost::weak_ptr<Player>();
    mTime = 0;
    mWolf = wolf;
    mLevel = wolf->mLevel;
    mDistance = distance;
    setRequiredControlFlags(2);
}

bool BegGoal::playerHoldingInteresting(boost::shared_ptr<Player> player)
{
    for (int hand = InteractionHand::eInteractionHand_MainHand; hand <= InteractionHand::eInteractionHand_OffHand;
         ++hand)
    {
        not_null_ptr<ItemInstance> item = player->getItemInHand((InteractionHand::EInteractionHand)hand);

        if (mWolf->isTame() && item->getItem() == Items::bone)
        {
            return true;
        }

        if (mWolf->isFood(item))
        {
            return true;
        }
    }

    return false;
}

bool BegGoal::canUse()
{
    mPlayer = boost::weak_ptr<Player>(mLevel->getNearestPlayer(mWolf->shared_from_this(), mDistance, -1.0));

    if (!mPlayer.lock())
    {
        return false;
    }

    mWolf->setDespawnProtected();
    return playerHoldingInteresting(mPlayer.lock());
}

bool BegGoal::canContinueToUse()
{
    if (!mPlayer.lock() || !mPlayer.lock()->isAlive())
    {
        return false;
    }

    if (mWolf->distanceToSqr(mPlayer.lock()) > (double)(mDistance * mDistance))
    {
        return false;
    }

    mWolf->setDespawnProtected();
    return mTime > 0 && playerHoldingInteresting(mPlayer.lock());
}

void BegGoal::start()
{
    mWolf->setIsInterested(true);
    mTime = mWolf->getRandom()->nextInt(40) + 40;
}

void BegGoal::stop()
{
    mWolf->setIsInterested(false);
    mPlayer = boost::weak_ptr<Player>();
}

void BegGoal::tick()
{
    mWolf->getLookControl()->setLookAt(mPlayer.lock()->mX, mPlayer.lock()->mY + mPlayer.lock()->getEyeHeight(),
                                       mPlayer.lock()->mZ, 10.0f, (float)mWolf->getMaxHeadXRot());
    --mTime;
}

void BegGoal::setLevel(Level* level)
{
    mLevel = level;
}
