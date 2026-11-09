#include "game/core/EnderMan.hpp"
#include "game/entity/mob/IndirectEntityDamageSource.hpp"
#include "game/core/IsLookingAtMePredicate.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Endermite.hpp"
#include "game/core/Mth.hpp"

#include <cmath>
#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/MeleeAttackGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPathTypes.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/core/PIXNamedEvents.hpp"

static const EntityDataAccessor<const BlockState*>* DATA_CARRIED_BLOCK = NULL;
static const EntityDataAccessor<bool>* DATA_CREEPY = NULL;
static boost::unordered_set<Block*> sCarriableBlocks;

AttributeModifier* EnderMan::SPEED_MODIFIER_ATTACKING =
    (new AttributeModifier(eMODIFIER_ID_EnderManAttackingSpeedBoost, 0.15f, 0))->setSerialize(false);

EnderMan::EnderMan(Level* level) : Monster(level)
{
    fjDerivedCtorCalls();
    mLastCreepySound = 0;
    mTargetChangeTime = 0;
    setSize(0.6f, 2.9f);
    mMaxUpStep = 1.0f;
    setPathfindingMalus(BlockPathTypes::WATER, -1.0f);
}

void EnderMan::staticCtor()
{
    DATA_CARRIED_BLOCK = SynchedEntityData::defineId<const BlockState*>(EntityType::ENDER_MAN, EntityType::MONSTER,
                                                                        EntityDataSerializers::BLOCK_STATE);
    DATA_CREEPY = SynchedEntityData::defineId<bool>(EntityType::ENDER_MAN, EntityType::MONSTER,
                                                    EntityDataSerializers::BOOLEAN);
    sCarriableBlocks.insert(Blocks::grass);
    sCarriableBlocks.insert(Blocks::dirt);
    sCarriableBlocks.insert(Blocks::sand);
    sCarriableBlocks.insert(Blocks::gravel);
    sCarriableBlocks.insert(Blocks::yellow_flower);
    sCarriableBlocks.insert(Blocks::red_flower);
    sCarriableBlocks.insert(Blocks::brown_mushroom);
    sCarriableBlocks.insert(Blocks::red_mushroom);
    sCarriableBlocks.insert(Blocks::tnt);
    sCarriableBlocks.insert(Blocks::cactus);
    sCarriableBlocks.insert(Blocks::clay);
    sCarriableBlocks.insert(Blocks::pumpkin);
    sCarriableBlocks.insert(Blocks::melon_block);
    sCarriableBlocks.insert(Blocks::mycelium);
    sCarriableBlocks.insert(Blocks::netherrack);
}

void EnderMan::registerGoals()
{
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(2, new MeleeAttackGoal(this, 1.0, false), true);
    mGoalSelector.addGoal(7, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(8, new LookAtPlayerGoal(this, typeid(Player), 8.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
    mGoalSelector.addGoal(10, new EndermanLeaveBlockGoal(this), true);
    mGoalSelector.addGoal(11, new EndermanTakeBlockGoal(this), true);
    mTargetSelector.addGoal(1, new EndermanLookForPlayerGoal(this), true);
    mTargetSelector.addGoal(2, new HurtByTargetGoal(this, false, arrayWithLength<unsigned long long>()), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Endermite), true, false, false), true);
}

void EnderMan::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(40.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.3f);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(7.0);
    getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->setBaseValue(64.0);
}

void EnderMan::setTarget(boost::shared_ptr<LivingEntity> target)
{
    Mob::setTarget(target);
    AttributeInstance* speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
    if (!target)
    {
        mTargetChangeTime = 0;
        mEntityData->set(DATA_CREEPY, false, true);
        speed->removeModifier(SPEED_MODIFIER_ATTACKING);
    }
    else
    {
        mTargetChangeTime = mTickCount;
        mEntityData->set(DATA_CREEPY, true, true);
        if (!speed->hasModifier(SPEED_MODIFIER_ATTACKING))
            speed->addModifier(new AttributeModifier(*SPEED_MODIFIER_ATTACKING));
    }
}

void EnderMan::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem<const BlockState*>(DATA_CARRIED_BLOCK, NULL);
    mEntityData->createDateItem<bool>(DATA_CREEPY, false);
}

void EnderMan::playCreepySound()
{
    if (mTickCount >= mLastCreepySound + 400)
    {
        mLastCreepySound = mTickCount;
        if (!isSilent())
        {
            mLevel->playLocalSound(mX, mY + getEyeHeight(), mZ, SoundEvents::ENDERMEN_STARE, getSoundSource(), 2.5f,
                                   1.0f, false, 16.0f);
        }
    }
}

