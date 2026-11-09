#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Difficulty.hpp"
#include "game/core/Path.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/core/Sensing.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

MeleeAttackGoal::MeleeAttackGoal(PathfinderMob* mob, double speedModifier, bool followingTargetEvenIfNotSeen)
    : mAttackInterval(20)
{
    _init(mob, speedModifier, followingTargetEvenIfNotSeen);
}

MeleeAttackGoal::~MeleeAttackGoal()
{
    delete mPath;
}

void MeleeAttackGoal::_init(PathfinderMob* mob, double speedModifier, bool followingTargetEvenIfNotSeen)
{
    mMob = mob;
    mLevel = mob->mLevel;
    mSpeedModifier = speedModifier;
    mFollowingTargetEvenIfNotSeen = followingTargetEvenIfNotSeen;
    setRequiredControlFlags(3);
    mTicksUntilNextAttack = 0;
    mPath = NULL;
    mTicksUntilNextPathRecalculation = 0;
    mTargetZ = 0.0;
    mTargetY = 0.0;
    mTargetX = 0.0;
}

bool MeleeAttackGoal::canUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        return false;
    if (!target->isAlive())
        return false;

    delete mPath;
    mPath = mMob->getNavigation()->createPath(target);
    if (mPath != NULL)
        return true;

    if (!(getAttackReachSqr(target) < mMob->distanceToSqr(target->mX, target->mBoundingBox->mMinY, target->mZ)))
        return true;

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType())
        && mMob->mLevel->getDifficulty() == Difficulty::PEACEFUL)
    {
        mMob->setTarget(boost::shared_ptr<LivingEntity>());
        return false;
    }
    return false;
}

bool MeleeAttackGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        return false;
    if (!target->isAlive())
        return false;

    if (!mFollowingTargetEvenIfNotSeen)
        return !mMob->getNavigation()->isDone();

    if (!mMob->isWithinRestriction(BlockPos(target)))
        return false;

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType()))
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(target);
        if (player->isSpectator() || player->isCreative() || mMob->mLevel->getDifficulty() == Difficulty::PEACEFUL)
            return false;
    }
    return true;
}

void MeleeAttackGoal::start()
{
    mMob->getNavigation()->moveTo(mPath, mSpeedModifier);
    mPath = NULL;
    mTicksUntilNextPathRecalculation = 0;
}

void MeleeAttackGoal::stop()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (target && eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType()))
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(target);
        if (player->isSpectator() || player->isCreative())
            mMob->setTarget(boost::shared_ptr<LivingEntity>());
    }

    mMob->getNavigation()->stop();
}

void MeleeAttackGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    mMob->getLookControl()->setLookAt(target, 30.0f, 30.0f);
    double distance = mMob->distanceToSqr(target->mX, target->mBoundingBox->mMinY, target->mZ);
    --mTicksUntilNextPathRecalculation;
    int canSeeTarget = mFollowingTargetEvenIfNotSeen || mMob->getSensing()->canSee(target);

    if (canSeeTarget && mTicksUntilNextPathRecalculation <= 0
        && ((mTargetX == 0.0 && mTargetY == 0.0 && mTargetZ != 0.0)
            || target->distanceToSqr(mTargetX, mTargetY, mTargetZ) >= 1.0 || mMob->getRandom()->nextFloat() < 0.05f))
    {
        mTargetX = target->mX;
        mTargetY = target->mBoundingBox->mMinY;
        mTargetZ = target->mZ;

        int pathRecalculationDelay = mMob->getRandom()->nextInt(7);
        mTicksUntilNextPathRecalculation = pathRecalculationDelay + 4;
        if (distance > 1024.0)
            mTicksUntilNextPathRecalculation += 10;
        else if (distance > 256.0)
            mTicksUntilNextPathRecalculation += 5;

        if (!mMob->getNavigation()->moveTo(target, mSpeedModifier))
            mTicksUntilNextPathRecalculation += 15;
    }

    mTicksUntilNextAttack = std::max(mTicksUntilNextAttack - 1, 0);
    checkAndPerformAttack(target, distance);
}

void MeleeAttackGoal::checkAndPerformAttack(boost::shared_ptr<LivingEntity> target, double distance)
{
    double attackReachSqr = getAttackReachSqr(target);
    if (distance <= attackReachSqr && mTicksUntilNextAttack <= 0)
    {
        mTicksUntilNextAttack = mAttackInterval;
        mMob->swing(InteractionHand::eInteractionHand_MainHand);
        mMob->doHurtTarget(target);
    }
}

double MeleeAttackGoal::getAttackReachSqr(boost::shared_ptr<LivingEntity> target)
{
    Entity* reachEntity = Entity::instanceof(mMob->getVehicle(), EntityType::MOB) ? mMob->getVehicle().get() : mMob;
    return (reachEntity->mBbWidth + reachEntity->mBbWidth) * (reachEntity->mBbWidth + reachEntity->mBbWidth)
           + target->mBbWidth;
}

void MeleeAttackGoal::setLevel(Level* level)
{
    mLevel = level;
}
