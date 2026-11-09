#include "game/entity/mob/Monster.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/EServerSetting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Settings.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/PIXNamedEvents.hpp"

Monster::Monster(Level* level) : PathfinderMob(level)
{
    mExperienceReward = 5;
    mCategories |= EntityCategory::eEntityCategory_Monster;
}

unsigned long long Monster::GetType()
{
    return EntityType::MONSTER;
}

SoundSource::ESoundSource Monster::getSoundSource()
{
    return SoundSource::eSoundSource_Hostile;
}

void Monster::aiStep()
{
    updateSwingTime();
    if (getBrightness() > 0.5f)
        mNoActionTime += 2;
    Mob::aiStep();
}

void Monster::tick()
{
    PIXBeginNamedEvent(0.0f, "Monster tick");
    Mob::tick();
    if (!mLevel->mIsClientSide)
    {
        if (mLevel->getDifficulty() == Difficulty::PEACEFUL || Minecraft::GetInstance()->isTutorial())
            remove();
        else if (!gMinecraftServer->mSettings->getBoolean(eServerSetting_SpawnMonsters, true))
            remove();
    }
    PIXEndNamedEvent();
}

const SoundEvent* Monster::getSwimSound()
{
    return SoundEvents::HOSTILE_SWIM;
}

const SoundEvent* Monster::getSwimSplashSound()
{
    return SoundEvents::HOSTILE_SPLASH;
}

bool Monster::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;
    return LivingEntity::hurt(source, damage);
}

const SoundEvent* Monster::getHurtSound(const DamageSource*)
{
    return SoundEvents::HOSTILE_HURT;
}

const SoundEvent* Monster::getDeathSound()
{
    return SoundEvents::HOSTILE_DEATH;
}

const SoundEvent* Monster::getFallDamageSound(int distance)
{
    return distance > 4 ? SoundEvents::HOSTILE_BIG_FALL : SoundEvents::HOSTILE_SMALL_FALL;
}

bool Monster::doHurtTarget(boost::shared_ptr<Entity> target)
{
    boost::shared_ptr<LivingEntity> attacker = boost::static_pointer_cast<LivingEntity>(shared_from_this());
    float damage = (float)getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue();
    int knockback = 0;
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, target->GetType()))
    {
        boost::shared_ptr<LivingEntity> livingTarget = boost::static_pointer_cast<LivingEntity>(target);
        damage += EnchantmentHelper::getDamageBonus(getMainHandItem(), livingTarget);
        knockback += EnchantmentHelper::getKnockbackBonus(attacker, livingTarget);
    }
    EntityDamageSource* source = DamageSource::CreateMobAttack(attacker);
    bool hurtTarget = target->hurt(source, damage);
    delete source;
    if (hurtTarget)
    {
        if (knockback > 0 && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, target->GetType()))
        {
            boost::static_pointer_cast<LivingEntity>(target)->knockback(
                attacker, knockback * 0.5f, Mth::sin(mYRot * 0.017453292f), -Mth::cos(mYRot * 0.017453292f));
            mXd *= 0.6;
            mZd *= 0.6;
        }
        int fireAspect = EnchantmentHelper::getFireAspect(attacker);
        if (fireAspect > 0)
            target->setSecondsOnFire(fireAspect * 4);
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType()))
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(target);
            not_null_ptr<ItemInstance> mainHandItem = getMainHandItem();
            not_null_ptr<ItemInstance> useItem = player->isUsingItem() ? player->getUseItem() : ItemInstance::EMPTY;
        }
        doEnchantDamageEffects(attacker, target);
    }
    return hurtTarget;
}

float Monster::getWalkTargetValue(const BlockPos& pos)
{
    return 0.5f - mLevel->getBrightness(pos);
}

bool Monster::isDarkEnoughToSpawn()
{
    BlockPos pos(mX, (double)Mth::floor(mBoundingBox->mMinY), mZ);
    if (mLevel->getBrightness(LightLayer::SKY, pos) > mRandom->nextInt(32))
        return false;
    int brightness = mLevel->getRawBrightness(pos);
    if (mLevel->isThundering())
    {
        int skyDarken = mLevel->getSkyDarken();
        mLevel->setSkyDarken(10);
        brightness = mLevel->getRawBrightness(pos);
        mLevel->setSkyDarken(skyDarken);
    }
    return brightness <= mRandom->nextInt(8);
}

bool Monster::checkSpawnRules()
{
    return mLevel->getDifficulty() != Difficulty::PEACEFUL && isDarkEnoughToSpawn() && PathfinderMob::checkSpawnRules();
}

void Monster::registerAttributes()
{
    Mob::registerAttributes();
    getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE);
}

bool Monster::shouldDropExperience()
{
    return true;
}

bool Monster::isPreventingPlayerRest(boost::shared_ptr<Player>)
{
    return true;
}
