#include "game/core/Silverfish.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/MonsterEggBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/PIXNamedEvents.hpp"

Silverfish::Silverfish(Level* level) : Monster(level), mWakeUpFriendsGoal(this)
{
    fjDerivedCtorCalls();
    mCategories |= EntityCategory::eEntityCategory_Arthropod;
    setSize(0.4f, 0.3f);
}

Entity* Silverfish::create(Level* level)
{
    return new Silverfish(level);
}

void Silverfish::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::SILVERFISH);
}

void Silverfish::registerGoals()
{
    mGoalSelector.addGoal(1, new FloatGoal(this), true);
    mGoalSelector.addGoal(3, &mWakeUpFriendsGoal, false);
    mGoalSelector.addGoal(4, new MeleeAttackGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(5, new SilverfishMergeWithStoneGoal(this), true);
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(2, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
}

double Silverfish::getRidingHeight()
{
    return 0.1;
}

float Silverfish::getEyeHeight()
{
    return 0.1f;
}

void Silverfish::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(8.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.25);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(1.0);
}

bool Silverfish::makeStepSound()
{
    return true;
}

const SoundEvent* Silverfish::getAmbientSound()
{
    return SoundEvents::SILVERFISH_AMBIENT;
}

const SoundEvent* Silverfish::getHurtSound(const DamageSource* source)
{
    (void)source;
    return SoundEvents::SILVERFISH_HURT;
}

const SoundEvent* Silverfish::getDeathSound()
{
    return SoundEvents::SILVERFISH_DEATH;
}

void Silverfish::playStepSound(const BlockPos& pos, Block* block)
{
    playSound(SoundEvents::SILVERFISH_STEP, 0.15f, 1.0f);
}

bool Silverfish::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
    {
        return false;
    }
    if (dynamic_cast<const EntityDamageSource*>(source) != NULL || source == DamageSource::MAGIC)
    {
        mWakeUpFriendsGoal.notifyHurt();
    }
    return Monster::hurt(source, damage);
}

Item* Silverfish::getDeathLoot()
{
    return NULL;
}

void Silverfish::tick()
{
    PIXBeginNamedEvent(0.0f, "Silverfish tick");
    mYBodyRot = mYRot;
    Monster::tick();
    PIXEndNamedEvent();
}

void Silverfish::setYBodyRot(float yBodyRot)
{
    mYRot = yBodyRot;
    LivingEntity::setYBodyRot(yBodyRot);
}

float Silverfish::getWalkTargetValue(const BlockPos& pos)
{
    if (mLevel->getBlockState(pos.below())->getBlock() == Blocks::stone)
    {
        return 10.0f;
    }
    return Monster::getWalkTargetValue(pos);
}

bool Silverfish::isDarkEnoughToSpawn()
{
    return true;
}

bool Silverfish::checkSpawnRules()
{
    if (Monster::checkSpawnRules())
    {
        boost::shared_ptr<Player> nearestPlayer = mLevel->getNearestPlayer(shared_from_this(), 5.0, -1.0);
        return nearestPlayer == NULL;
    }
    return false;
}

EMobType Silverfish::getMobType()
{
    return MOB_TYPE_ARTHROPOD;
}

unsigned long long Silverfish::GetType()
{
    return EntityType::SILVERFISH;
}

Silverfish::SilverfishMergeWithStoneGoal::SilverfishMergeWithStoneGoal(Silverfish* silverfish)
    : RandomStrollGoal(silverfish, 1.0, 10)
{
    setRequiredControlFlags(1);
}

Silverfish::SilverfishMergeWithStoneGoal::~SilverfishMergeWithStoneGoal()
{
}

bool Silverfish::SilverfishMergeWithStoneGoal::canUse()
{
    if (mMob->getTarget())
    {
        return false;
    }
    if (!mMob->getNavigation()->isDone())
    {
        return false;
    }
    Random* random = mMob->getRandom();
    if (mMob->mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING) && random->nextInt(10) == 0)
    {
        mDirection = Direction::getRandomDirection(*random);
        BlockPos hostPos = BlockPos(mMob->mX, mMob->mY + 0.5, mMob->mZ).relative(mDirection);
        if (MonsterEggBlock::isCompatibleHostBlock(mMob->mLevel->getBlockState(hostPos)))
        {
            mShouldMerge = true;
            return true;
        }
    }
    mShouldMerge = false;
    return RandomStrollGoal::canUse();
}

bool Silverfish::SilverfishMergeWithStoneGoal::canContinueToUse()
{
    if (mShouldMerge)
        return false;
    return RandomStrollGoal::canContinueToUse();
}

void Silverfish::SilverfishMergeWithStoneGoal::start()
{
    if (!mShouldMerge)
    {
        RandomStrollGoal::start();
        return;
    }
    Level* level = mMob->mLevel;
    BlockPos mergePos = BlockPos(mMob->mX, mMob->mY + 0.5, mMob->mZ).relative(mDirection);
    const BlockState* host = level->getBlockState(mergePos);
    if (MonsterEggBlock::isCompatibleHostBlock(host))
    {
        level->setBlock(mergePos,
                        Blocks::monster_egg->defaultBlockState()->setValue(MonsterEggBlock::VARIANT,
                                                                           MonsterEggBlock::Variant::byHostBlock(host)),
                        Block::UPDATE_ALL, false);
        mMob->spawnAnim();
        mMob->remove();
    }
}

Silverfish::SilverfishWakeUpFriendsGoal::SilverfishWakeUpFriendsGoal(Silverfish* silverfish)
    : Goal(),
      mSilverfish(silverfish)
{
}

void Silverfish::SilverfishWakeUpFriendsGoal::notifyHurt()
{
    if (mTicksLeft == 0)
    {
        mTicksLeft = 20;
    }
}

bool Silverfish::SilverfishWakeUpFriendsGoal::canUse()
{
    return mTicksLeft > 0;
}

void Silverfish::SilverfishWakeUpFriendsGoal::tick()
{
    --mTicksLeft;
    if (mTicksLeft > 0)
    {
        return;
    }
    Level* level = mSilverfish->mLevel;
    Random* random = mSilverfish->getRandom();
    BlockPos center(mSilverfish->shared_from_this());
    for (int y = 0; y <= 5 && y >= -5; y = (y <= 0 ? 1 : 0) - y)
    {
        for (int x = 0; x <= 10 && x >= -10; x = (x <= 0 ? 1 : 0) - x)
        {
            for (int z = 0; z <= 10 && z >= -10; z = (z <= 0 ? 1 : 0) - z)
            {
                BlockPos pos = center.offset(x, y, z);
                const BlockState* state = level->getBlockState(pos);
                if (state->getBlock() == Blocks::monster_egg)
                {
                    if (level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
                    {
                        level->destroyBlock(pos, true);
                    }
                    else
                    {
                        level->setBlock(pos, state->getValue(MonsterEggBlock::VARIANT)->getHostBlockState(),
                                        Block::UPDATE_ALL, false);
                    }
                    if (random->nextBoolean())
                    {
                        return;
                    }
                }
            }
        }
    }
}
