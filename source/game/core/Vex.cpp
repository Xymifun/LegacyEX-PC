#include "game/core/Vex.hpp"
#include "game/core/EntityType.hpp"

#include "game/entity/mob/Mob.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/ai/LookControl.hpp"
#include "game/entity/ai/NearestAttackableTargetGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Mth.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include <cmath>

static const EntityDataAccessor<unsigned char>* DATA_FLAGS = NULL;
static const int FLAG_IS_CHARGING = 1;

Vex::Vex(Level* level) : Monster(level), owner()
{
    fjDerivedCtorCalls();
    mFireImmune = true;
    delete mMoveControl;
    mMoveControl = new VexMoveControl(this);
    setSize(0.4f, 0.8f);
    hasLimitedLife = false;
    limitedLife = 0;
    boundOrigin = 0;
    mExperienceReward = 3;
}

Vex::~Vex()
{
    delete boundOrigin;
}
Entity* Vex::create(Level* level)
{
    return new Vex(level);
}
void Vex::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId<unsigned char>(EntityType::VEX, EntityType::MONSTER,
                                                            EntityDataSerializers::BYTE);
}
void Vex::move(MoverType operation, double x, double y, double z, bool force)
{
    Entity::move(operation, x, y, z, false);
    checkInsideBlocks();
}
void Vex::tick()
{
    setNoGravity(true);
    mNoPhysics = true;
    Monster::tick();
    mNoPhysics = false;
    if (hasLimitedLife)
    {
        if (--limitedLife <= 0)
        {
            limitedLife = 20;
            hurt(DamageSource::STARVE, 1.0f);
        }
    }
}
void Vex::registerGoals()
{
    Mob::registerGoals();
    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(4, new VexChargeAttackGoal(this), true);
    mGoalSelector.addGoal(8, new VexRandomMoveGoal(this), true);
    mGoalSelector.addGoal(9, new LookAtPlayerGoal(this, typeid(Player), 3.0f, 1.0f), true);
    mGoalSelector.addGoal(10, new LookAtPlayerGoal(this, typeid(Mob), 8.0f), true);
    arrayWithLength<unsigned long long> entityTypes(1, true);
    entityTypes.data[0] = EntityType::VEX;
    mTargetSelector.addGoal(1, new HurtByTargetGoal(this, true, entityTypes), true);
    mTargetSelector.addGoal(2, new VexCopyOwnerTargetGoal(this), true);
    mTargetSelector.addGoal(3, new NearestAttackableTargetGoal(this, typeid(Player), true, false, false), true);
}
void Vex::registerAttributes()
{
    Monster::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(14.0);
    getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(4.0);
}
void Vex::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_FLAGS, static_cast<unsigned char>(0));
}
void Vex::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::VEX);
}
void Vex::setLimitedLife(int life)
{
    limitedLife = life;
    hasLimitedLife = true;
}
void Vex::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    if (tag->contains(L"BoundX"))
    {
        if (boundOrigin != 0)
            delete boundOrigin;
        boundOrigin = new BlockPos(tag->getInt(L"BoundX"), tag->getInt(L"BoundY"), tag->getInt(L"BoundZ"));
    }
    if (tag->contains(L"LifeTicks"))
        setLimitedLife(tag->getInt(L"LifeTicks"));
}
void Vex::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    if (boundOrigin != 0)
    {
        tag->putInt(L"BoundX", boundOrigin->x);
        tag->putInt(L"BoundY", boundOrigin->y);
        tag->putInt(L"BoundZ", boundOrigin->z);
    }
    if (hasLimitedLife)
        tag->putInt(L"LifeTicks", limitedLife);
}
boost::shared_ptr<Mob> Vex::getOwner()
{
    return owner.lock();
}
BlockPos* Vex::getBoundOrigin()
{
    return boundOrigin;
}
void Vex::setBoundOrigin(BlockPos* origin)
{
    delete boundOrigin;
    boundOrigin = origin;
}
bool Vex::getVexFlag(int flag)
{
    return (mEntityData->get(DATA_FLAGS) & flag) != 0;
}
void Vex::setVexFlag(int flag, bool value)
{
    int flags = mEntityData->get(DATA_FLAGS);
    if (value)
        flags |= flag;
    else
        flags &= ~flag;
    mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags), true);
}
bool Vex::isCharging()
{
    return getVexFlag(FLAG_IS_CHARGING);
}
void Vex::setIsCharging(bool value)
{
    setVexFlag(FLAG_IS_CHARGING, value);
}
void Vex::setOwner(const boost::shared_ptr<Mob>& owner)
{
    this->owner = owner;
}
const SoundEvent* Vex::getAmbientSound()
{
    return SoundEvents::VEX_AMBIENT;
}
const SoundEvent* Vex::getDeathSound()
{
    return SoundEvents::VEX_DEATH;
}
const SoundEvent* Vex::getHurtSound(const DamageSource*)
{
    return SoundEvents::VEX_HURT;
}
ResourceLocation* Vex::getDefaultLootTable()
{
    return &BuiltInLootTables::VEX;
}
ResourceLocation* Vex::getDefaultGearTable()
{
    return &BuiltInLootTables::VEX_GEAR;
}
int Vex::getLightColor()
{
    return 0xf000f0;
}
float Vex::getBrightness()
{
    return 1.0f;
}
MobGroupData* Vex::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                    const std::wstring& reason)
{
    populateDefaultEquipmentSlots(difficulty);
    populateDefaultEquipmentEnchantments(difficulty);
    return Mob::finalizeMobSpawn(difficulty, data, reason);
}
void Vex::populateDefaultEquipmentSlots(const DifficultyInstance& difficulty)
{
    Mob::populateDefaultEquipmentSlots(difficulty);
    setDropChance(EquipmentSlot::MAINHAND, 0.0f);
}
unsigned long long Vex::GetType()
{
    return EntityType::VEX;
}

