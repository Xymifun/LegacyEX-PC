#include "game/entity/ai/MobNearestMobTargetGoal.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

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

MobNearestMobTargetGoal::MobNearestMobTargetGoal(Mob* mob, const std::type_info& targetType)
    : Goal(),
      mCanAttack(this),
      mSorter(mob),
      mTarget(),
      mTargetType(&targetType)
{
    mMob = mob;
}

double MobNearestMobTargetGoal::getFollowDistance()
{
    AttributeInstance* attribute = mMob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE);
    if (attribute == NULL)
        return 16.0;
    return attribute->getValue();
}

bool MobNearestMobTargetGoal::canUse()
{
    double followDistance = getFollowDistance();
    Mob* mob = mMob;
    AABB* searchBox = mob->mBoundingBox->grow(followDistance, 4.0, followDistance);
    std::vector<boost::shared_ptr<Entity> >* entities = mob->mLevel->getEntitiesOfClass(*mTargetType, searchBox,
                                                                                        &mCanAttack, true);
    std::sort(entities->begin(), entities->end(), mSorter);
    if (entities->empty())
    {
        delete entities;
        return false;
    }

    mTarget = boost::static_pointer_cast<LivingEntity>(entities->at(0));
    delete entities;
    return true;
}

bool MobNearestMobTargetGoal::canContinueToUse()
{
    boost::shared_ptr<LivingEntity> target = mMob->getTarget();
    if (!target)
        return false;

    if (!target->isAlive())
        return false;

    double followDistance = getFollowDistance();
    if (mMob->distanceToSqr(target) > followDistance * followDistance)
        return false;

    bool isServerPlayer = eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, target->GetType());
    if (isServerPlayer)
    {
        boost::shared_ptr<ServerPlayer> player = boost::dynamic_pointer_cast<ServerPlayer>(target);
        if (player->mGameMode->isCreative())
            return false;
    }

    return true;
}

void MobNearestMobTargetGoal::start()
{
    mMob->setTarget(mTarget);
    Goal::start();
}

void MobNearestMobTargetGoal::stop()
{
    mMob->setTarget(boost::shared_ptr<LivingEntity>());
    Goal::start();
}
