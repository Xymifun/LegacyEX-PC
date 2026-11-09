#include "game/entity/ai/LandOnOwnersShoulderGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/ShoulderRidingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/generation/Player.hpp"

LandOnOwnersShoulderGoal::LandOnOwnersShoulderGoal(ShoulderRidingEntity* owner)
{
    mOwner = owner;
}

bool LandOnOwnersShoulderGoal::canUse()
{
    boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(mOwner->getOwner());
    bool canLand = owner && !boost::static_pointer_cast<Player>(owner)->isSpectator()
                   && !boost::static_pointer_cast<Player>(owner)->mAbilities.mFlying && !owner->isInWater();

    return !mOwner->isSitting() && canLand && mOwner->canSitOnShoulder();
}

bool LandOnOwnersShoulderGoal::canInterrupt()
{
    return !mShoulderRiding;
}

void LandOnOwnersShoulderGoal::start()
{
    mPlayer = boost::static_pointer_cast<Player>(mOwner->getOwner());
    mShoulderRiding = false;
}

void LandOnOwnersShoulderGoal::tick()
{
    if (!mShoulderRiding && !mOwner->isSitting() && !mOwner->isLeashed())
    {
        boost::shared_ptr<Player> player = mPlayer.lock();
        if (player && mOwner->mBoundingBox->intersects(player->mBoundingBox))
        {
            mShoulderRiding = mOwner->setEntityOnShoulder(player);
        }
    }
}