Vex::VexChargeAttackGoal::VexChargeAttackGoal(Vex* value) : Goal(), owner(value)
{
    setRequiredControlFlags(1);
}
Vex::VexChargeAttackGoal::~VexChargeAttackGoal()
{
}
bool Vex::VexChargeAttackGoal::canUse()
{
    if (owner->getTarget() != 0 && !owner->getMoveControl()->hasWanted() && owner->mRandom->nextInt(7) == 0)
        return owner->distanceToSqr(owner->getTarget()) > 4.0;
    return false;
}
bool Vex::VexChargeAttackGoal::canContinueToUse()
{
    return owner->getMoveControl()->hasWanted() && owner->isCharging() && owner->getTarget() != 0
           && owner->getTarget()->isAlive();
}
void Vex::VexChargeAttackGoal::start()
{
    boost::shared_ptr<LivingEntity> target = owner->getTarget();
    Vec3* eye = target->getEyePosition(1.0f);
    owner->mMoveControl->setWantedPosition(eye->x, eye->y, eye->z, 1.0);
    owner->setIsCharging(true);
    owner->playSound(SoundEvents::VEX_CHARGE, 1.0f, 1.0f);
}
void Vex::VexChargeAttackGoal::stop()
{
    owner->setIsCharging(false);
}
void Vex::VexChargeAttackGoal::tick()
{
    boost::shared_ptr<LivingEntity> target = owner->getTarget();
    if (owner->mBoundingBox->intersects(target->mBoundingBox))
    {
        owner->doHurtTarget(target);
        owner->setIsCharging(false);
    }
    else
    {
        const double distance = owner->distanceToSqr(target);
        if (distance < 9.0)
        {
            Vec3* eye = target->getEyePosition(1.0f);
            owner->mMoveControl->setWantedPosition(eye->x, eye->y, eye->z, 1.0);
        }
    }
}
std::wstring Vex::VexChargeAttackGoal::getName()
{
    return L"Vex Charging Attack";
}

