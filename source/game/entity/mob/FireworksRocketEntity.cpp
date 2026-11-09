#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/GameMode.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <cstdlib>
#include <string>
#include <typeinfo>
#include <vector>

#include "game/client/render/ParticleType.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/FireworksItem.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

const EntityDataAccessor<not_null_ptr<ItemInstance> >* FireworksRocketEntity::DATA_ITEM = NULL;
const EntityDataAccessor<int>* FireworksRocketEntity::DATA_ATTACHED_TO_ENTITY = NULL;

FireworksRocketEntity::FireworksRocketEntity(Level* level, const boost::shared_ptr<Player>& player,
                                             const not_null_ptr<ItemInstance>& item)
    : Entity(level, true),
      mAttachedToEntity(),
      mOwner()
{
    _init(player->mX, player->mY + player->getEyeHeight(), player->mZ, item);
    mHasDirectionalData = true;
    Vec3* look = player->getLookAngle();
    mXd = look->x;
    mYd = look->y;
    mZd = look->z;
    mOwner = player;
}

FireworksRocketEntity::FireworksRocketEntity(Level* level, double x, double y, double z,
                                             not_null_ptr<ItemInstance> item)
    : Entity(level, true),
      mAttachedToEntity(),
      mOwner()
{
    _init(x, y, z, item);
    mOwner = boost::shared_ptr<Player>();
}

FireworksRocketEntity::FireworksRocketEntity(Level* level, const not_null_ptr<ItemInstance>& item,
                                             const boost::shared_ptr<LivingEntity>& entity)
    : Entity(level, true),
      mAttachedToEntity(),
      mOwner()
{
    _init(entity->mX, entity->mY, entity->mZ, item);
    mEntityData->set(DATA_ATTACHED_TO_ENTITY, entity->getId(), true);
    mAttachedToEntity = entity;
    mOwner = boost::shared_ptr<Player>();
}

FireworksRocketEntity::FireworksRocketEntity(Level* level)
    : Entity(level, true),
      mLife(0),
      mLifetime(0),
      mAttachedToEntity(),
      mHasDirectionalData(true),
      mOwner()
{
    fjDerivedCtorCalls();
    setSize(0.25f, 0.25f);
}

FireworksRocketEntity::FireworksRocketEntity(Level* level, double x, double y, double z, double xd, double yd,
                                             double zd, not_null_ptr<ItemInstance> item, bool hasDirectionalData,
                                             const boost::shared_ptr<Player>& owner)
    : Entity(level, true),
      mAttachedToEntity(),
      mOwner()
{
    _init(x, y, z, item);
    mDirectionalX = static_cast<int>(Math::round(xd));
    mDirectionalZ = static_cast<int>(Math::round(zd));
    mDirectionalY = static_cast<int>(Math::round(yd));
    mXd = xd;
    mZd = zd;
    mYd = yd;
    mDirectionalInitialized = true;
    mHasDirectionalData = hasDirectionalData;
    mOwner = owner;
}

void FireworksRocketEntity::_init(double x, double y, double z, const not_null_ptr<ItemInstance>& item)
{
    defineSynchedData();
    fjDerivedCtorCalls();
    mLife = 0;
    setSize(0.25f, 0.25f);
    setPos(x, y, z);
    mHasDirectionalData = true;

    int flightDuration = 1;
    if (!item->isEmpty() && item->hasTag())
    {
        bool updateDirty = !Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks();
        mEntityData->set(DATA_ITEM, item->copy(), updateDirty);
        CompoundTag* fireworksTag = item->getTag()->getCompound(FireworksItem::TAG_FIREWORKS);
        if (fireworksTag != NULL)
        {
            flightDuration += fireworksTag->getByte(FireworksItem::TAG_FLIGHT);
        }
    }

    mXd = mRandom->nextGaussian() * 0.001;
    mZd = mRandom->nextGaussian() * 0.001;
    mYd = 0.05;
    mDirectionalX = 0;
    mDirectionalZ = 0;
    mDirectionalY = 0;

    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        mLifetime = 40;
    }
    else
    {
        mLifetime = flightDuration * 10 + mRandom->nextInt(6) + mRandom->nextInt(7);
    }

    mOwner = boost::shared_ptr<Player>();
}

void FireworksRocketEntity::staticCtor()
{
    DATA_ATTACHED_TO_ENTITY = SynchedEntityData::defineId<int>(EntityType::FIREWORKS_ROCKET_ENTITY, EntityType::ENTITY,
                                                               EntityDataSerializers::INT);
    DATA_ITEM = SynchedEntityData::defineId<not_null_ptr<ItemInstance> >(
        EntityType::FIREWORKS_ROCKET_ENTITY, EntityType::ENTITY, EntityDataSerializers::ITEM_INSTANCE);
}

