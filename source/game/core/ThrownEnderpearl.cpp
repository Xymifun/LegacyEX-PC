#include "game/core/ThrownEnderpearl.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Endermite.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/TheEndGatewayEntity.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/WorldBorder.hpp"

ThrownEnderpearl::ThrownEnderpearl(Level* level) : ThrowableProjectile(level), mThrower()
{
    _init();
}

ThrownEnderpearl::ThrownEnderpearl(Level* level, boost::shared_ptr<LivingEntity> owner)
    : ThrowableProjectile(level, owner),
      mThrower()
{
    _init();
    mThrower = owner;
}

ThrownEnderpearl::ThrownEnderpearl(Level* level, double x, double y, double z)
    : ThrowableProjectile(level, x, y, z),
      mThrower()
{
    _init();
}

Entity* ThrownEnderpearl::create(Level* level)
{
    return new ThrownEnderpearl(level);
}

void ThrownEnderpearl::PlayTeleportSound()
{
    mLevel->playSound(boost::shared_ptr<Player>(), mXo, mYo, mZo, SoundEvents::ENDERMEN_TELEPORT, getSoundSource(),
                      1.0f, 1.0f, 16.0f);
    playSound(SoundEvents::ENDERMEN_TELEPORT, 1.0f, 1.0f);
}

bool ThrownEnderpearl::onHit(const HitResult* result)
{
    Projectile::onHit(result);
    boost::shared_ptr<LivingEntity> owner = getOwner();

    if (result->mEntity)
    {
        if (result->mEntity == mThrower)
            return false;

        DamageSource* damage = DamageSource::CreateThrown(shared_from_this(), getOwner());
        result->mEntity->hurt(damage, 0.0f);
        delete damage;
    }

    if (result->mType == HitResult::BLOCK)
    {
        BlockPos position = result->getBlockPos();
        boost::shared_ptr<BlockEntity> blockEntity = mLevel->getBlockEntity(position);
        if (blockEntity && eTYPE_DERIVED_FROM(BlockEntityType::THE_END_GATEWAY_ENTITY, blockEntity->GetType()))
        {
            boost::shared_ptr<TheEndGatewayEntity> gateway = boost::static_pointer_cast<TheEndGatewayEntity>(
                blockEntity);
            if (owner)
            {
                gateway->teleportEntity(owner);
                remove();
                return true;
            }
            gateway->teleportEntity(shared_from_this());
            return true;
        }
    }

    for (int i = 0; i < 32; ++i)
    {
        mLevel->addParticle(PARTICLE_ENDER, mX, mY + mRandom->nextDouble() * 2.0, mZ, mRandom->nextGaussian(), 0.0,
                            mRandom->nextGaussian(), arrayWithLength<int>());
    }

    if (!mLevel->mIsClientSide)
    {
        AABB* constraints = mLevel->mDimension->getPlayerConstraints(BlockPos(this))->shrink(1.0);
        if (!constraints->contains(BlockPos(this)))
        {
            remove();
            return true;
        }

        if (owner && owner->instanceof(EntityType::SERVER_PLAYER))
        {
            boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(getOwner());
            if (!player->mRemoved && !player->mConnection->isDisconnected() && player->mLevel == mLevel
                && !player->isSleeping())
            {
                double distance = player->distanceTo(mX, mY, mZ);
                if (distance > 100.0)
                {
                    player->awardStat(GenericStats::beamMeUp(), GenericStats::param_beamMeUp((int)distance));
                }

                if (mRandom->nextFloat() < 0.05f && mLevel->getGameRules()->getBoolean(GameRules::RULE_DOMOBSPAWNING))
                {
                    boost::shared_ptr<Endermite> endermite(new Endermite(mLevel));
                    endermite->setPlayerSpawned(true);
                    endermite->moveTo(owner->mX, owner->mY, owner->mZ, owner->mYRot, owner->mXRot);
                    mLevel->addEntity(endermite);
                }

                if (owner->isPassenger())
                    owner->stopRidingBeforeTeleport();
                owner->teleportTo(mX, mY, mZ);
                PlayTeleportSound();
                owner->mFallDistance = 0.0f;
                owner->hurt(DamageSource::FALL, 5.0f);
            }
        }
        else if (owner)
        {
            owner->teleportTo(mX, mY, mZ);
            PlayTeleportSound();
            owner->mFallDistance = 0.0f;
        }
        remove();
    }
    return true;
}

void ThrownEnderpearl::addDataWalkers(DataFixerUpper*)
{
}

void ThrownEnderpearl::_init()
{
    fjDerivedCtorCalls();
    mThrower = boost::shared_ptr<LivingEntity>();
}

void ThrownEnderpearl::tick()
{
    PIXBeginNamedEvent(0.0f, "Thrown enderpearl tick");
    boost::shared_ptr<LivingEntity> owner = getOwner();
    if (owner && eTYPE_DERIVED_FROM(EntityType::PLAYER, owner->GetType()) && !owner->isAlive())
    {
        remove();
    }
    else
    {
        ThrowableProjectile::tick();
    }
    PIXEndNamedEvent();
}

unsigned long long ThrownEnderpearl::GetType()
{
    return EntityType::THROWN_ENDERPEARL;
}
