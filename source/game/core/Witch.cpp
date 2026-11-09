#include "game/core/Witch.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"
#include <cmath>
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/RangedAttackGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Material.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/core/EquipmentSlot.hpp"

const EntityDataAccessor<bool>* Witch::DATA_USING_ITEM = NULL;
static AttributeModifier* SPEED_MODIFIER_DRINKING =
    (new AttributeModifier(eMODIFIER_ID_WitchDrinkingSpeedPenalty, -0.25, 0))->setSerialize(false);
Witch::Witch(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    setSize(0.6f, 1.95f);
    mUsingTime = 0;
    mCategories |= EntityCategory::eEntityCategory_Humanoid | EntityCategory::eEntityCategory_Monster;
}
Entity* Witch::create(Level* level)
{
    return new Witch(level);
}
void Witch::staticCtor()
{
    DATA_USING_ITEM = SynchedEntityData::defineId<bool>(EntityType::WITCH, EntityType::MONSTER,
                                                        EntityDataSerializers::BOOLEAN);
}
void Witch::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::WITCH);
}
void Witch::performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor)
{
    if (isDrinkingPotion())
        return;
    double targetY = target->mY + target->getEyeHeight() - 1.100000023841858;
    double dx = target->mX + target->mXd - mX;
    double dy = targetY - 1.100000023841858 - mY;
    double dz = target->mZ + target->mZd - mZ;
    float horizontal = Mth::sqrt(dx * dx + dz * dz);
    Potion* potion = Potions::HARMING;
    if (horizontal >= 8.0f && !target->hasEffect(MobEffects::MOVEMENT_SLOWDOWN))
    {
        potion = Potions::SLOWNESS;
    }
    else if (target->getHealth() >= 8.0f && !target->hasEffect(MobEffects::POISON))
    {
        potion = Potions::POISON;
    }
    else if (horizontal <= 3.0f && !target->hasEffect(MobEffects::WEAKNESS) && mRandom->nextFloat() < 0.25f)
    {
        potion = Potions::WEAKNESS;
    }
    boost::shared_ptr<ThrownPotion> projectile(new ThrownPotion(
        mLevel, boost::static_pointer_cast<LivingEntity>(shared_from_this()),
        PotionUtils::setPotion(not_null_ptr<ItemInstance>(new ItemInstance(Items::splashPotion)), potion)));
    projectile->mXRot -= -20.0f;
    projectile->shoot(dx, dy + horizontal * 0.2f, dz, 0.75f, 8.0f);
    playSound(SoundEvents::WITCH_THROW, 1.0f, getRandom()->nextFloat() * 0.4f + 0.8f);
    mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::WITCH_THROW, getSoundSource(), 1.0f,
                      mRandom->nextFloat() * 0.4f + 0.8f, 16.0f);
    mLevel->addEntity(projectile);
}
bool Witch::isAggressive()
{
    return false;
}
void Witch::setAggressive(bool)
{
}
void Witch::registerGoals()
{
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, new RangedAttackGoal(this, this, 1.0, 60, 10.0f), true);
    mGoalSelector.addGoal(2, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(3, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(3, new RandomLookAroundGoal(this), true);
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
}
void Witch::defineSynchedData()
{
    Monster::defineSynchedData();
    getEntityData()->define(DATA_USING_ITEM, false);
}
const SoundEvent* Witch::getAmbientSound()
{
    return SoundEvents::WITCH_AMBIENT;
}
const SoundEvent* Witch::getHurtSound(const DamageSource*)
{
    return SoundEvents::WITCH_HURT;
}
const SoundEvent* Witch::getDeathSound()
{
    return SoundEvents::WITCH_DEATH;
}
void Witch::setUsingItem(bool usingItem)
{
    getEntityData()->set(DATA_USING_ITEM, usingItem, true);
}
bool Witch::isDrinkingPotion()
{
    return getEntityData()->get(DATA_USING_ITEM);
}
void Witch::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(26.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
}
void Witch::aiStep()
{
    if (!mLevel->mIsClientSide)
    {
        if (isDrinkingPotion())
        {
            if (mUsingTime-- <= 0)
            {
                setUsingItem(false);
                not_null_ptr<ItemInstance> item = getMainHandItem();
                setItemSlot(EquipmentSlot::MAINHAND, ItemInstance::EMPTY);
                if (item->getItem() == Items::potion)
                {
                    std::vector<MobEffectInstance*>* effects = PotionUtils::getMobEffects(item);
                    if (effects != NULL)
                    {
                        for (std::vector<MobEffectInstance*>::iterator it = effects->begin(); it != effects->end();
                             ++it)
                            addEffect(new MobEffectInstance(*it), boost::shared_ptr<Entity>());
                        delete effects;
                    }
                }
                getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->removeModifier(SPEED_MODIFIER_DRINKING);
            }
        }
        else
        {
            Potion* potion = NULL;
            if (mRandom->nextFloat() < 0.15f && isUnderLiquid(Material::WATER)
                && !hasEffect(MobEffects::WATER_BREATHING))
            {
                potion = Potions::WATER_BREATHING;
            }
            else if (mRandom->nextFloat() < 0.15f
                     && (isOnFire() || (getLastDamageSource() != NULL && getLastDamageSource()->isFire()))
                     && !hasEffect(MobEffects::FIRE_RESISTANCE))
            {
                potion = Potions::FIRE_RESISTANCE;
            }
            else if (mRandom->nextFloat() < 0.05f && getHealth() < getMaxHealth())
            {
                potion = Potions::HEALING;
            }
            else if (mRandom->nextFloat() < 0.25f && getTarget() != NULL && !hasEffect(MobEffects::MOVEMENT_SPEED)
                     && getTarget()->distanceToSqr(shared_from_this()) > 121.0)
            {
                potion = Potions::SWIFTNESS;
            }
            else if (mRandom->nextFloat() < 0.25f && getTarget() != NULL && !hasEffect(MobEffects::MOVEMENT_SPEED)
                     && getTarget()->distanceToSqr(shared_from_this()) > 121.0)
            {
                potion = Potions::SWIFTNESS;
            }

            if (potion != NULL)
            {
                setItemSlot(
                    EquipmentSlot::MAINHAND,
                    PotionUtils::setPotion(not_null_ptr<ItemInstance>(new ItemInstance(Items::potion)), potion));
                mUsingTime = getMainHandItem()->getUseDuration();
                setUsingItem(true);
                mLevel->playSound(boost::shared_ptr<Player>(), mX, mY, mZ, SoundEvents::WITCH_DRINK, getSoundSource(),
                                  1.0f, 0.8f + getRandom()->nextFloat() * 0.4f, 16.0f);
                AttributeInstance* speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
                speed->removeModifier(SPEED_MODIFIER_DRINKING);
                speed->addModifier(new AttributeModifier(*SPEED_MODIFIER_DRINKING));
            }
        }

        if (mRandom->nextFloat() < 0.00075f)
        {
            mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::WITCH_HAT_MAGIC, 0);
        }
    }
    Monster::aiStep();
}

void Witch::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::WITCH_HAT_MAGIC)
    {
        for (int i = 0; i < mRandom->nextInt(35) + 10; ++i)
        {
            mLevel->addParticle(PARTICLE_WITCH_MAGIC, mX + mRandom->nextGaussian() * 0.13f,
                                (mBoundingBox->mMaxY + 0.5) + mRandom->nextGaussian() * 0.13f,
                                mZ + mRandom->nextGaussian() * 0.13f, 0.0, 0.0, 0.0, arrayWithLength<int>());
        }
    }
    else
    {
        Monster::handleEntityEvent(event, data);
    }
}

float Witch::getDamageAfterMagicAbsorb(const DamageSource* source, float damage)
{
    float result = LivingEntity::getDamageAfterMagicAbsorb(source, damage);
    if (source->getEntity() == shared_from_this())
        result = 0.0f;
    if (source->isMagic())
        result *= 0.15f;
    return result;
}
ResourceLocation* Witch::getDefaultLootTable()
{
    return &BuiltInLootTables::WITCH;
}
float Witch::getEyeHeight()
{
    float eyeHeight = 1.62f;
    return eyeHeight;
}
unsigned long long Witch::GetType()
{
    return EntityType::WITCH;
}