bool FireworksRocketEntity::isAttachedToEntity()
{
    return mEntityData->get(DATA_ATTACHED_TO_ENTITY) >= 0;
}

bool FireworksRocketEntity::shouldRenderAtSqrDistance(double distanceSqr)
{
    return distanceSqr < 4096.0 && !isAttachedToEntity();
}

bool FireworksRocketEntity::shouldRender(double x, double y, double z)
{
    return Entity::shouldRender(x, y, z) && !isAttachedToEntity();
}

void FireworksRocketEntity::lerpMotion(double xd, double yd, double zd)
{
    mXd = xd;
    mYd = yd;
    mZd = zd;
    if (mXRotO == 0.0f && mYRotO == 0.0f)
    {
        double horizontal = Mth::sqrt(xd * xd + zd * zd);
        mYRot = static_cast<float>(std::atan2(xd, zd) * Mth::RAD_TO_DEG);
        mYRotO = mYRot;
        mXRot = static_cast<float>(std::atan2(yd, horizontal) * Mth::RAD_TO_DEG);
        mXRotO = mXRot;
    }
}

double FireworksRocketEntity::HandleDirectionalAcceleration(int direction, double velocity)
{
    if (direction == 1)
    {
        if (mDirectionalInitialized)
        {
            velocity -= 0.95;
            mDirectionalInitialized = false;
        }
        return velocity + 0.04;
    }
    if (direction == -1)
    {
        if (mDirectionalInitialized)
        {
            velocity += 0.95;
            mDirectionalInitialized = false;
        }
        return velocity - 0.04;
    }
    return velocity;
}

void FireworksRocketEntity::dealExplosionDamage()
{
    float power = 0.0f;
    ListTag* explosions = NULL;
    not_null_ptr<ItemInstance> item = mEntityData->get(DATA_ITEM);
    if (!item->isEmpty())
    {
        CompoundTag* fireworksTag = static_cast<CompoundTag*>(item->getTagElement(FireworksItem::TAG_FIREWORKS));
        if (fireworksTag != NULL)
        {
            explosions = fireworksTag->getList(FireworksItem::TAG_EXPLOSIONS, Tag::TAG_COMPOUND);
            if (explosions != NULL && !explosions->isEmpty())
            {
                power = static_cast<float>(explosions->size()) * 2.0f + 5.0f;
            }
        }
    }
    if (power > 0.0f)
    {
        boost::shared_ptr<LivingEntity> attached = mAttachedToEntity.lock();
        if (attached != NULL)
        {
            attached->hurt(DamageSource::FIREWORKS, static_cast<float>(explosions->size()) * 2.0f + 5.0f);
        }

        double radius = 5.0;
        Vec3* center = Vec3::newTemp(mX, mY, mZ);
        std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(LivingEntity),
                                                                                       mBoundingBox->grow(radius));
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(*it);
            if (living == attached || distanceToSqr(living) > radius * radius)
            {
                continue;
            }
            bool visible = false;
            for (int i = 0; i < 2; ++i)
            {
                Vec3* target = Vec3::newTemp(living->mX, living->mY + living->mBbHeight * 0.5 * i, living->mZ);
                HitResult* hit = mLevel->clip(center, target, false, true, false, false, NULL, false);
                if (hit == NULL || hit->mType == HitResult::MISS)
                {
                    visible = true;
                    break;
                }
            }
            if (visible)
            {
                float damage = power * static_cast<float>(std::sqrt((radius - distanceTo(living)) / radius));
                living->hurt(DamageSource::FIREWORKS, damage);
            }
        }
        delete entities;
    }
}