bool EnderMan::isCreepy()
{
    return mEntityData->get(DATA_CREEPY);
}

void EnderMan::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    if (DATA_CREEPY->getId() == accessor->getId() && isCreepy() && mLevel->mIsClientSide)
        playCreepySound();
    LivingEntity::onSyncedDataUpdated(accessor);
}

void EnderMan::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::ENDER_MAN);
}

const BlockState* EnderMan::getCarryingBlock()
{
    return mEntityData->get(DATA_CARRIED_BLOCK);
}

void EnderMan::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    const BlockState* state = getCarryingBlock();
    if (state != NULL)
    {
        tag->putShort(L"carried", (short)Block::getId(state->getBlock()));
        tag->putShort(L"carriedData", (short)state->getBlock()->convertBlockStateToLegacyData(state));
    }
}

void EnderMan::setCarryingBlock(const BlockState* state)
{
    mEntityData->set(DATA_CARRIED_BLOCK, state, true);
}

void EnderMan::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    const BlockState* state;
    if (tag->contains(L"carried", Tag::TAG_STRING))
        state = Block::byString(tag->getString(L"carried"))->getBlockState(tag->getShort(L"carriedData") & 0xffff);
    else
        state = Block::byId(tag->getShort(L"carried"))->getBlockState(tag->getShort(L"carriedData") & 0xffff);
    if (state == NULL || state->getBlock() == NULL || state->getMaterial() == Material::AIR)
        state = NULL;
    setCarryingBlock(state);
}

bool EnderMan::isLookingAtMe(boost::shared_ptr<Player> player)
{
    not_null_ptr<ItemInstance> helmet = player->mInventory->m_armor[3];
    if (helmet->getItem() == Item::byBlock(Blocks::carved_pumpkin))
        return false;

    Vec3* view = player->getViewVector(1.0f)->normalize();
    Vec3* delta = Vec3::newTemp(
        mX - player->mX, mBoundingBox->mMinY + getEyeHeight() - (player->mY + player->getEyeHeight()), mZ - player->mZ);
    const double distance = delta->length();
    Vec3* toEnderMan = delta->normalize();
    if (view->dot(toEnderMan) > 1.0 - 0.025 / distance)
        return player->canSee(shared_from_this());
    return false;
}

float EnderMan::getEyeHeight()
{
    float eyeHeight = 2.55f;
    return eyeHeight;
}

void EnderMan::aiStep()
{
    if (mLevel->mIsClientSide)
    {
        for (int i = 0; i < 2; ++i)
        {
            mLevel->addParticle(PARTICLE_ENDER, mX + (mRandom->nextDouble() - 0.5) * (double)mBbWidth,
                                mY + mRandom->nextDouble() * (double)mBbHeight - 0.25,
                                mZ + (mRandom->nextDouble() - 0.5) * (double)mBbWidth,
                                (mRandom->nextDouble() - 0.5) * 2.0, -mRandom->nextDouble(),
                                (mRandom->nextDouble() - 0.5) * 2.0, arrayWithLength<int>());
        }
    }
    mJumping = false;
    Monster::aiStep();
}

bool EnderMan::teleport(double x, double y, double z)
{
    const bool teleported = randomTeleport(x, y, z);
    if (teleported)
    {
        mLevel->playSound(boost::shared_ptr<Player>(), mXo, mYo, mZo, SoundEvents::ENDERMEN_TELEPORT, getSoundSource(),
                          1.0f, 1.0f, 16.0f);
        playSound(SoundEvents::ENDERMEN_TELEPORT, 1.0f, 1.0f);
    }
    return teleported;
}

bool EnderMan::teleport()
{
    double range = getAttribute(SharedMonsterAttributes::FOLLOW_RANGE)->getValue();
    double x = mX + (mRandom->nextDouble() - 0.5) * range;
    double y = mY + std::floor((mRandom->nextDouble() - 0.5) * range);
    double z = mZ + (mRandom->nextDouble() - 0.5) * range;
    return teleport(x, y, z);
}

void EnderMan::customServerAiStep()
{
    if (isInWaterRainOrBubble())
        hurt(DamageSource::DROWN, 1.0f);
    if (mLevel->isDay() && mTickCount >= mTargetChangeTime + 600)
    {
        float brightness = (float)getBrightness();
        if (brightness > 0.5f)
        {
            int shouldTeleport = mLevel->canSeeSky(BlockPos(shared_from_this()))
                                 && mRandom->nextFloat() * 30.0f < 2.0f * (brightness - 0.4f);
            if (shouldTeleport)
            {
                setTarget(boost::shared_ptr<LivingEntity>());
                teleport();
            }
        }
    }
    Mob::customServerAiStep();
}

