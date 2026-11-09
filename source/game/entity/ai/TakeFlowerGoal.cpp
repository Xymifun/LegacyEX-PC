#include "game/entity/ai/TakeFlowerGoal.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"

TakeFlowerGoal::TakeFlowerGoal(Villager* villager) : mGolem(), mWantsToTakeFlower(false)
{
    mTicks = 0;
    mGolem = boost::weak_ptr<VillagerGolem>();
    mVillager = villager;
    setRequiredControlFlags(3);
}

bool TakeFlowerGoal::canUse()
{
    if (mVillager->getAge() >= 0 || !mVillager->mLevel->isDay())
    {
        return false;
    }

    std::vector<boost::shared_ptr<Entity> >* entities = mVillager->mLevel->getEntitiesOfClass(
        typeid(VillagerGolem), mVillager->mBoundingBox->grow(6.0, 2.0, 6.0));

    if (entities->size() == 0)
    {
        delete entities;
        return false;
    }

    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<VillagerGolem> golem = boost::dynamic_pointer_cast<VillagerGolem>(*it);
        if (golem->getOfferFlowerTick() > 0)
        {
            mGolem = boost::weak_ptr<VillagerGolem>(golem);
            break;
        }
    }

    delete entities;
    return mGolem.lock().get() != NULL;
}

bool TakeFlowerGoal::canContinueToUse()
{
    return mGolem.lock() && mGolem.lock()->getOfferFlowerTick() > 0;
}

void TakeFlowerGoal::start()
{
    mTicks = mVillager->getRandom()->nextInt(320);
    mWantsToTakeFlower = false;
    mGolem.lock()->getNavigation()->stop();
}

void TakeFlowerGoal::stop()
{
    mGolem = boost::weak_ptr<VillagerGolem>();
    mVillager->getNavigation()->stop();
}

void TakeFlowerGoal::tick()
{
    mVillager->getLookControl()->setLookAt(mGolem.lock(), 30.0f, 30.0f);

    if (mGolem.lock()->getOfferFlowerTick() == mTicks)
    {
        mVillager->getNavigation()->moveTo(mGolem.lock(), 0.5);
        mWantsToTakeFlower = true;
    }

    if (mWantsToTakeFlower)
    {
        if (mVillager->distanceToSqr(mGolem.lock()) < 4.0)
        {
            mGolem.lock()->offerFlower(false);
            mVillager->getNavigation()->stop();
        }
    }
}
