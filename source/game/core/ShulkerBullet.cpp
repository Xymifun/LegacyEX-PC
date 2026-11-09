#include "game/core/ShulkerBullet.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <typeinfo>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/core/HitResult.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/projectile/ProjectileUtil.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/Level.hpp"
#include "game/util/AABB.hpp"
#include "game/world/generation/Player.hpp"

ShulkerBullet::ShulkerBullet(Level* level)
    : Entity(level, true),
      mOwner(),
      mTarget(),
      mOwnerUniqueId(),
      mOwnerPos(),
      mTargetUniqueId(),
      mTargetPos()
{
    init();
    setSize(0.3125f, 0.3125f);
    mNoPhysics = true;
}

ShulkerBullet::ShulkerBullet(Level* level, double x, double y, double z, double xd, double yd, double zd)
    : Entity(level, true),
      mOwner(),
      mTarget(),
      mOwnerUniqueId(),
      mOwnerPos(),
      mTargetUniqueId(),
      mTargetPos()
{
    init();
    moveTo(x, y, z, mYRot, mXRot);
    mXd = xd;
    mYd = yd;
    mZd = zd;
}

ShulkerBullet::ShulkerBullet(Level* level, boost::shared_ptr<LivingEntity> owner, boost::shared_ptr<Entity> target,
                             const Direction::Axis* axis)
    : Entity(level, true),
      mOwner(),
      mTarget(),
      mOwnerUniqueId(),
      mOwnerPos(),
      mTargetUniqueId(),
      mTargetPos()
{
    init();
    mOwner = owner;
    BlockPos pos(boost::static_pointer_cast<Entity>(owner));
    moveTo(pos.x + 0.5, pos.y + 0.5, pos.z + 0.5, mYRot, mXRot);
    mTarget = target;
    mMoveDirection = Direction::UP;
    selectNextMoveDirection(axis);
}

Entity* ShulkerBullet::create(Level* level)
{
    return new ShulkerBullet(level);
}

void ShulkerBullet::init()
{
    fjDerivedCtorCalls();
    mTargetUniqueId = L"";
    mOwner = boost::shared_ptr<LivingEntity>();
    mTarget = boost::shared_ptr<Entity>();
    mMoveDirection = Direction::UP;
    mSteps = 0;
    mPowerX = 0.0;
    mPowerY = 0.0;
    mPowerZ = 0.0;
    mOwnerUniqueId = L"";
    mOwnerPos = BlockPos(0, 0, 0);
    mTargetUniqueId = L"";
    mTargetPos = BlockPos(0, 0, 0);
    defineSynchedData();
}

void ShulkerBullet::setMoveDirection(const Direction* direction)
{
    mMoveDirection = direction;
}

void ShulkerBullet::selectNextMoveDirection(const Direction::Axis* axis)
{
    BlockPos destination(0, 0, 0);
    double heightOffset = 0.5;
    if (mTarget == NULL)
    {
        destination = BlockPos(this).below();
    }
    else
    {
        heightOffset = mTarget->mBbHeight * 0.5;
        destination = BlockPos((int)mTarget->mX, (int)mTarget->mY + (int)heightOffset, (int)mTarget->mZ);
    }

    double targetX = destination.x + 0.5;
    double targetY = destination.y + heightOffset;
    double targetZ = destination.z + 0.5;
    const Direction* direction = NULL;
    if (destination.distSqrToCenter(mX, mY, mZ) >= 4.0)
    {
        BlockPos current(this);
        std::vector<const Direction*> choices;
        if (axis != Direction::Axis::X)
        {
            if (current.x < destination.x && mLevel->isEmptyBlock(current.east()))
                choices.push_back(Direction::EAST);
            else if (current.x > destination.x && mLevel->isEmptyBlock(current.west()))
                choices.push_back(Direction::WEST);
        }
        if (axis != Direction::Axis::Y)
        {
            if (current.y < destination.y && mLevel->isEmptyBlock(current.above()))
                choices.push_back(Direction::UP);
            else if (current.y > destination.y && mLevel->isEmptyBlock(current.below()))
                choices.push_back(Direction::DOWN);
        }
        if (axis != Direction::Axis::Z)
        {
            if (current.z < destination.z && mLevel->isEmptyBlock(current.south()))
                choices.push_back(Direction::SOUTH);
            else if (current.z > destination.z && mLevel->isEmptyBlock(current.north()))
                choices.push_back(Direction::NORTH);
        }

        direction = Direction::getRandomDirection(*mRandom);
        if (choices.empty())
        {
            for (int attempts = 5; !mLevel->isEmptyBlock(current.relative(direction)) && attempts > 0; --attempts)
                direction = Direction::getRandomDirection(*mRandom);
        }
        else
        {
            direction = choices.at(mRandom->nextInt((int)choices.size()));
        }

        targetX = mX + direction->getStepX();
        targetY = mY + direction->getStepY();
        targetZ = mZ + direction->getStepZ();
    }

    setMoveDirection(direction);
    double dx = targetX - mX;
    double dy = targetY - mY;
    double dz = targetZ - mZ;
    double length = Mth::sqrt(dx * dx + dy * dy + dz * dz);
    if (length == 0.0)
    {
        mPowerX = 0.0;
        mPowerY = 0.0;
        mPowerZ = 0.0;
    }
    else
    {
        mPowerX = dx / length * 0.15;
        mPowerY = dy / length * 0.15;
        mPowerZ = dz / length * 0.15;
    }
    mHasImpulse = true;
    mSteps = 10 + mRandom->nextInt(5) * 10;
}