void FireworksRocketEntity::tick()
{
    PIXBeginNamedEvent(0.0f, "Fireworks rocket entity tick");

    mXOld = mX;
    mYOld = mY;
    mZOld = mZ;
    Entity::tick();

    if (isAttachedToEntity())
    {
        boost::shared_ptr<LivingEntity> attached = mAttachedToEntity.lock();

        if (attached == NULL)
        {
            boost::shared_ptr<Entity> entity = mLevel->getEntity(mEntityData->get(DATA_ATTACHED_TO_ENTITY));

            if (entity != NULL && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
            {
                attached = boost::static_pointer_cast<LivingEntity>(entity);
            }
        }

        if (attached != NULL)
        {
            if (attached->isFallFlying())
            {
                Vec3* look = attached->getLookAngle();
                attached->mXd += look->x * 0.1 + (look->x * 1.5 - attached->mXd) * 0.5;
                attached->mYd += look->y * 0.1 + (look->y * 1.5 - attached->mYd) * 0.5;
                attached->mZd += look->z * 0.1 + (look->z * 1.5 - attached->mZd) * 0.5;
            }

            setPos(attached->mX, attached->mY, attached->mZ);
            mXd = attached->mXd;
            mYd = attached->mYd;
            mZd = attached->mZd;
        }
    }
    else
    {
        if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
        {
            if (mDirectionalX == 0 && mDirectionalY == 0 && mDirectionalZ == 0)
            {
                mYd += 0.04;
            }
            else
            {
                mXd = HandleDirectionalAcceleration(mDirectionalX, mXd);
                mYd = HandleDirectionalAcceleration(mDirectionalY, mYd);
                mZd = HandleDirectionalAcceleration(mDirectionalZ, mZd);
            }
        }

        move(eMoverType_Self, mXd, mYd, mZd, false);
    }

    double facingX = mXd;
    double facingY = mYd;
    double facingZ = mZd;

    if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        if (mDirectionalX == 0 && mDirectionalY == 0 && mDirectionalZ == 0)
        {
            facingX = 0.0;
            facingY = 1.0;
            facingZ = 0.0;
        }
        else
        {
            facingX = mDirectionalX;
            facingY = mDirectionalY;
            facingZ = mDirectionalZ;
        }
    }

    double horizontal = Mth::sqrt(facingX * facingX + facingZ * facingZ);
    mYRot = (float)(atan2(facingX, facingZ) * Mth::RAD_TO_DEG);
    mXRot = (float)(atan2(facingY, horizontal) * Mth::RAD_TO_DEG);

    while (mXRot - mXRotO < -180.0f)
    {
        mXRotO -= 360.0f;
    }

    while (mXRot - mXRotO >= 180.0f)
    {
        mXRotO += 360.0f;
    }

    while (mYRot - mYRotO < -180.0f)
    {
        mYRotO -= 360.0f;
    }

    while (mYRot - mYRotO >= 180.0f)
    {
        mYRotO += 360.0f;
    }

    mXRot = mXRotO + (mXRot - mXRotO) * 0.2f;
    mYRot = mYRotO + (mYRot - mYRotO) * 0.2f;

    if (mLife == 0 && mLevel->mIsClientSide && !isSilent())
    {
        mLevel->playLocalSound(mX, mY, mZ, SoundEvents::FIREWORK_LAUNCH, SoundSource::eSoundSource_Ambient, 3.0f, 1.0f,
                               false, 16.0f);
    }

    ++mLife;

    int particleInterval = 2;
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        particleInterval = 4;
    }

    if (mLevel->mIsClientSide)
    {
        if (mLife % particleInterval == 0)
        {
            double py;
            double vx;
            double vy;
            double vz;

            if (mDirectionalX != 0 || mDirectionalY != 0 || mDirectionalZ != 0
                || Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
            {
                py = mY;
                double drag = Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks() ? 0.1 : 0.5;
                vy = mRandom->nextGaussian() * 0.05 - mYd * drag;
                vz = mRandom->nextGaussian() * 0.05 - mZd * drag;
                vx = mRandom->nextGaussian() * 0.05 - mXd * drag;
            }
            else
            {
                py = mY - 0.3;
                vy = -(mYd * 0.5);
                vz = mRandom->nextGaussian() * 0.05;
                vx = mRandom->nextGaussian() * 0.05;
            }

            if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
            {
                arrayWithLength<int> noColour;
                mLevel->addParticle(PARTICLE_FIREWORKS_SPARK, mX, py, mZ, vx, vy, vz, noColour);
            }
            else
            {
                int playerIndex = mOwner != NULL ? mOwner->mPlayerIndex : 0;
                unsigned int nametagColour = PlayerRenderer::getNametagColour(playerIndex);
                if (playerIndex == 0)
                {
                    nametagColour = 0xffffffff;
                }

                arrayWithLength<int> colour(4, true);
                colour.data[0] = (nametagColour >> 16) & 0xff;
                colour.data[1] = (nametagColour >> 8) & 0xff;
                colour.data[2] = nametagColour & 0xff;
                colour.data[3] = 0xff;
                mLevel->addParticle(PARTICLE_FIREWORKS_SPARK, mX, py, mZ, vx, vy, vz, colour);
            }
        }
    }
    else
    {
        if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks() && mCollision)
        {
            if (mOwner != NULL)
            {
                GameMode* gameMode = boost::static_pointer_cast<Player>(mOwner)->GetGameMode();
                BlockPos pos(mX, mY, mZ);
                bool mayDestroy = gameMode->MayDestroy(pos);
                mLevel->explode(shared_from_this(), mX, mY, mZ, 2.0f, false, mayDestroy, false, false, false, -1.0f);
            }

            remove();
        }
    }

    if (!mLevel->mIsClientSide && mLife > mLifetime)
    {
        dealExplosionDamage();
        remove();
    }

    if (mRemoved && !mLevel->mIsClientSide)
    {
        mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::FIREWORKS_EXPLODE, 0);
    }

    PIXEndNamedEvent();
}

