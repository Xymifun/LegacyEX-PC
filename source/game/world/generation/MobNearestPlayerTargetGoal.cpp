#include "game/world/generation/MobNearestPlayerTargetGoal.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <typeinfo>
#include <vector>

#include "game/core/AttributeInstance.hpp"
#include "game/core/GameMode.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/server/ServerPlayerGameMode.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

MobNearestPlayerTargetGoal::MobNearestPlayerTargetGoal(Mob* mob)
    : Goal(),
      mTargetSelector(this),
      mSorter(mob),
      mTarget()
{
    mMob = mob;
}

double MobNearestPlayerTargetGoal::getFollowDistance()
{
    AttributeInstance* attribute = mMob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE);
    if (attribute == NULL)
    {
        return 16.0;
    }

    return attribute->getValue();
}

bool MobNearestPlayerTargetGoal::canUse()
{
    double followDistance = getFollowDistance();
    std::vector<boost::shared_ptr<Entity> >* entities = mMob->mLevel->getEntitiesOfClass(
        typeid(Player), mMob->mBoundingBox->grow(followDistance, 4.0, followDistance), &mTargetSelector, false);

    std::sort(entities->begin(), entities->end(), mSorter);

    if (entities->empty())
    {
        delete entities;
        return false;
    }

    mTarget = boost::dynamic_pointer_cast<LivingEntity>(entities->at(0));
    delete entities;
    return true;
}

bool MobNearestPlayerTargetGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
    {
        return false;
    }

    if (!target->isAlive())
    {
        return false;
    }

    Team* team = mMob->getTeam();
    Team* targetTeam = target->getTeam();
    if (team != NULL && targetTeam == team)
    {
        return false;
    }

    double followDistance = getFollowDistance();
    if (mMob->distanceToSqr(target) > followDistance * followDistance)
    {
        return false;
    }

    if (eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, target->GetType()))
    {
        boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(target);
        if (player->mGameMode->isCreative())
        {
            return false;
        }
    }

    return true;
}

void MobNearestPlayerTargetGoal::start()
{
    mMob->setTarget(mTarget);
    Goal::start();
}

void MobNearestPlayerTargetGoal::stop()
{
    mMob->setTarget(boost::shared_ptr<LivingEntity>());
    Goal::start();
}