void ShulkerBullet::onHit(const HitResult* result)
{
    if (!result->mEntity)
    {
        static_cast<ServerLevel*>(mLevel)->sendParticles(PARTICLE_LARGE_EXPLODE, mX, mY, mZ, 2, 0.2, 0.2, 0.2, 0.0,
                                                         arrayWithLength<int>());
        playSound(SoundEvents::SHULKER_BULLET_HIT, 1.0f, 1.0f);
    }
    else
    {
        DamageSource* damageSource = DamageSource::CreateIndirectMobAttack(shared_from_this(), mOwner, true);
        if (result->mEntity->hurt(damageSource, 4.0f))
        {
            doEnchantDamageEffects(mOwner, result->mEntity);
            if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, result->mEntity->GetType()))
            {
                boost::static_pointer_cast<LivingEntity>(result->mEntity)
                    ->addEffect(new MobEffectInstance(MobEffects::LEVITATION, 200), shared_from_this());
            }
        }
        delete damageSource;
    }
    remove();
}

void ShulkerBullet::tick()
{
    if (!mLevel->mIsClientSide && mLevel->getDifficulty() == Difficulty::PEACEFUL)
    {
        remove();
        return;
    }

    Entity::tick();

    if (!mLevel->mIsClientSide)
    {
        if (mTarget == NULL && mTargetUniqueId != L"")
        {
            std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(
                typeid(LivingEntity), AABB::newTemp(mTargetPos.offset(-2, -2, -2), mTargetPos.offset(2, 2, 2)));

            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
            {
                if ((*it)->getUUID() == mTargetUniqueId)
                {
                    mTarget = *it;
                    break;
                }
            }

            mTargetUniqueId = L"";
            delete entities;
        }

        if (mOwner == NULL && mOwnerUniqueId != L"")
        {
            std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(
                typeid(LivingEntity), AABB::newTemp(mOwnerPos.offset(-2, -2, -2), mOwnerPos.offset(2, 2, 2)));

            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
            {
                if ((*it)->getUUID() == mOwnerUniqueId)
                {
                    mOwner = boost::static_pointer_cast<LivingEntity>(*it);
                    break;
                }
            }

            mOwnerUniqueId = L"";
            delete entities;
        }

        bool tracking;
        if (mTarget != NULL && mTarget->isAlive()
            && !(eTYPE_DERIVED_FROM(EntityType::PLAYER, mTarget->GetType())
                 && boost::static_pointer_cast<Player>(mTarget)->isSpectator()))
            tracking = true;
        else
            tracking = false;

        if (tracking)
        {
            mPowerX = Mth::clamp(mPowerX * 1.025, -1.0, 1.0);
            mPowerY = Mth::clamp(mPowerY * 1.025, -1.0, 1.0);
            mPowerZ = Mth::clamp(mPowerZ * 1.025, -1.0, 1.0);
            mXd += (mPowerX - mXd) * 0.2;
            mYd += (mPowerY - mYd) * 0.2;
            mZd += (mPowerZ - mZd) * 0.2;
        }
        else if (!isNoGravity())
        {
            mYd -= 0.04;
        }

        std::vector<boost::shared_ptr<Entity> >* ignoredEntities = NULL;
        if (mOwner != NULL)
        {
            ignoredEntities = new std::vector<boost::shared_ptr<Entity> >();
            ignoredEntities->push_back(mOwner);
        }

        HitResult* hit = ProjectileUtil::forwardsRaycast(shared_from_this(), true, ignoredEntities, NULL);
        delete ignoredEntities;

        if (hit != NULL)
        {
            onHit(hit);
            delete hit;
        }
    }

    setPos(mX + mXd, mY + mYd, mZ + mZd);
    ProjectileUtil::rotateTowardsMovement(shared_from_this(), 0.5f);

    if (mLevel->mIsClientSide)
    {
        arrayWithLength<int> parameters;
        mLevel->addParticle(PARTICLE_END_ROD, mX - mXd, mY - mYd + 0.15, mZ - mZd, 0.0, 0.0, 0.0, parameters);
    }
    else if (mTarget != NULL && !mTarget->mRemoved)
    {
        if (mSteps > 0)
        {
            --mSteps;
            if (mSteps == 0)
            {
                selectNextMoveDirection(mMoveDirection == NULL ? NULL : mMoveDirection->getAxis());
            }
        }

        if (mMoveDirection != NULL)
        {
            BlockPos pos(shared_from_this());
            const Direction::Axis* axis = mMoveDirection->getAxis();

            if (mLevel->isSolidBlockingBlockInLoadedChunk(pos.relative(mMoveDirection), false))
            {
                selectNextMoveDirection(axis);
            }
            else
            {
                BlockPos targetPos(mTarget);
                if ((axis == Direction::Axis::X && pos.x == targetPos.x)
                    || (axis == Direction::Axis::Z && pos.z == targetPos.z)
                    || (axis == Direction::Axis::Y && pos.y == targetPos.y))
                {
                    selectNextMoveDirection(axis);
                }
            }
        }
    }
}

