#include "game/core/IllusionIllager.hpp"
#include "game/core/Mth.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/EntityType.hpp"
#include <cmath>
#include <typeinfo>
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AbstractIllager.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RangedBowAttackGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"

extern SpellcasterIllager::IllagerSpell* gSpell4;
extern SpellcasterIllager::IllagerSpell* gSpell5;

IllusionIllager::IllusionIllager(Level* level) : SpellcasterIllager(level), RangedAttackMob()
{
    fjDerivedCtorCalls();
    setSize(0.6f, 1.95f);
    mExperienceReward = 5;
    mDisguisedPosition = Vec3::newPermanent(0.0, 0.0, 0.0);
    mDisguisedPositionO = Vec3::newPermanent(0.0, 0.0, 0.0);
    mDisguisedPosition2 = Vec3::newPermanent(0.0, 0.0, 0.0);
    mDisguisedPosition2O = Vec3::newPermanent(0.0, 0.0, 0.0);
    mDisguisedPosition3 = Vec3::newPermanent(0.0, 0.0, 0.0);
    mDisguisedPosition3O = Vec3::newPermanent(0.0, 0.0, 0.0);
    mDisguisedPosition4 = Vec3::newPermanent(0.0, 0.0, 0.0);
    mDisguisedPosition4O = Vec3::newPermanent(0.0, 0.0, 0.0);
}

Entity* IllusionIllager::create(Level* level)
{
    return new IllusionIllager(level);
}
bool IllusionIllager::isAggressive()
{
    return getIllagerFlag(1);
}
void IllusionIllager::setAggressive(bool aggressive)
{
    setIllagerFlag(1, aggressive);
}

void IllusionIllager::registerGoals()
{
    Mob::registerGoals();
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(4, new IllusionistMirrorSpellGoal(this), true);
    mGoalSelector.addGoal(5, new IllusionistBlindnessSpellGoal(this), true);
    mGoalSelector.addGoal(6, new RangedBowAttackGoal(this, 0.5, 20, 15.0f), true);
    mGoalSelector.addGoal(8, new RandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f), true);

    arrayWithLength<unsigned long long> hurtByTargetTypes(1, true);
    hurtByTargetTypes.data[0] = EntityType::ILLUSION_ILLAGER;
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, hurtByTargetTypes), true);
    mTargetSelector.addGoal(
        2, (new NearestAttackableTargetGoal(this, typeid(Player), true, false, false))->setUnseenMemoryTicks(300),
        true);
    mTargetSelector.addGoal(
        3, (new NearestAttackableTargetGoal(this, typeid(Villager), false, false, false))->setUnseenMemoryTicks(300),
        true);
    mTargetSelector.addGoal(
        3,
        (new NearestAttackableTargetGoal(this, typeid(VillagerGolem), false, false, false))->setUnseenMemoryTicks(300),
        true);
}

void IllusionIllager::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(18.0);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(32.0);
}

MobGroupData* IllusionIllager::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                                const std::wstring& dataString)
{
    setItemSlot(EquipmentSlot::MAINHAND, not_null_ptr<ItemInstance>(new ItemInstance(Items::bow)));
    return Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}
void IllusionIllager::defineSynchedData()
{
    SpellcasterIllager::defineSynchedData();
}
ResourceLocation* IllusionIllager::getDefaultLootTable()
{
    return BuiltInLootTables::EMPTY;
}
AABB* IllusionIllager::getBoundingBoxForCulling()
{
    return mBoundingBox->grow(3.0, 0.0, 3.0);
}

