#include "game/core/EvocationIllager.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/EntityType.hpp"
#include <algorithm>
#include <cmath>
#include <typeinfo>
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/EvocationFangs.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/core/Sheep.hpp"
#include "game/core/SnowMan.hpp"
#include "game/core/Vex.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/RandomStrollGoal.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/Monster.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"

extern SpellcasterIllager::IllagerSpell* gSpell1;
extern SpellcasterIllager::IllagerSpell* gSpell2;
extern SpellcasterIllager::IllagerSpell* gSpell3;

EvocationIllager::EvocationIllager(Level* level) : SpellcasterIllager(level), mWololoTarget()
{
    fjDerivedCtorCalls();
    setSize(0.6f, 1.95f);
    mExperienceReward = 10;
    mCategories |= EntityCategory::eEntityCategory_Villager | EntityCategory::eEntityCategory_Humanoid
                   | EntityCategory::eEntityCategory_Monster;
}
void EvocationIllager::registerGoals()
{
    Mob::registerGoals();
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, new AvoidEntityGoal(this, typeid(Player), 8.0f, 0.6, 1.0), true);
    mGoalSelector.addGoal(4, new EvokerSummonSpellGoal(this), true);
    mGoalSelector.addGoal(5, new EvokerAttackSpellGoal(this), true);
    mGoalSelector.addGoal(6, new EvokerWololoSpellGoal(this), true);
    mGoalSelector.addGoal(8, new RandomStrollGoal(this, 0.6), true);
    mGoalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f), true);

    arrayWithLength<unsigned long long> hurtByTargetTypes(1, true);
    hurtByTargetTypes[0] = EntityType::EVOCATION_ILLAGER;
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, hurtByTargetTypes), true);
    mTargetSelector.addGoal(
        2, (new NearestAttackableTargetGoal(this, typeid(Player), true, false, false))->setUnseenMemoryTicks(300),
        true);
    mTargetSelector.addGoal(
        3, (new NearestAttackableTargetGoal(this, typeid(Villager), true, false, false))->setUnseenMemoryTicks(300),
        true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(VillagerGolem), true, false, false), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(SnowMan), true, false, false), true);
}
void EvocationIllager::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.5);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(12.0);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(24.0);
}
void EvocationIllager::defineSynchedData()
{
    SpellcasterIllager::defineSynchedData();
}
void EvocationIllager::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::EVOCATION_ILLAGER);
}
void EvocationIllager::readAdditionalSaveData(CompoundTag* tag)
{
    SpellcasterIllager::readAdditionalSaveData(tag);
}
void EvocationIllager::addAdditonalSaveData(CompoundTag* tag)
{
    SpellcasterIllager::addAdditonalSaveData(tag);
    tag->putInt(L"SpellTicks", getSpellCastingTime());
}
ResourceLocation* EvocationIllager::getDefaultLootTable()
{
    return &BuiltInLootTables::EVOCATION_ILLAGER;
}
void EvocationIllager::customServerAiStep()
{
    SpellcasterIllager::customServerAiStep();
}
void EvocationIllager::tick()
{
    SpellcasterIllager::tick();
}
bool EvocationIllager::isAlliedTo(boost::shared_ptr<Entity> entity)
{
    if (!entity)
        return false;
    if (entity == shared_from_this())
        return true;
    if (Entity::isAlliedTo(entity))
        return true;
    if (eTYPE_DERIVED_FROM(EntityType::VEX, entity->GetType()))
    {
        return isAlliedTo(boost::static_pointer_cast<Vex>(entity)->getOwner());
    }
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType())
        && boost::static_pointer_cast<LivingEntity>(entity)->getMobType() == MOB_TYPE_ILLAGER)
    {
        return getTeam() == NULL && entity->getTeam() == NULL;
    }
    return false;
}
const SoundEvent* EvocationIllager::getAmbientSound()
{
    return SoundEvents::EVOCATION_ILLAGER_AMBIENT;
}
const SoundEvent* EvocationIllager::getDeathSound()
{
    return SoundEvents::EVOCATION_ILLAGER_DEATH;
}
const SoundEvent* EvocationIllager::getHurtSound(const DamageSource*)
{
    return SoundEvents::EVOCATION_ILLAGER_HURT;
}
void EvocationIllager::setWololoTarget(const boost::shared_ptr<Sheep>& target)
{
    mWololoTarget = target;
}
boost::shared_ptr<Sheep> EvocationIllager::getWololoTarget()
{
    return mWololoTarget.lock();
}
const SoundEvent* EvocationIllager::getCastingSoundEvent()
{
    return SoundEvents::EVOCATION_ILLAGER_CAST_SPELL;
}
Entity* EvocationIllager::create(Level* level)
{
    return new EvocationIllager(level);
}
unsigned long long EvocationIllager::GetType()
{
    return EntityType::EVOCATION_ILLAGER;
}