bool ShulkerBullet::hurt(const DamageSource* source, float damage)
{
    if (!mLevel->mIsClientSide)
    {
        playSound(SoundEvents::SHULKER_BULLET_HURT, 1.0f, 1.0f);
        static_cast<ServerLevel*>(mLevel)->sendParticles(PARTICLE_CRIT, mX, mY, mZ, 15, 0.2, 0.2, 0.2, 0.0,
                                                         arrayWithLength<int>());
        remove();
    }

    return true;
}

SoundSource::ESoundSource ShulkerBullet::getSoundSource()
{
    return SoundSource::eSoundSource_Hostile;
}

void ShulkerBullet::addAdditonalSaveData(CompoundTag* tag)
{
    if (mOwner != NULL)
    {
        BlockPos pos(mOwner);
        CompoundTag* owner = new CompoundTag();
        owner->putString(L"UUID", mOwner->getUUID());
        owner->putInt(L"X", pos.x);
        owner->putInt(L"Y", pos.y);
        owner->putInt(L"Z", pos.z);
        tag->put(L"Owner", owner);
    }
    if (mTarget != NULL)
    {
        BlockPos pos(mTarget);
        CompoundTag* target = new CompoundTag();
        target->putString(L"UUID", mTarget->getUUID());
        target->putInt(L"X", pos.x);
        target->putInt(L"Y", pos.y);
        target->putInt(L"Z", pos.z);
        tag->put(L"Target", target);
    }
    if (mMoveDirection != NULL)
        tag->putInt(L"Dir", mMoveDirection->get3DDataValue());
    tag->putInt(L"Steps", mSteps);
    tag->putDouble(L"TXD", mPowerX);
    tag->putDouble(L"TYD", mPowerY);
    tag->putDouble(L"TZD", mPowerZ);
}

void ShulkerBullet::readAdditionalSaveData(CompoundTag* tag)
{
    mSteps = tag->getInt(L"Steps");
    mPowerX = tag->getDouble(L"TXD");
    mPowerY = tag->getDouble(L"TYD");
    mPowerZ = tag->getDouble(L"TZD");
    if (tag->contains(L"Dir", Tag::TAG_ANY_NUMERIC))
        mMoveDirection = Direction::from3DDataValue(tag->getInt(L"Dir"));
    if (tag->contains(L"Owner", Tag::TAG_COMPOUND))
    {
        CompoundTag* owner = tag->getCompound(L"Owner");
        mOwnerUniqueId = owner->getString(L"UUID");
        mOwnerPos = BlockPos(owner->getInt(L"X"), owner->getInt(L"Y"), owner->getInt(L"Z"));
    }
    if (tag->contains(L"Target", Tag::TAG_COMPOUND))
    {
        CompoundTag* target = tag->getCompound(L"Target");
        mTargetUniqueId = target->getString(L"UUID");
        mTargetPos = BlockPos(target->getInt(L"X"), target->getInt(L"Y"), target->getInt(L"Z"));
    }
}

bool ShulkerBullet::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 16384.0;
}

float ShulkerBullet::getBrightness()
{
    return 1.0f;
}

int ShulkerBullet::getLightColor()
{
    return 0xf000f0;
}

bool ShulkerBullet::isPickable()
{
    return true;
}


bool ShulkerBullet::isOnFire()
{
    return false;
}

void ShulkerBullet::defineSynchedData()
{
}

unsigned long long ShulkerBullet::GetType()
{
    return EntityType::SHULKER_BULLET;
}