void IllusionIllager::aiStep()
{
    Monster::aiStep();

    if (mLevel->mIsClientSide && isInvisible())
    {
        int nextLevel = mIllusionLevel - 1;
        if (nextLevel < 0)
        {
            nextLevel = 0;
        }

        if (mHurtTime == 1 || mTickCount % 1200 == 0)
        {
            mIllusionLevel = 3;

            int roll;
            double y;
            delete mDisguisedPosition;
            mDisguisedPosition = mDisguisedPositionO;
            roll = mRandom->nextInt(6) - 4;
            y = 0.0;
            if (roll > 0)
                y = roll;
            mDisguisedPositionO = Vec3::newPermanent((-6.0f + (float)mRandom->nextInt(13)) * 0.5, y,
                                                     (-6.0f + (float)mRandom->nextInt(13)) * 0.5);
            delete mDisguisedPosition2;
            mDisguisedPosition2 = mDisguisedPosition2O;
            roll = mRandom->nextInt(6) - 4;
            mDisguisedPosition2O = Vec3::newPermanent((-6.0f + (float)mRandom->nextInt(13)) * 0.5,
                                                      roll <= 0 ? 0.0 : (double)roll,
                                                      (-6.0f + (float)mRandom->nextInt(13)) * 0.5);
            delete mDisguisedPosition3;
            mDisguisedPosition3 = mDisguisedPosition3O;
            roll = mRandom->nextInt(6) - 4;
            mDisguisedPosition3O = Vec3::newPermanent((-6.0f + (float)mRandom->nextInt(13)) * 0.5,
                                                      roll <= 0 ? 0.0 : (double)roll,
                                                      (-6.0f + (float)mRandom->nextInt(13)) * 0.5);
            delete mDisguisedPosition4;
            mDisguisedPosition4 = mDisguisedPosition4O;
            roll = mRandom->nextInt(6) - 4;
            mDisguisedPosition4O = Vec3::newPermanent((-6.0f + (float)mRandom->nextInt(13)) * 0.5,
                                                      roll <= 0 ? 0.0 : (double)roll,
                                                      (-6.0f + (float)mRandom->nextInt(13)) * 0.5);

            for (int i = 0; i < 16; ++i)
            {
                mLevel->addParticle(PARTICLE_CLOUD, mX + (mRandom->nextDouble() - 0.5) * mBbWidth,
                                    mY + mRandom->nextDouble() * mBbHeight,
                                    mZ + (mRandom->nextDouble() - 0.5) * mBbWidth, 0.0, 0.0, 0.0,
                                    arrayWithLength<int>());
            }

            mLevel->playLocalSound(mX, mY, mZ, SoundEvents::ILLUSION_ILLAGER_MIRROR_MOVE, getSoundSource(), 1.0f, 1.0f,
                                   false, 16.0f);
        }
        else if (mHurtTime == mHurtDuration - 1)
        {
            mIllusionLevel = 3;
            delete mDisguisedPosition;
            mDisguisedPosition = mDisguisedPositionO;
            mDisguisedPositionO = Vec3::newPermanent(0.0, 0.0, 0.0);
            delete mDisguisedPosition2;
            mDisguisedPosition2 = mDisguisedPosition2O;
            mDisguisedPosition2O = Vec3::newPermanent(0.0, 0.0, 0.0);
            delete mDisguisedPosition3;
            mDisguisedPosition3 = mDisguisedPosition3O;
            mDisguisedPosition3O = Vec3::newPermanent(0.0, 0.0, 0.0);
            delete mDisguisedPosition4;
            mDisguisedPosition4 = mDisguisedPosition4O;
            mDisguisedPosition4O = Vec3::newPermanent(0.0, 0.0, 0.0);
        }
        else
        {
            mIllusionLevel = nextLevel;
        }
    }
}

bool IllusionIllager::isAlliedTo(boost::shared_ptr<Entity> entity)
{
    if (Entity::isAlliedTo(entity))
        return true;
    bool isLiving = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType());
    int illager = isLiving && boost::static_pointer_cast<LivingEntity>(entity)->getMobType() == MOB_TYPE_ILLAGER;
    if (illager)
        return getTeam() == NULL && entity->getTeam() == NULL;
    return false;
}
const SoundEvent* IllusionIllager::getAmbientSound()
{
    return SoundEvents::ILLUSION_ILLAGER_AMBIENT;
}
const SoundEvent* IllusionIllager::getDeathSound()
{
    return SoundEvents::ILLUSION_ILLAGER_DEATH;
}
const SoundEvent* IllusionIllager::getHurtSound(const DamageSource*)
{
    return SoundEvents::ILLUSION_ILLAGER_HURT;
}
const SoundEvent* IllusionIllager::getCastingSoundEvent()
{
    return SoundEvents::ILLUSION_ILLAGER_CAST_SPELL;
}
int IllusionIllager::getArmPose()
{
    return isCastingSpell() ? 2 : (isAggressive() ? 3 : 0);
}
unsigned long long IllusionIllager::GetType()
{
    return EntityType::ILLUSION_ILLAGER;
}

arrayWithLength<Vec3*> IllusionIllager::getIllusionOffsets(float partialTick)
{
    arrayWithLength<Vec3*> offsets(4, true);
    if (mIllusionLevel < 1)
    {
        offsets.data[0] = Vec3::newTemp(mDisguisedPositionO);
        offsets.data[1] = Vec3::newTemp(mDisguisedPosition2O);
        offsets.data[2] = Vec3::newTemp(mDisguisedPosition3O);
        offsets.data[3] = Vec3::newTemp(mDisguisedPosition4O);
        return offsets;
    }
    double factor = std::pow(static_cast<double>((mIllusionLevel - partialTick) / 3.0f), 0.25);
    double inverse = 1.0 - factor;
    Vec3* previous = mDisguisedPositionO->scale(inverse);
    offsets.data[0] = previous->add(mDisguisedPosition->scale(factor));
    offsets.data[1] = mDisguisedPosition2O->scale(inverse)->add(mDisguisedPosition2->scale(factor));
    offsets.data[2] = mDisguisedPosition3O->scale(inverse)->add(mDisguisedPosition3->scale(factor));
    offsets.data[3] = mDisguisedPosition4O->scale(inverse)->add(mDisguisedPosition4->scale(factor));
    return offsets;
}

