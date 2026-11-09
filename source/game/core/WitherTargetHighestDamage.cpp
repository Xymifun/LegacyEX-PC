#include "game/core/WitherTargetHighestDamage.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Vec3.hpp"

WitherTargetHighestDamage::WitherTargetHighestDamage(WitherBoss* wither) : TargetGoal(wither, false, false)
{
    setRequiredControlFlags(1);
}

boost::shared_ptr<LivingEntity> WitherTargetHighestDamage::getHighestDamageTarget()
{
    int highestDamage = 0;
    boost::shared_ptr<LivingEntity> target;

    std::map<boost::shared_ptr<ServerPlayer>, int>& party = static_cast<WitherBoss*>(mMob)->getPlayerParty();
    for (std::map<boost::shared_ptr<ServerPlayer>, int>::iterator it = party.begin(); it != party.end(); ++it)
    {
        int damage = it->second;
        boost::shared_ptr<LivingEntity> player = it->first;
        if (damage > highestDamage && canAttack(player, false))
        {
            highestDamage = damage;
            target = player;
        }
    }
    return target;
}

bool WitherTargetHighestDamage::canUse()
{
    boost::shared_ptr<LivingEntity> target = getHighestDamageTarget();
    if (target && canAttack(target, false))
    {
        mTarget = target;
        return true;
    }
    return false;
}

bool WitherTargetHighestDamage::canContinueToUse()
{
    if (TargetGoal::canContinueToUse() && getHighestDamageTarget() == mTarget.lock())
    {
        return true;
    }
    return false;
}

void WitherTargetHighestDamage::start()
{
    static_cast<WitherBoss*>(mMob)->setTarget(mTarget.lock());
}

bool WitherTargetHighestDamage::canAttack(boost::shared_ptr<LivingEntity> target, bool allowInvulnerablePlayers)
{
    double followDistance = mMob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->getValue();
    Vec3* delta = mMob->getPos(1.0f)->subtract(target->mX, target->mY, target->mZ);
    if (delta->lengthSqr() > followDistance * followDistance)
        return false;
    return TargetGoal::canAttack(mMob, target, allowInvulnerablePlayers, mMustSee);
}