EvocationIllager::EvokerAttackSpellGoal::EvokerAttackSpellGoal(EvocationIllager* evoker)
    : SpellcasterUseSpellGoal(evoker)
{
}
EvocationIllager::EvokerAttackSpellGoal::~EvokerAttackSpellGoal()
{
}
std::wstring EvocationIllager::EvokerAttackSpellGoal::getName()
{
    return L"Evoker Attack Spell";
}
int EvocationIllager::EvokerAttackSpellGoal::getCastingTime()
{
    return 40;
}
int EvocationIllager::EvokerAttackSpellGoal::getCastingInterval()
{
    return 100;
}
const SoundEvent* EvocationIllager::EvokerAttackSpellGoal::getSpellPrepareSound()
{
    return SoundEvents::EVOCATION_ILLAGER_PREPARE_ATTACK;
}
SpellcasterIllager::IllagerSpell* EvocationIllager::EvokerAttackSpellGoal::getSpell()
{
    return gSpell2;
}
void EvocationIllager::EvokerAttackSpellGoal::createSpellEntity(double x, double z, double minY, double maxY,
                                                                float yRot, int warmupDelay)
{
    BlockPos pos(x, maxY, z);
    double yOffset = 0.0;
    Level* level = mSpellcaster->mLevel;
    bool found = false;
    do
    {
        if (!level->isSolidBlockingBlockInLoadedChunk(pos, true)
            && level->isSolidBlockingBlockInLoadedChunk(pos.below(), true))
        {
            if (!level->isEmptyBlock(pos))
            {
                const AABB* shape = level->getBlockState(pos)->getClipAABB(level, pos);
                if (shape != NULL)
                {
                    yOffset = shape->mMaxY;
                }
            }
            found = true;
            break;
        }
        pos = pos.below();
    } while (static_cast<double>(pos.y) >= std::floor(minY) - 1.0);
    if (found)
    {
        boost::shared_ptr<EvocationFangs> fangs(
            new EvocationFangs(level, x, static_cast<double>(pos.y) + yOffset, z, yRot, warmupDelay,
                               boost::static_pointer_cast<LivingEntity>(mSpellcaster->shared_from_this())));
        level->addEntity(fangs);
    }
}
void EvocationIllager::EvokerAttackSpellGoal::performSpellCasting()
{
    boost::shared_ptr<LivingEntity> target = mSpellcaster->getTarget();
    double minY = std::min(target->mY, mSpellcaster->mY);
    double maxY = std::max(target->mY, mSpellcaster->mY);
    maxY += 1.0;
    float angle = static_cast<float>(std::atan2(target->mZ - mSpellcaster->mZ, target->mX - mSpellcaster->mX));
    bool isNear = mSpellcaster->distanceToSqr(target) < 9.0;
    if (isNear)
    {
        for (int i = 0; i < 5; ++i)
        {
            float angle1 = angle + static_cast<float>(i) * 3.1415927f * 0.4f;
            float cosAngle = cosf(angle1);
            float sinAngle = sinf(angle1);
            createSpellEntity(cosAngle * 1.5 + mSpellcaster->mX, sinAngle * 1.5 + mSpellcaster->mZ, minY, maxY, angle1,
                              0);
        }
        for (int i = 0; i < 8; ++i)
        {
            float angle2 = angle + static_cast<float>(i) * 3.1415927f * 2.0f / 8.0f + 1.2566371f;
            float cosAngle = cosf(angle2);
            float sinAngle = sinf(angle2);
            createSpellEntity(cosAngle * 2.5 + mSpellcaster->mX, sinAngle * 2.5 + mSpellcaster->mZ, minY, maxY, angle2,
                              3);
        }
    }
    else
    {
        for (int i = 0; i < 16; ++i)
        {
            double radius = 1.25 * static_cast<double>(i + 1);
            float cosAngle = cosf(angle);
            float sinAngle = sinf(angle);
            createSpellEntity(cosAngle * radius + mSpellcaster->mX, sinAngle * radius + mSpellcaster->mZ, minY, maxY,
                              angle, i);
        }
    }
}

