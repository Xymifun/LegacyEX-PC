#include "game/entity/ai/TargetGoal.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Mth.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/AttributeInstance.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/Path.hpp"
#include "game/core/Sensing.hpp"
#include "game/core/Team.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/OwnableEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/WitherSkeleton.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

TargetGoal::TargetGoal(PathfinderMob* mob, bool mustSee) : Goal()
{
    _init(mob, mustSee, false);
}

TargetGoal::TargetGoal(PathfinderMob* mob, bool mustSee, bool mustReach) : Goal()
{
    _init(mob, mustSee, mustReach);
}

void TargetGoal::_init(PathfinderMob* mob, bool mustSee, bool mustReach)
{
    mMustSee = mustSee;
    mMob = mob;
    mMustReach = mustReach;
    mTargetSearchDelay = 0;
    mCanReach = 0;
    mUnseenTicks = 0;
    mTarget.reset();
    mUnseenMemoryTicks = 60;
}

bool TargetGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        target = mTarget.lock();

    if (!target)
        return false;

    if (!target->isAlive())
        return false;

    if (eTYPE_DERIVED_FROM(EntityType::WITHER_SKELETON, mMob->GetType()))
    {
        boost::shared_ptr<WitherBoss> parent = static_cast<WitherSkeleton*>(mMob)->getWitherParent();
        if (parent && target->getId() == parent->getId())
            return false;
    }

    Team* team = mMob->getTeam();
    Team* targetTeam = target->getTeam();
    if (team != NULL && targetTeam == team)
        return false;

    double followDistance = getFollowDistance();
    if (mMob->distanceToSqr(target) > followDistance * followDistance)
        return false;

    if (mMustSee)
    {
        if (mMob->getSensing()->canSee(target))
            mUnseenTicks = 0;
        else if (++mUnseenTicks > mUnseenMemoryTicks)
            return false;
    }

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType()))
    {
        if (target->mLevel->getDifficulty() == Difficulty::PEACEFUL)
        {
            mMob->setTarget(boost::shared_ptr<LivingEntity>());
            return false;
        }

        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(target);
        if (player->mAbilities.mInvulnerable)
            return false;
    }

    mMob->setTarget(target);
    return true;
}

double TargetGoal::getFollowDistance()
{
    AttributeInstance* attribute = mMob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE);
    if (attribute == NULL)
        return 16.0;
    return attribute->getValue();
}

void TargetGoal::start()
{
    mCanReach = 0;
    mTargetSearchDelay = 0;
    mUnseenTicks = 0;
}

void TargetGoal::stop()
{
    mMob->setTarget(boost::shared_ptr<LivingEntity>());
    mTarget.reset();
}

bool TargetGoal::canAttack(Mob* mob, boost::shared_ptr<LivingEntity> target, bool allowInvulnerablePlayers,
                           bool requireSight)
{
    if (!target)
        return false;

    if (target == mob->shared_from_this())
        return false;

    if (!target->isAlive())
        return false;

    if (!mob->canAttackType(target->GetType()))
        return false;

    if (mob->isAlliedTo(target))
        return false;

    OwnableEntity* mobOwnable = dynamic_cast<OwnableEntity*>(mob);
    if (mobOwnable != NULL && !mobOwnable->getOwnerUUID().empty())
    {
        boost::shared_ptr<OwnableEntity> targetOwnable = boost::dynamic_pointer_cast<OwnableEntity>(target);
        if (targetOwnable && mobOwnable->getOwnerUUID() == targetOwnable->getOwnerUUID())
            return false;

        if (target == mobOwnable->getOwner())
            return false;
    }
    else if (eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType()))
    {
        if ((!allowInvulnerablePlayers && boost::static_pointer_cast<Player>(target)->mAbilities.mInvulnerable)
            || target->mLevel->getDifficulty() == Difficulty::PEACEFUL)
            return false;
    }

    if (requireSight && !mob->getSensing()->canSee(target))
        return false;

    return true;
}

bool TargetGoal::canReach(boost::shared_ptr<LivingEntity> target)
{
    mTargetSearchDelay = mMob->getRandom()->nextInt(5) + 10;
    Path* path = mMob->getNavigation()->createPath(target);
    if (path == NULL)
        return false;

    Node* last = path->last();
    if (last == NULL)
    {
        delete path;
        return false;
    }

    int dx = last->x - Mth::floor(target->mX);
    int dz = last->z - Mth::floor(target->mZ);
    delete path;
    return (double)(dx * dx + dz * dz) <= 2.25;
}

bool TargetGoal::canAttack(boost::shared_ptr<LivingEntity> target, bool allowInvulnerablePlayers)
{
    if (!canAttack(mMob, target, allowInvulnerablePlayers, mMustSee))
        return false;

    if (!mMob->isWithinRestriction(BlockPos(Mth::floor(target->mX), Mth::floor(target->mY), Mth::floor(target->mZ))))
        return false;

    if (!mMustReach)
        return true;

    if (--mTargetSearchDelay <= 0)
        mCanReach = 0;

    if (mCanReach == 0)
        mCanReach = canReach(target) ? 1 : 2;

    if (mCanReach == 2)
        return false;

    return true;
}

TargetGoal* TargetGoal::setUnseenMemoryTicks(int ticks)
{
    mUnseenMemoryTicks = ticks;
    return this;
}
