#include "game/entity/ai/AvoidEntityGoal.hpp"

#include <vector>

#include "game/core/Path.hpp"
#include "game/entity/ai/AvoidEntityGoalEntitySelector.hpp"
#include "game/entity/ai/EntitySelector.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/MoverType.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/AABB.hpp"
#include "game/util/RandomPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Predicates.hpp"

AvoidEntityGoal::AvoidEntityGoal(PathfinderMob* mob, const std::type_info& avoidType, float maxDistance,
                                 double walkSpeedModifier, double sprintSpeedModifier)
    : Goal(),
      mAvoidType(&avoidType),
      mToAvoid()
{
    _init(mob, Predicates<boost::shared_ptr<Entity> >::alwaysTrue, maxDistance, walkSpeedModifier, sprintSpeedModifier);
}

AvoidEntityGoal::~AvoidEntityGoal()
{
    if (mPath != NULL)
        delete mPath;

    if (mSelector != Predicates<boost::shared_ptr<Entity> >::alwaysTrue)
        delete mSelector;

    delete mEntitySelector;
}

void AvoidEntityGoal::_init(PathfinderMob* mob, const Predicate<boost::shared_ptr<Entity> >* selector,
                            float maxDistance, double walkSpeedModifier, double sprintSpeedModifier)
{
    mMob = mob;
    mSelector = selector;
    mMaxDistance = maxDistance;
    mWalkSpeedModifier = walkSpeedModifier;
    mSprintSpeedModifier = sprintSpeedModifier;
    mNavigation = mob->getNavigation();
    setRequiredControlFlags(1);
    mEntitySelector = new AvoidEntityGoalEntitySelector(this);
    mToAvoid = boost::weak_ptr<Entity>();
    mPath = NULL;
}

bool AvoidEntityGoal::canUse()
{
    const Predicate<boost::shared_ptr<Entity> >* predicates[] = {EntitySelector::NO_CREATIVE_OR_SPECTATOR,
                                                                 mEntitySelector, mSelector};

    Predicate<boost::shared_ptr<Entity> >* predicate = new Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate(
        Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate::eOperator_And, predicates, 3);

    AABB* bb = mMob->mBoundingBox->grow(mMaxDistance, 3.0, mMaxDistance);
    std::vector<boost::shared_ptr<Entity> >* entities = mMob->mLevel->getEntitiesOfClass(*mAvoidType, bb, predicate,
                                                                                         true);

    delete predicate;

    if (entities->empty())
    {
        delete entities;
        return false;
    }

    mToAvoid = boost::weak_ptr<Entity>();

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        if (!(*it)->isCreative())
        {
            mToAvoid = boost::weak_ptr<Entity>(*it);
            break;
        }
    }

    delete entities;

    boost::shared_ptr<Entity> toAvoid = mToAvoid.lock();
    if (toAvoid.get() == NULL)
        return false;

    Vec3* pos = RandomPos::getPosAvoid(boost::dynamic_pointer_cast<PathfinderMob>(mMob->shared_from_this()), 16, 7,
                                       Vec3::newTemp(toAvoid->mX, toAvoid->mY, toAvoid->mZ));
    if (pos == NULL)
        return false;

    if (toAvoid->distanceToSqr(pos->x, pos->y, pos->z) < mToAvoid.lock()->distanceToSqr(mMob->shared_from_this()))
        return false;

    delete mPath;
    mPath = mNavigation->createPath(pos->x, pos->y, pos->z);
    return mPath != NULL;
}

bool AvoidEntityGoal::canContinueToUse()
{
    return mToAvoid.lock().get() != NULL && !mNavigation->isDone() && !mMob->getNavigation()->isRiverFollowing();
}

void AvoidEntityGoal::start()
{
    if (!mMob->getNavigation()->isRiverFollowing())
    {
        mNavigation->moveTo(mPath, mWalkSpeedModifier);
        mPath = NULL;
    }
}

void AvoidEntityGoal::stop()
{
    mToAvoid = boost::weak_ptr<Entity>();
}

void AvoidEntityGoal::tick()
{
    if (mMob->distanceToSqr(mToAvoid.lock()) < 49.0)
        mMob->getNavigation()->setSpeedModifier(mSprintSpeedModifier);
    else
        mMob->getNavigation()->setSpeedModifier(mWalkSpeedModifier);

    if (mMob->getNavigation()->isRiverFollowing())
    {
        Vec3* direction = mMob->getPos(0.0f)->subtract(mToAvoid.lock()->getPos(0.0f));
        Vec3* delta = direction->normalize()->add(mMob->mXd, mMob->mYd, mMob->mZd)->multiply(mMob->getSpeed() * 0.75f);
        mMob->move(eMoverType_Self, delta->x, delta->y, delta->z, false);
    }
}
