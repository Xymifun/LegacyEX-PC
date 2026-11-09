#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/core/EquipmentSlot.hpp"
#include "game/entity/ai/SubselectEntitySelector.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/generation/Player.hpp"

NearestAttackableTargetGoal::DistComp::DistComp(Entity* entity) : mEntity(entity)
{
}

bool NearestAttackableTargetGoal::DistComp::operator()(boost::shared_ptr<Entity> first,
                                                       boost::shared_ptr<Entity> second)
{
    return mEntity->distanceToSqr(first) < mEntity->distanceToSqr(second);
}

NearestAttackableTargetGoal::DisguisedRangeModifier::DisguisedRangeModifier(PathfinderMob* mob) : mMob(mob)
{
}

NearestAttackableTargetGoal::DisguisedRangeModifier::~DisguisedRangeModifier()
{
}

double NearestAttackableTargetGoal::DisguisedRangeModifier::apply(boost::shared_ptr<Player> player) const
{
    not_null_ptr<ItemInstance> item = player->getItemSlot(EquipmentSlot::HEAD);
    if (item->getItem() == Items::skull)
    {
        int damage = item->getDamageValue();
        bool isSkeleton = eTYPE_DERIVED_FROM(EntityType::SKELETON, mMob->GetType());
        bool disguisedAsSkeleton = isSkeleton && damage == 0;
        bool disguisedAsZombie = eTYPE_DERIVED_FROM(EntityType::ZOMBIE, mMob->GetType()) && damage == 2;
        bool isCreeper = eTYPE_DERIVED_FROM(EntityType::CREEPER, mMob->GetType());
        bool disguisedAsCreeper = isCreeper && damage == 4;
        if (disguisedAsSkeleton || disguisedAsZombie || disguisedAsCreeper)
            return 0.5;
    }
    return 1.0;
}

NearestAttackableTargetGoal::NearestAttackableTargetGoal(PathfinderMob* mob, const std::type_info& targetType,
                                                         bool mustSee, bool mustReach, bool includeRemoved)
    : TargetGoal(mob, mustSee, mustReach),
      mTargetType(&targetType),
      mSorter(mob)
{
    mRandomInterval = 10;
    setRequiredControlFlags(1);
    mTargetSelector = new SubselectEntitySelector(this, Predicates<boost::shared_ptr<Entity> >::alwaysTrue, false);
    mIncludeRemoved = includeRemoved;
    mRangeModifier = new DisguisedRangeModifier(mob);
}

NearestAttackableTargetGoal::NearestAttackableTargetGoal(PathfinderMob* mob, const std::type_info& targetType,
                                                         int randomInterval, bool mustSee, bool mustReach,
                                                         const Predicate<boost::shared_ptr<Entity> >* selector,
                                                         bool includeRemoved, bool ownsSelector)
    : TargetGoal(mob, mustSee, mustReach),
      mTargetType(&targetType),
      mSorter(mob)
{
    mRandomInterval = randomInterval;
    setRequiredControlFlags(1);
    mTargetSelector = new SubselectEntitySelector(this, selector, ownsSelector);
    mIncludeRemoved = includeRemoved;
    mRangeModifier = new DisguisedRangeModifier(mob);
}

NearestAttackableTargetGoal::~NearestAttackableTargetGoal()
{
    delete mRangeModifier;
    delete mTargetSelector;
}

bool NearestAttackableTargetGoal::canUse()
{
    if (mRandomInterval > 0 && mMob->getRandom()->nextInt(mRandomInterval) != 0)
        return false;
    if (*mTargetType == typeid(Player) || *mTargetType == typeid(ServerPlayer))
    {
        mTarget = mMob->mLevel->getNearestAttackablePlayer(mMob->mX, mMob->mY + mMob->getEyeHeight(), mMob->mZ,
                                                           getFollowDistance(), getFollowDistance(), mRangeModifier,
                                                           mTargetSelector);
        return mTarget.lock().get() != NULL;
    }

    std::vector<boost::shared_ptr<Entity> >* entities = mMob->mLevel->getEntitiesOfClass(
        *mTargetType, getTargetSearchArea(getFollowDistance()), mTargetSelector, mIncludeRemoved);
    bool found = false;
    if (entities != NULL && !entities->empty())
    {
        std::sort(entities->begin(), entities->end(), mSorter);
        double distance = getFollowDistance();
        if (mMob->distanceToSqr(entities->at(0)) <= distance * distance)
        {
            mTarget = boost::weak_ptr<LivingEntity>(boost::dynamic_pointer_cast<LivingEntity>(entities->at(0)));
            found = true;
        }
    }
    delete entities;
    return found;
}

bool NearestAttackableTargetGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        target = TargetGoal::mTarget.lock();

    if (!target)
        return false;

    if (!target->isAlive())
        return false;

    if (!mTargetSelector->apply(target))
        return false;

    return TargetGoal::canContinueToUse();
}

void NearestAttackableTargetGoal::start()
{
    mMob->setTarget(mTarget.lock());
    TargetGoal::start();
}

AABB* NearestAttackableTargetGoal::getTargetSearchArea(double distance)
{
    return mMob->mBoundingBox->grow(distance, 4.0, distance);
}