void FireworksRocketEntity::setExplosionColourToPipColour(CompoundTag* tag)
{
    ListTag* explosions = tag->getList(FireworksItem::TAG_EXPLOSIONS, Tag::TAG_COMPOUND);
    if (explosions == NULL)
    {
        return;
    }
    CompoundTag* explosion = explosions->getCompound(0);
    if (explosion == NULL)
    {
        return;
    }
    int playerIndex = mOwner != NULL ? mOwner->mPlayerIndex : 0;
    int colour = PlayerRenderer::getNametagColour(playerIndex);
    if (playerIndex == 0)
    {
        colour = -1;
    }
    arrayWithLength<int> colours;
    colours = arrayWithLength<int>(1, true);
    colours.data[0] = colour;
    explosion->putIntArray(FireworksItem::TAG_E_COLORS, colours);
}

void FireworksRocketEntity::handleEntityEvent(unsigned char event, int data)
{
    if (event != EntityEvent::FIREWORKS_EXPLODE)
    {
        Entity::handleEntityEvent(event, data);
        return;
    }

    not_null_ptr<ItemInstance> item = mEntityData->get(DATA_ITEM);
    CompoundTag* fireworksTag = NULL;

    if (!item->isEmpty())
    {
        fireworksTag = static_cast<CompoundTag*>(item->getTagElement(FireworksItem::TAG_FIREWORKS));
        if (fireworksTag != NULL && Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
        {
            setExplosionColourToPipColour(fireworksTag);
        }
    }

    mLevel->createFireworks(mX, mY, mZ, mXd, mYd, mZd, fireworksTag);
}

void FireworksRocketEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity,
                     new ItemDataWalker(EntityType::FIREWORKS_ROCKET_ENTITY, L"FireworksItem"));
}

void FireworksRocketEntity::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"Life", mLife);
    tag->putInt(L"LifeTime", mLifetime);
    not_null_ptr<ItemInstance> item = mEntityData->get(DATA_ITEM);
    if (!item->isEmpty())
    {
        tag->put(L"FireworksItem", item->save(new CompoundTag()));
    }
}

void FireworksRocketEntity::readAdditionalSaveData(CompoundTag* tag)
{
    mLife = tag->getInt(L"Life");
    mLifetime = tag->getInt(L"LifeTime");
    CompoundTag* itemTag = tag->getCompound(L"FireworksItem");
    if (itemTag != NULL)
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(itemTag));
        if (!item->isEmpty())
        {
            mEntityData->set(DATA_ITEM, item, true);
        }
    }
}

bool FireworksRocketEntity::isAttackable()
{
    return false;
}

boost::shared_ptr<LivingEntity> FireworksRocketEntity::getOwner() const
{
    return mOwner;
}

void FireworksRocketEntity::updateDeltaAfterMove(Block* block, double x, double y, double z, double& movedX,
                                                 double& movedY, double& movedZ)
{
    bool directional = Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks();
    if (!directional || !mLevel->mIsClientSide)
    {
        Entity::updateDeltaAfterMove(block, x, y, z, movedX, movedY, movedZ);
    }
}

void FireworksRocketEntity::defineSynchedData()
{
    mEntityData->define(DATA_ITEM, ItemInstance::EMPTY);
    mEntityData->define(DATA_ATTACHED_TO_ENTITY, -1);
}

Entity* FireworksRocketEntity::create(Level* level)
{
    return new FireworksRocketEntity(level);
}

unsigned long long FireworksRocketEntity::GetType()
{
    return EntityType::FIREWORKS_ROCKET_ENTITY;
}

bool FireworksRocketEntity::shouldTick()
{
    return true;
}
