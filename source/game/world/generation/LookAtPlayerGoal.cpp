#include "game/world/generation/LookAtPlayerGoal.hpp"

#include "game/entity/ai/EntitySelector.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/generation/Player.hpp"

LookAtPlayerGoal::LookAtPlayerGoal(Mob* mob, const std::type_info& lookAtType, float lookDistance)
    : mMob(mob),
      mLookAt(),
      mLookDistance(lookDistance),
      mProbability(0.02f),
      mLookAtType(&lookAtType)
{
    setRequiredControlFlags(2);
    mLookTime = 0;
}

LookAtPlayerGoal::LookAtPlayerGoal(Mob* mob, const std::type_info& lookAtType, float lookDistance, float probability)
    : mMob(mob),
      mLookAt(),
      mLookDistance(lookDistance),
      mProbability(probability),
      mLookAtType(&lookAtType)
{
    setRequiredControlFlags(2);
    mLookTime = 0;
}

bool LookAtPlayerGoal::canUse()
{
    if (mMob->getRandom()->nextFloat() >= mProbability)
    {
        return false;
    }

    if (mMob->getTarget())
    {
        mLookAt = mMob->getTarget();
    }

    if (*mLookAtType == typeid(Player))
    {
        Predicate<boost::shared_ptr<Entity> >* notRiding = EntitySelector::notRiding(
            boost::static_pointer_cast<Mob>(mMob->shared_from_this()));
        const Predicate<boost::shared_ptr<Entity> >* selectors[2] = {EntitySelector::NO_SPECTATORS, notRiding};
        Predicate<boost::shared_ptr<Entity> >* selector =
            new Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate(
                Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate::eOperator_And, selectors, 2);

        mLookAt = mMob->mLevel->getNearestPlayerPredicate(mMob->mX, mMob->mY, mMob->mZ, (double)mLookDistance,
                                                          selector);

        delete notRiding;
        delete selector;
    }
    else
    {
        mLookAt = mMob->mLevel->getClosestEntityOfClass(
            *mLookAtType, mMob->mBoundingBox->grow((double)mLookDistance, 3.0, (double)mLookDistance),
            mMob->shared_from_this());
    }

    return mLookAt.lock().get() != NULL;
}

bool LookAtPlayerGoal::canContinueToUse()
{
    if (!mLookAt.lock() || !mLookAt.lock()->isAlive())
    {
        return false;
    }

    if (mMob->distanceToSqr(mLookAt.lock()) > (double)(mLookDistance * mLookDistance))
    {
        return false;
    }

    if (mMob->hasPassenger(mLookAt.lock()))
    {
        return false;
    }

    return mLookTime > 0;
}

void LookAtPlayerGoal::start()
{
    mLookTime = mMob->getRandom()->nextInt(40) + 40;
}

void LookAtPlayerGoal::stop()
{
    mLookAt = boost::weak_ptr<Entity>();
}

void LookAtPlayerGoal::tick()
{
    mMob->getLookControl()->setLookAt(mLookAt.lock()->mX, mLookAt.lock()->mY + mLookAt.lock()->getEyeHeight(),
                                      mLookAt.lock()->mZ, (float)mMob->getMaxHeadYRot(), (float)mMob->getMaxHeadXRot());
    --mLookTime;
}

bool LookAtPlayerGoal::LookAtPlayerGoalEntitySelector::apply(boost::shared_ptr<Entity> entity) const
{
    return EntitySelector::NO_SPECTATORS->apply(entity)
           && EntitySelector::NOT_BEING_RIDDEN_ENTITY_SELECTOR->apply(entity);
}

LookAtPlayerGoal::LookAtPlayerGoalEntitySelector::~LookAtPlayerGoalEntitySelector()
{
}
