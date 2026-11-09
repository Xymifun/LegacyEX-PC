#include "game/entity/ai/SitGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/TamableAnimal.hpp"

SitGoal::SitGoal(TamableAnimal* mob) : Goal(), mMob(mob), mWantToSit(false)
{
    setRequiredControlFlags(5);
}

SitGoal::~SitGoal()
{
}

bool SitGoal::canUse()
{
    if (!mMob->isTame() || mMob->isInWaterOrBubble() || !mMob->mOnGround)
    {
        return false;
    }

    boost::shared_ptr<LivingEntity> owner = boost::dynamic_pointer_cast<LivingEntity>(mMob->getOwner());

    if (owner == NULL)
    {
        return true;
    }

    if (mMob->distanceToSqr(owner) < 144.0 && owner->getLastHurtByMob() != NULL)
    {
        return false;
    }

    return mWantToSit;
}

void SitGoal::start()
{
    mMob->getNavigation()->stop();
    mMob->setSitting(true);
}

void SitGoal::stop()
{
    mMob->setSitting(false);
}

void SitGoal::wantToSit(bool wantToSit)
{
    mWantToSit = wantToSit;
}