EvocationIllager::EvokerSummonSpellGoal::EvokerSummonSpellGoal(EvocationIllager* evoker)
    : SpellcasterUseSpellGoal(evoker)
{
}
EvocationIllager::EvokerSummonSpellGoal::~EvokerSummonSpellGoal()
{
}
bool EvocationIllager::EvokerSummonSpellGoal::canUse()
{
    if (!SpellcasterUseSpellGoal::canUse())
        return false;
    EvocationIllager* evoker = static_cast<EvocationIllager*>(mSpellcaster);
    AABB* area = evoker->mBoundingBox->grow(16.0);
    std::vector<boost::shared_ptr<Entity> >* vexes = evoker->mLevel->getEntitiesOfClass(typeid(Vex), area);
    int count = (int)vexes->size();
    vexes->clear();
    delete vexes;
    return mSpellcaster->mRandom->nextInt(8) + 1 > count;
}
std::wstring EvocationIllager::EvokerSummonSpellGoal::getName()
{
    return L"Evoker Summon Spell";
}
int EvocationIllager::EvokerSummonSpellGoal::getCastingTime()
{
    return 100;
}
int EvocationIllager::EvokerSummonSpellGoal::getCastingInterval()
{
    return 340;
}
void EvocationIllager::EvokerSummonSpellGoal::performSpellCasting()
{
    Level* level = mSpellcaster->mLevel;
    for (int i = 0; i < 3; ++i)
    {
        BlockPos* spawnPos = new BlockPos(
            BlockPos(mSpellcaster)
                .offset(mSpellcaster->mRandom->nextInt(5) - 2, 1, mSpellcaster->mRandom->nextInt(5) - 2));
        boost::shared_ptr<Vex> vex(new Vex(level));
        vex->moveTo(*spawnPos, 0.0f, 0.0f);
        vex->finalizeMobSpawn(level->getCurrentDifficultyAt(*spawnPos), NULL, std::wstring(L""));
        vex->setOwner(boost::static_pointer_cast<Mob>(mSpellcaster->shared_from_this()));
        vex->setBoundOrigin(spawnPos);
        vex->setLimitedLife(mSpellcaster->mRandom->nextInt(90) * 20 + 600);
        level->addEntity(vex);
    }
}
const SoundEvent* EvocationIllager::EvokerSummonSpellGoal::getSpellPrepareSound()
{
    return SoundEvents::EVOCATION_ILLAGER_PREPARE_SUMMON;
}
SpellcasterIllager::IllagerSpell* EvocationIllager::EvokerSummonSpellGoal::getSpell()
{
    return gSpell1;
}

EvocationIllager::EvokerWololoSpellGoal::EvokerWololoSpellGoal(EvocationIllager* evoker)
    : SpellcasterUseSpellGoal(evoker)
{
}
EvocationIllager::EvokerWololoSpellGoal::~EvokerWololoSpellGoal()
{
}
void EvocationIllager::EvokerWololoSpellGoal::tick()
{
    SpellcasterUseSpellGoal::tick();
    boost::shared_ptr<Sheep> target = static_cast<EvocationIllager*>(mSpellcaster)->getWololoTarget();
    if (target)
        mSpellcaster->getLookControl()->setLookAt(target, static_cast<float>(mSpellcaster->getMaxHeadYRot()),
                                                  static_cast<float>(mSpellcaster->getMaxHeadXRot()));
}
bool EvocationIllager::EvokerWololoSpellGoal::canUse()
{
    if (mSpellcaster->getTarget())
        return false;
    if (mSpellcaster->isCastingSpell() || mNextAttackTickCount > mSpellcaster->mTickCount)
        return false;
    if (!mSpellcaster->mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
        return false;

    SheepSelector selector;
    std::vector<boost::shared_ptr<Entity> >* sheep = mSpellcaster->mLevel->getEntitiesOfClass(
        typeid(Sheep), mSpellcaster->mBoundingBox->grow(16.0, 4.0, 16.0), &selector, false);
    if (sheep->empty())
    {
        delete sheep;
        return false;
    }
    {
        boost::shared_ptr<Sheep> target = boost::static_pointer_cast<Sheep>(
            sheep->at(mSpellcaster->mRandom->nextInt((int)sheep->size())));
        static_cast<EvocationIllager*>(mSpellcaster)->setWololoTarget(target);
    }
    delete sheep;
    return true;
}
bool EvocationIllager::EvokerWololoSpellGoal::canContinueToUse()
{
    return static_cast<EvocationIllager*>(mSpellcaster)->getWololoTarget()
           && static_cast<EvocationIllager*>(mSpellcaster)->isCastingSpell();
}
void EvocationIllager::EvokerWololoSpellGoal::stop()
{
    SpellcasterUseSpellGoal::stop();
    static_cast<EvocationIllager*>(mSpellcaster)->setWololoTarget(boost::shared_ptr<Sheep>());
}
std::wstring EvocationIllager::EvokerWololoSpellGoal::getName()
{
    return L"Wololo!";
}
void EvocationIllager::EvokerWololoSpellGoal::performSpellCasting()
{
    boost::shared_ptr<Sheep> sheep = static_cast<EvocationIllager*>(mSpellcaster)->getWololoTarget();
    if (sheep && sheep->isAlive())
        sheep->setColor(DyeColor::RED);
}
int EvocationIllager::EvokerWololoSpellGoal::getCastWarmupTime()
{
    return 40;
}
int EvocationIllager::EvokerWololoSpellGoal::getCastingTime()
{
    return 60;
}
int EvocationIllager::EvokerWololoSpellGoal::getCastingInterval()
{
    return 140;
}
const SoundEvent* EvocationIllager::EvokerWololoSpellGoal::getSpellPrepareSound()
{
    return SoundEvents::EVOCATION_ILLAGER_PREPARE_WOLOLO;
}
SpellcasterIllager::IllagerSpell* EvocationIllager::EvokerWololoSpellGoal::getSpell()
{
    return gSpell3;
}
bool EvocationIllager::EvokerWololoSpellGoal::SheepSelector::apply(boost::shared_ptr<Entity> entity) const
{
    return boost::static_pointer_cast<Sheep>(entity)->getColor() == DyeColor::BLUE;
}
