#include "game/entity/ai/OfferFlowerGoal.hpp"

#include <boost/shared_ptr.hpp>
#include <string>
#include <typeinfo>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"

OfferFlowerGoal::OfferFlowerGoal(VillagerGolem* golem)
{
    mTicks = 0;
    mGolem = golem;
    setRequiredControlFlags(3);
}

bool OfferFlowerGoal::canUse()
{
    if (!mGolem->mLevel->isDay())
        return false;
    if (mGolem->getRandom()->nextInt(8000) != 0)
        return false;

    VillagerGolem* golem = mGolem;
    mTarget = boost::weak_ptr<Villager>(boost::dynamic_pointer_cast<Villager>(mGolem->mLevel->getClosestEntityOfClass(
        typeid(Villager), golem->mBoundingBox->grow(6.0, 2.0, 6.0), golem->shared_from_this())));
    return !!mTarget.lock();
}

bool OfferFlowerGoal::canContinueToUse()
{
    return mTicks > 0 && mTarget.lock();
}

void OfferFlowerGoal::start()
{
    mTicks = 400;
    mGolem->offerFlower(true);
}

void OfferFlowerGoal::stop()
{
    mGolem->offerFlower(false);
    mTarget = boost::weak_ptr<Villager>();
}

void OfferFlowerGoal::tick()
{
    mGolem->getLookControl()->setLookAt(mTarget.lock(), 30.0f, 30.0f);
    --mTicks;
}