bool EnderMan::teleportTowards(boost::shared_ptr<Entity> entity)
{
    Vec3* direction = Vec3::newTemp(mX - entity->mX,
                                    mBoundingBox->mMinY + mBbHeight / 2.0f - entity->mY + entity->getEyeHeight(),
                                    mZ - entity->mZ)
                          ->normalize();
    double x = mX + (mRandom->nextDouble() - 0.5) * 8.0 - direction->x * 16.0;
    double y = mY + (mRandom->nextInt(16) - 8) - direction->y * 16.0;
    double z = mZ + (mRandom->nextDouble() - 0.5) * 8.0 - direction->z * 16.0;
    return teleport(x, y, z);
}

const SoundEvent* EnderMan::getAmbientSound()
{
    return isCreepy() ? SoundEvents::ENDERMEN_SCREAM : SoundEvents::ENDERMEN_AMBIENT;
}
const SoundEvent* EnderMan::getHurtSound(const DamageSource*)
{
    return SoundEvents::ENDERMEN_HURT;
}
const SoundEvent* EnderMan::getDeathSound()
{
    return SoundEvents::ENDERMEN_DEATH;
}
void EnderMan::dropEquipment(bool wasRecentlyHit, int lootingModifier)
{
    Monster::dropEquipment(wasRecentlyHit, lootingModifier);
    const BlockState* state = getCarryingBlock();
    if (state != NULL)
    {
        Item* item = Item::byBlock(state->getBlock());
        ItemInstance* stack;
        if (item->isStackedByData())
            stack = new ItemInstance(item, 1, state->getBlock()->convertBlockStateToLegacyData(state));
        else
            stack = new ItemInstance(item, 1, 0);
        spawnAtLocation(not_null_ptr<ItemInstance>(stack), 0.0f);
    }
}
ResourceLocation* EnderMan::getDefaultLootTable()
{
    return &BuiltInLootTables::ENDERMAN;
}

bool EnderMan::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
        return false;

    if (dynamic_cast<const IndirectEntityDamageSource*>(source) != NULL)
    {
        for (int attempt = 0; attempt < 64; ++attempt)
        {
            if (teleport())
                return true;
        }
        return false;
    }

    bool hurt = Monster::hurt(source, damage);
    if (!mLevel->mIsClientSide && mRandom->rollPercentageChance(90) && isAlive())
        teleport();
    return hurt;
}

Entity* EnderMan::create(Level* level)
{
    return new EnderMan(level);
}
unsigned long long EnderMan::GetType()
{
    return EntityType::ENDER_MAN;
}

