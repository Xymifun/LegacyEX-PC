#include "game/entity/ai/MakeLoveGoal.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Village.hpp"
#include "game/world/generation/Villages.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/block/BlockPos.hpp"

MakeLoveGoal::MakeLoveGoal(Villager* villager)
{
    mVillage = boost::weak_ptr<Village>();
    mPartner = boost::weak_ptr<Villager>();
    mRunningTicks = 0;
    mVillager = villager;
    mLevel = villager->mLevel;
    setRequiredControlFlags(3);
}

bool MakeLoveGoal::villageNeedsMoreVillagers()
{
    boost::shared_ptr<Village> village = mVillage.lock();
    if (village == NULL)
        return false;

    if (!village->isBreedTimerOk())
        return false;

    int maxPopulation = (int)((float)village->getDoorCount() * 0.35);
    return village->getPopulationSize() < maxPopulation;
}

bool MakeLoveGoal::canUse()
{
    if (mVillager->getAge() != 0)
        return false;

    if (mVillager->getRandom()->nextInt(500) != 0)
        return false;

    mVillage = mLevel->getVillages()->getClosestVillage(BlockPos(mVillager->shared_from_this()), 0);
    if (!mVillage.lock())
        return false;

    if (!villageNeedsMoreVillagers() || !mVillager->isWillingToBreed(true))
        return false;

    Villager* villager = mVillager;
    boost::shared_ptr<Entity> entity = mLevel->getClosestEntityOfClass(
        typeid(Villager), villager->mBoundingBox->grow(8.0, 3.0, 8.0), villager->shared_from_this());
    if (!entity)
        return false;

    mPartner = boost::weak_ptr<Villager>(boost::dynamic_pointer_cast<Villager>(entity));
    if (mPartner.lock()->getAge() != 0 || !mPartner.lock()->isWillingToBreed(true))
        return false;

    return true;
}

void MakeLoveGoal::start()
{
    mRunningTicks = 300;
    mVillager->setInLove(true);
}

void MakeLoveGoal::stop()
{
    mVillage = boost::weak_ptr<Village>();
    mPartner = boost::weak_ptr<Villager>();
    mVillager->setInLove(false);
}

bool MakeLoveGoal::canContinueToUse()
{
    return mPartner.lock() && mRunningTicks > -1 && villageNeedsMoreVillagers() && mVillager->getAge() == 0
           && mVillager->isWillingToBreed(false);
}

void MakeLoveGoal::breed()
{
    mPartner.lock()->setAge(6000);
    mVillager->setAge(6000);

    if (mLevel->canCreateMore(EntityType::VILLAGER, Level::eSpawnType_Breed))
    {
        boost::shared_ptr<Villager> villager = boost::dynamic_pointer_cast<Villager>(
            mVillager->getBreedOffspring(mPartner.lock()));

        mPartner.lock()->setWillingToBreed(false);
        mVillager->setWillingToBreed(false);
        villager->setAge(-24000);
        villager->moveTo(mVillager->mX, mVillager->mY, mVillager->mZ, 0.0f, 0.0f);
        mLevel->addEntity(villager);
        mLevel->broadcastEntityEvent(villager, EntityEvent::LOVE_HEARTS, 0);
    }
}

void MakeLoveGoal::tick()
{
    --mRunningTicks;

    mVillager->getLookControl()->setLookAt(mPartner.lock(), 10.0F, 30.0F);
    if (mVillager->distanceToSqr(mPartner.lock()) > 2.25)
    {
        mVillager->getNavigation()->moveTo(mPartner.lock(), 0.25);
    }
    else if (mRunningTicks == 0 && mPartner.lock()->isInLove())
    {
        breed();
    }

    if (mVillager->getRandom()->nextInt(35) == 0)
        mLevel->broadcastEntityEvent(mVillager->shared_from_this(), EntityEvent::LOVE_HEARTS, 0);
}

void MakeLoveGoal::setLevel(Level* level)
{
    mLevel = level;
}