Vex::VexCopyOwnerTargetGoal::VexCopyOwnerTargetGoal(PathfinderMob* value) : TargetGoal(value, false)
{
}
bool Vex::VexCopyOwnerTargetGoal::canUse()
{
    Vex* vex = static_cast<Vex*>(mMob);
    boost::shared_ptr<Mob> owner = vex->getOwner();
    return owner != 0 && owner->getTarget() != 0 && canAttack(owner->getTarget(), false);
}
void Vex::VexCopyOwnerTargetGoal::start()
{
    boost::shared_ptr<Mob> owner = static_cast<Vex*>(mMob)->getOwner();
    static_cast<Vex*>(mMob)->setTarget(owner->getTarget());
    TargetGoal::start();
}
std::wstring Vex::VexCopyOwnerTargetGoal::getName()
{
    return L"Vex Copy Owner Target";
}

Vex::VexMoveControl::VexMoveControl(Vex* value) : MoveControl(value)
{
}
Vex::VexMoveControl::~VexMoveControl()
{
}
void Vex::VexMoveControl::tick()
{
    Vex* vex = static_cast<Vex*>(mMob);
    if (mOperation != MOVE_TO)
        return;
    const double dx = mWantedX - vex->mX;
    const double dy = mWantedY - vex->mY;
    const double dz = mWantedZ - vex->mZ;
    const double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (distance < vex->mBoundingBox->getSize())
    {
        mOperation = WAIT;
        vex->mXd *= 0.5;
        vex->mYd *= 0.5;
        vex->mZd *= 0.5;
        return;
    }
    vex->mXd += dx / distance * 0.05 * mSpeedModifier;
    vex->mYd += dy / distance * 0.05 * mSpeedModifier;
    vex->mZd += dz / distance * 0.05 * mSpeedModifier;
    if (!vex->getTarget())
    {
        const float rotation = -static_cast<float>(std::atan2(vex->mXd, vex->mZd)) * Mth::RAD_TO_DEG;
        vex->mYRot = rotation;
        vex->mYBodyRot = rotation;
    }
    else
    {
        const double targetDx = vex->getTarget()->mX - vex->mX;
        const double targetDz = vex->getTarget()->mZ - vex->mZ;
        const float rotation = -static_cast<float>(std::atan2(targetDx, targetDz)) * Mth::RAD_TO_DEG;
        vex->mYRot = rotation;
        vex->mYBodyRot = rotation;
    }
}

Vex::VexRandomMoveGoal::VexRandomMoveGoal(Vex* value) : Goal(), owner(value)
{
    setRequiredControlFlags(1);
}
Vex::VexRandomMoveGoal::~VexRandomMoveGoal()
{
}
bool Vex::VexRandomMoveGoal::canUse()
{
    return !owner->getMoveControl()->hasWanted() && owner->mRandom->nextInt(7) == 0;
}
bool Vex::VexRandomMoveGoal::canContinueToUse()
{
    return false;
}
void Vex::VexRandomMoveGoal::tick()
{
    const BlockPos* origin = owner->getBoundOrigin();
    if (origin == 0)
        origin = new BlockPos(owner);
    Level* level = owner->mLevel;
    Random* random = owner->mRandom;
    for (int attempt = 0; attempt != 3; ++attempt)
    {
        const BlockPos candidate = origin->offset(random->nextInt(15) - 7, random->nextInt(11) - 5,
                                                  random->nextInt(15) - 7);
        if (level->isEmptyBlock(candidate))
        {
            owner->mMoveControl->setWantedPosition(candidate.x + 0.5, candidate.y + 0.5, candidate.z + 0.5, 0.25);
            if (owner->getTarget() == NULL)
                owner->getLookControl()->setLookAt(candidate.x + 0.5, candidate.y + 0.5, candidate.z + 0.5, 180.0f,
                                                   20.0f);
            break;
        }
    }
}
std::wstring Vex::VexRandomMoveGoal::getName()
{
    return L"Vex Random Move";
}
