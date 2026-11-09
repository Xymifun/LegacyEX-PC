#include "game/entity/ai/RunAroundLikeCrazyGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/EntityType.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/world/Level.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/Player.hpp"

RunAroundLikeCrazyGoal::RunAroundLikeCrazyGoal(AbstractHorse* horse, double speedModifier)
    : mHorse(horse),
      mSpeedModifier(speedModifier)
{
    mWantedX = mWantedY = mWantedZ = 0.0;
    setRequiredControlFlags(1);
}

RunAroundLikeCrazyGoal::~RunAroundLikeCrazyGoal()
{
}

bool RunAroundLikeCrazyGoal::canUse()
{
    if (mHorse->isTamed() || !mHorse->isVehicle())
    {
        return false;
    }

    bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, mHorse->getControllingPassenger()->GetType());
    if (!isPlayer)
    {
        return false;
    }

    Vec3* target = RandomPos::getPos(boost::static_pointer_cast<PathfinderMob>(mHorse->shared_from_this()), 5, 4, -1);
    if (target == NULL)
    {
        return false;
    }

    mWantedX = target->x;
    mWantedY = target->y;
    mWantedZ = target->z;
    return true;
}

void RunAroundLikeCrazyGoal::start()
{
    mHorse->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}

bool RunAroundLikeCrazyGoal::canContinueToUse()
{
    return !mHorse->isTamed() && !mHorse->getNavigation()->isDone() && mHorse->isVehicle();
}

void RunAroundLikeCrazyGoal::tick()
{
    if (mHorse->isTamed() || mHorse->getRandom()->nextInt(50) != 0)
    {
        return;
    }

    boost::shared_ptr<Entity> passenger = mHorse->getControllingPassenger();
    if (!passenger)
    {
        return;
    }

    bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, passenger->GetType());
    if (isPlayer)
    {
        int temper = mHorse->getTemper();
        int maxTemper = mHorse->getMaxTemper();
        if (maxTemper > 0 && mHorse->getRandom()->nextInt(maxTemper) < temper)
        {
            mHorse->tameWithName(boost::static_pointer_cast<Player>(passenger));
            return;
        }

        mHorse->modifyTemper(5);
    }

    mHorse->ejectPassengers();
    mHorse->makeMad();
    mHorse->mLevel->broadcastEntityEvent(mHorse->shared_from_this(), EntityEvent::TAMING_FAILED, 0);
}