EnderMan::EndermanLeaveBlockGoal::EndermanLeaveBlockGoal(EnderMan* enderMan) : mEnderMan(enderMan)
{
}
EnderMan::EndermanLeaveBlockGoal::~EndermanLeaveBlockGoal()
{
}
bool EnderMan::EndermanLeaveBlockGoal::canUse()
{
    if (mEnderMan->getCarryingBlock() == NULL)
        return false;
    if (!mEnderMan->mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
        return false;
    if (mEnderMan->getRandom()->nextInt(2000) != 0)
        return false;
    return true;
}
bool EnderMan::EndermanLeaveBlockGoal::canPlaceBlock(Level* level, const BlockPos& pos, Block* block,
                                                     const BlockState* oldState, const BlockState* newState)
{
    if (!block->mayPlace(level, pos))
        return false;
    if (oldState->getMaterial() != Material::AIR)
        return false;
    if (newState->getMaterial() == Material::AIR)
        return false;
    if (!newState->isCubeShaped())
        return false;
    return true;
}

void EnderMan::EndermanLeaveBlockGoal::tick()
{
    Random* random = mEnderMan->getRandom();
    Level* level = mEnderMan->mLevel;
    int x = Mth::floor(mEnderMan->mX - 1.0 + random->nextDouble() * 2.0);
    int y = Mth::floor(mEnderMan->mY + random->nextDouble() * 2.0);
    int z = Mth::floor(mEnderMan->mZ - 1.0 + random->nextDouble() * 2.0);
    BlockPos pos(x, y, z);
    const BlockState* state = level->getBlockState(pos);
    BlockPos below = pos.below();
    const BlockState* belowState = level->getBlockState(below);
    const BlockState* carried = mEnderMan->getCarryingBlock();
    if (carried != NULL && canPlaceBlock(level, pos, carried->getBlock(), state, belowState))
    {
        level->setBlock(pos, mEnderMan->getCarryingBlock(), Block::UPDATE_ALL, false);
        mEnderMan->setCarryingBlock(NULL);
    }
}
std::wstring EnderMan::EndermanLeaveBlockGoal::getName()
{
    return std::wstring(L"EndermanLeaveBlock");
}

EnderMan::EndermanLookForPlayerGoal::EndermanLookForPlayerGoal(EnderMan* enderMan)
    : NearestAttackableTargetGoal(enderMan, typeid(Player), true, false, false),
      mEnderMan(enderMan)
{
}

bool EnderMan::EndermanLookForPlayerGoal::canUse()
{
    EnderMan* enderMan = mEnderMan;
    const double range = getFollowDistance();
    IsLookingAtMePredicate predicate;
    predicate.mEnderMan = enderMan;
    mPendingTarget = boost::static_pointer_cast<Player>(enderMan->mLevel->getNearestAttackablePlayer(
        enderMan->mX, enderMan->mY, enderMan->mZ, range, range, NULL, &predicate));
    return mPendingTarget.get() != NULL;
}

void EnderMan::EndermanLookForPlayerGoal::start()
{
    mAggroTime = 5;
    mTeleportTime = 0;
}

void EnderMan::EndermanLookForPlayerGoal::stop()
{
    mPendingTarget = boost::shared_ptr<Player>();
    TargetGoal::stop();
}

bool EnderMan::EndermanLookForPlayerGoal::canContinueToUse()
{
    if (mPendingTarget != NULL)
    {
        if (!mEnderMan->isLookingAtMe(mPendingTarget))
            return false;
        mEnderMan->lookAt(boost::shared_ptr<Entity>(mPendingTarget), 10.0f, 10.0f);
        return true;
    }
    return NearestAttackableTargetGoal::canContinueToUse();
}

void EnderMan::EndermanLookForPlayerGoal::tick()
{
    if (mPendingTarget != NULL)
    {
        if (--mAggroTime > 0)
            return;
        NearestAttackableTargetGoal::mTarget = mPendingTarget;
        mPendingTarget = boost::shared_ptr<Player>();
        NearestAttackableTargetGoal::start();
        return;
    }

    boost::shared_ptr<Entity> target = NearestAttackableTargetGoal::mTarget.lock();
    if (target != NULL)
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, target->GetType())
            && mEnderMan->isLookingAtMe(boost::static_pointer_cast<Player>(target)))
        {
            if (target->distanceToSqr(mEnderMan->shared_from_this()) < 16.0)
                mEnderMan->teleport();
            mTeleportTime = 0;
        }
        else
        {
            bool tooFar = target->distanceToSqr(mEnderMan->shared_from_this()) > 256.0;
            if (tooFar && mTeleportTime++ >= 30)
            {
                if (mEnderMan->teleportTowards(target))
                    mTeleportTime = 0;
            }
        }
    }
    Goal::tick();
}

EnderMan::EndermanTakeBlockGoal::EndermanTakeBlockGoal(EnderMan* enderMan) : mEnderMan(enderMan)
{
}
EnderMan::EndermanTakeBlockGoal::~EndermanTakeBlockGoal()
{
}
bool EnderMan::EndermanTakeBlockGoal::canUse()
{
    if (mEnderMan->getCarryingBlock() != NULL)
        return false;
    if (!mEnderMan->mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
        return false;
    if (mEnderMan->getRandom()->nextInt(20) != 0)
        return false;
    return true;
}
void EnderMan::EndermanTakeBlockGoal::tick()
{
    Random* random = mEnderMan->getRandom();
    Level* level = mEnderMan->mLevel;
    int x = Mth::floor(mEnderMan->mX - 2.0 + random->nextDouble() * 4.0);
    int y = Mth::floor(mEnderMan->mY + random->nextDouble() * 3.0);
    int z = Mth::floor(mEnderMan->mZ - 2.0 + random->nextDouble() * 4.0);
    BlockPos pos(x, y, z);
    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    Vec3* from = Vec3::newTemp(std::floor(mEnderMan->mX) + 0.5, static_cast<float>(y) + 0.5f,
                               std::floor(mEnderMan->mZ) + 0.5);
    Vec3* to = Vec3::newTemp(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f, static_cast<float>(z) + 0.5f);
    HitResult* hit = level->clip(from, to, false, true, false, false, NULL, false);
    bool hitMatchesPos = hit != NULL && hit->getBlockPos().equals(pos);
    bool carriable = sCarriableBlocks.find(block) != sCarriableBlocks.end();
    if (carriable && hitMatchesPos)
    {
        mEnderMan->setCarryingBlock(state);
        level->removeBlock(pos);
    }
    if (hit != NULL)
        delete hit;
}
std::wstring EnderMan::EndermanTakeBlockGoal::getName()
{
    return L"EndermanTakeBlock";
}
