#include "game/entity/ai/FindMountGoal.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"

FindMountGoal::FindMountGoal(Mob& mob, float targetDistance, int startDelay, bool avoidWater, bool targetNeeded,
                             float mountDistance)
    : mTarget(),
      mTargetDistance(targetDistance),
      mStartCounter(0),
      mStartDelay(startDelay),
      mAvoidWater(avoidWater),
      mTargetNeeded(targetNeeded),
      mMountDistance(mountDistance),
      mMob(&mob)
{
    setRequiredControlFlags(3);
}

bool FindMountGoal::canUse()
{
    if (!static_cast<Zombie*>(mMob)->isJockey())
        return false;

    if (!mMob->getNavigation() || mMob->isPassenger())
        return false;

    boost::shared_ptr<Entity> attackTarget = mMob->getTarget();
    if (mTargetNeeded && (!attackTarget || mMob->isPassenger()))
        return false;

    if (mStartCounter < mStartDelay)
    {
        ++mStartCounter;
        return false;
    }

    std::vector<boost::shared_ptr<Entity> >* entities = mMob->mLevel->getEntitiesOfClass(
        typeid(Mob), mMob->mBoundingBox->grow((double)mTargetDistance));

    static const unsigned long long mountTypes[] = {
        EntityType::CHICKEN,        EntityType::CAVE_SPIDER, EntityType::COW,          EntityType::DONKEY,
        EntityType::HORSE,          EntityType::HUSK,        EntityType::MUSHROOM_COW, EntityType::MULE,
        EntityType::OCELOT,         EntityType::PIG,         EntityType::SHEEP,        EntityType::SKELETON_HORSE,
        EntityType::SPIDER,         EntityType::WOLF,        EntityType::ZOMBIE,       EntityType::ZOMBIE_HORSE,
        EntityType::ZOMBIE_VILLAGER};
    double nearestDistance = -1.0;
    for (unsigned int i = 0; i < entities->size(); ++i)
    {
        boost::shared_ptr<Entity> candidate = entities->at(i);
        if (eTYPE_DERIVED_FROM(EntityType::MOB, candidate->GetType()) && mMob->getId() != candidate->getId())
        {
            boost::shared_ptr<Mob> mount = boost::static_pointer_cast<Mob>(candidate);
            boost::shared_ptr<Entity> self = mMob->shared_from_this();
            if (mount->isAlive() && mount->canAddPassenger(self) && !mount->isSleeping())
            {
                if ((eTYPE_DERIVED_FROM(EntityType::TAMABLE_ANIMAL, mount->GetType())
                     && boost::static_pointer_cast<TamableAnimal>(mount)->isTame())
                    || (eTYPE_DERIVED_FROM(EntityType::ABSTRACT_HORSE, mount->GetType())
                        && boost::static_pointer_cast<AbstractHorse>(mount)->isTamed())
                    || mount->isBaby())
                    continue;
                if (mAvoidWater && mount->isInWater())
                    continue;

                bool mountType = false;
                for (unsigned int index = 0; index < sizeof(mountTypes) / sizeof(mountTypes[0]); ++index)
                {
                    if (mount->GetType() == mountTypes[index])
                    {
                        mountType = true;
                        break;
                    }
                }
                if (!mountType)
                    continue;

                double distance = mount->distanceToSqr(self);
                if (nearestDistance < 0.0 || distance < nearestDistance)
                {
                    mTarget = mount;
                    nearestDistance = distance;
                }
            }
        }
    }

    delete entities;
    boost::shared_ptr<Entity> target = mTarget.lock();
    if (target)
    {
        mTimeToRecalcPath = mMob->mLevel->mRandom->nextInt(7) + 4;
        return mMob->getNavigation()->moveTo(target, 1.0);
    }
    return false;
}

bool FindMountGoal::canContinueToUse()
{
    boost::shared_ptr<Entity> target = mTarget.lock();
    if (target && target->isAlive() && !mMob->isPassenger() && target->canAddPassenger(mMob->shared_from_this()))
    {
        return true;
    }
    mMob->setTarget(boost::shared_ptr<LivingEntity>());
    return false;
}

void FindMountGoal::start()
{
}

void FindMountGoal::stop()
{
    mMob->getNavigation()->stop();
    mTarget.reset();
    mStartCounter = 0;
}

float FindMountGoal::getAttackReachSqr()
{
    float reach = mMob->mBbWidth + mMob->mBbWidth + mTarget.lock()->mBbWidth + 2.0f;
    return reach * reach;
}

void FindMountGoal::tick()
{
    boost::shared_ptr<Entity> target = mTarget.lock();
    if (!target)
        return;

    mMob->getLookControl()->setLookAt(target, 30.0f, 30.0f);
    double mountRange = (double)mMountDistance;
    if (mMountDistance < 0.0f)
        mountRange = getAttackReachSqr();

    Vec3* ridingPos = target->getPos(0.0f)->add(0.0, target->getRidingHeight(), 0.0);
    if (mMob != target.get() && mMob->distanceToSqr(BlockPos(ridingPos)) < mountRange && target->isAlive())
    {
        static_cast<Entity*>(mMob)->startRiding(target);
        stop();
        return;
    }

    --mTimeToRecalcPath;
    if (mTimeToRecalcPath < 1)
    {
        mTimeToRecalcPath = mMob->mLevel->mRandom->nextInt(7) + 4;
        mMob->getNavigation()->moveTo(target, 1.0);
    }
}

std::wstring FindMountGoal::getName()
{
    return std::wstring(L"FindMountGoal");
}
