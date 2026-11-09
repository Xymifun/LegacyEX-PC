#include "game/entity/ai/HurtByTargetGoal.hpp"

#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/EntityType.hpp"
#include "game/entity/ai/TargetGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"

HurtByTargetGoal::HurtByTargetGoal(PathfinderMob* mob, bool alertSameType,
                                   arrayWithLength<unsigned long long> entityTypes)
    : TargetGoal(mob, false),
      mAlertSameType(alertSameType)
{
    mEntityTypes = entityTypes;
    setRequiredControlFlags(1);
    mTimestamp = 0;
}

HurtByTargetGoal::~HurtByTargetGoal()
{
    delete[] mEntityTypes.data;
}

bool HurtByTargetGoal::canUse()
{
    int timestamp = mMob->getLastHurtByMobTimestamp();
    return timestamp != mTimestamp && canAttack(mMob->getLastHurtByMob(), false) && !mMob->mHurtByPlayerOnPeaceful;
}

void HurtByTargetGoal::alertOthers()
{
    double followDistance = getFollowDistance();
    std::vector<boost::shared_ptr<Entity> >* entities = mMob->mLevel->getEntitiesOfClass(
        typeid(*mMob), AABB::newTemp(mMob->mX, mMob->mY, mMob->mZ, mMob->mX + 1.0, mMob->mY + 1.0, mMob->mZ + 1.0)
                           ->grow(followDistance, 4.0, followDistance));

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<PathfinderMob> mob = boost::dynamic_pointer_cast<PathfinderMob>(*it);
        if (mMob->shared_from_this() == mob)
            continue;

        if (mob->getTarget())
            continue;

        if (eTYPE_DERIVED_FROM(EntityType::TAMABLE_ANIMAL, mMob->GetType())
            && eTYPE_DERIVED_FROM(EntityType::TAMABLE_ANIMAL, mob->GetType())
            && static_cast<TamableAnimal*>(mMob)->getOwner()
                   != boost::dynamic_pointer_cast<TamableAnimal>(mob)->getOwner())
            continue;

        if (mob->isAlliedTo(mMob->getLastHurtByMob()))
            continue;

        bool excluded = false;
        unsigned int i;
        for (i = 0; i != mEntityTypes.length; ++i)
        {
            unsigned long long entityType = mEntityTypes[i];
            if (entityType == mob->GetType())
            {
                excluded = true;
                break;
            }
        }
        if (excluded)
            continue;

        alertOther(mob, mMob->getLastHurtByMob());
    }

    delete entities;
}

void HurtByTargetGoal::start()
{
    mMob->setTarget(mMob->getLastHurtByMob());
    mTarget = mMob->getTarget();
    mTimestamp = mMob->getLastHurtByMobTimestamp();
    mUnseenMemoryTicks = 300;
    if (mAlertSameType)
        alertOthers();
    TargetGoal::start();
}

void HurtByTargetGoal::alertOther(boost::shared_ptr<PathfinderMob> mob, boost::shared_ptr<LivingEntity> target)
{
    mob->setTarget(target);
}
