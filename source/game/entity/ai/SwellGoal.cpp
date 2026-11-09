#include "game/entity/ai/SwellGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Creeper.hpp"
#include "game/core/Sensing.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/LivingEntity.hpp"

SwellGoal::SwellGoal(Creeper* creeper) : mTarget()
{
    mTarget = boost::weak_ptr<LivingEntity>();
    mCreeper = creeper;
    setRequiredControlFlags(1);
}

bool SwellGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mCreeper->getTarget();
    return mCreeper->getSwellDir() > 0 || (target && mCreeper->distanceToSqr(target) < 9.0);
}

void SwellGoal::start()
{
    mCreeper->getNavigation()->stop();
    mTarget = boost::weak_ptr<LivingEntity>(mCreeper->getTarget());
}

void SwellGoal::stop()
{
    mTarget = boost::weak_ptr<LivingEntity>();
}

void SwellGoal::tick()
{
    if (mTarget.lock().get() == NULL)
    {
        mCreeper->setSwellDir(-1);
        return;
    }
    if (mCreeper->distanceToSqr(mTarget.lock()) > 49.0)
    {
        mCreeper->setSwellDir(-1);
        return;
    }
    if (!mCreeper->getSensing()->canSee(mTarget.lock()))
    {
        mCreeper->setSwellDir(-1);
        return;
    }
    mCreeper->setSwellDir(1);
}