void IllusionIllager::performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor)
{
    boost::shared_ptr<Arrow> arrow = getArrow(distanceFactor);
    double dx = target->mX - mX;
    double dy = target->mBoundingBox->mMinY + target->mBbHeight / 3.0f - arrow->mY;
    double dz = target->mZ - mZ;
    double horizontal = Mth::sqrt(dx * dx + dz * dz);
    arrow->shoot(dx, dy + horizontal * 0.2f, dz, 1.6f, (float)(14 - mLevel->getDifficulty()->getId() * 4));
    playSound(SoundEvents::SKELETON_SHOOT, 1.0f, 1.0f / (getRandom()->nextFloat() * 0.4f + 0.8f));
    mLevel->addEntity(arrow);
}

boost::shared_ptr<Arrow> IllusionIllager::getArrow(float distanceFactor)
{
    boost::shared_ptr<LivingEntity> owner = boost::static_pointer_cast<LivingEntity>(shared_from_this());
    boost::shared_ptr<TippableArrow> arrow(new TippableArrow(mLevel, owner));
    arrow->setEnchantmentEffectsFromEntity(owner, distanceFactor);
    return arrow;
}

IllusionIllager::IllusionistBlindnessSpellGoal::IllusionistBlindnessSpellGoal(IllusionIllager* illager)
    : SpellcasterUseSpellGoal(illager),
      mTargetId(0)
{
}
IllusionIllager::IllusionistBlindnessSpellGoal::~IllusionistBlindnessSpellGoal()
{
}
bool IllusionIllager::IllusionistBlindnessSpellGoal::canUse()
{
    if (!SpellcasterUseSpellGoal::canUse())
        return false;
    if (mSpellcaster->getTarget().get() == NULL)
        return false;
    if (mSpellcaster->getTarget()->getId() == mTargetId)
        return false;
    if (!mSpellcaster->mLevel->getCurrentDifficultyAt(BlockPos(mSpellcaster))
             .isHarderThan(static_cast<float>(Difficulty::NORMAL->getId())))
        return false;
    return true;
}
void IllusionIllager::IllusionistBlindnessSpellGoal::start()
{
    SpellcasterUseSpellGoal::start();
    mTargetId = mSpellcaster->getTarget()->getId();
}
void IllusionIllager::IllusionistBlindnessSpellGoal::performSpellCasting()
{
    mSpellcaster->getTarget()->addEffect(new MobEffectInstance(MobEffects::BLINDNESS, 400),
                                         boost::shared_ptr<Entity>());
}
int IllusionIllager::IllusionistBlindnessSpellGoal::getCastingTime()
{
    return 20;
}
int IllusionIllager::IllusionistBlindnessSpellGoal::getCastingInterval()
{
    return 180;
}
const SoundEvent* IllusionIllager::IllusionistBlindnessSpellGoal::getSpellPrepareSound()
{
    return SoundEvents::ILLUSION_ILLAGER_PREPARE_BLINDNESS;
}
SpellcasterIllager::IllagerSpell* IllusionIllager::IllusionistBlindnessSpellGoal::getSpell()
{
    return gSpell5;
}
IllusionIllager::IllusionistMirrorSpellGoal::IllusionistMirrorSpellGoal(IllusionIllager* illager)
    : SpellcasterUseSpellGoal(illager)
{
}
IllusionIllager::IllusionistMirrorSpellGoal::~IllusionistMirrorSpellGoal()
{
}
bool IllusionIllager::IllusionistMirrorSpellGoal::canUse()
{
    if (!SpellcasterUseSpellGoal::canUse())
        return false;
    if (mSpellcaster->hasEffect(MobEffects::INVISIBILITY))
        return false;
    return true;
}
void IllusionIllager::IllusionistMirrorSpellGoal::performSpellCasting()
{
    mSpellcaster->addEffect(new MobEffectInstance(MobEffects::INVISIBILITY, 1200), boost::shared_ptr<Entity>());
}
int IllusionIllager::IllusionistMirrorSpellGoal::getCastingTime()
{
    return 20;
}
int IllusionIllager::IllusionistMirrorSpellGoal::getCastingInterval()
{
    return 340;
}
const SoundEvent* IllusionIllager::IllusionistMirrorSpellGoal::getSpellPrepareSound()
{
    return SoundEvents::ILLUSION_ILLAGER_PREPARE_MIRROR;
}
SpellcasterIllager::IllagerSpell* IllusionIllager::IllusionistMirrorSpellGoal::getSpell()
{
    return gSpell4;
}
