#include "game/entity/mob/TamableAnimal.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/CombatTracker.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/SitGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"

const EntityDataAccessor<unsigned char>* TamableAnimal::DATA_FLAGS = NULL;
const EntityDataAccessor<std::wstring>* TamableAnimal::DATA_OWNER_UUID = NULL;

TamableAnimal::TamableAnimal(Level* level) : Animal(level), mSitGoal(NULL)
{
}

TamableAnimal::~TamableAnimal()
{
    delete mSitGoal;
}

std::wstring TamableAnimal::getOwnerUUID()
{
    return mEntityData->get(DATA_OWNER_UUID);
}

boost::shared_ptr<Entity> TamableAnimal::getOwner()
{
    std::wstring uuid = getOwnerUUID();
    if (uuid.empty())
        return boost::shared_ptr<Entity>();
    return mLevel->getPlayerByUUID(uuid);
}

void TamableAnimal::staticCtor()
{
    DATA_FLAGS = SynchedEntityData::defineId(EntityType::TAMABLE_ANIMAL, EntityType::ANIMAL,
                                             EntityDataSerializers::BYTE);
    DATA_OWNER_UUID = SynchedEntityData::defineId(EntityType::TAMABLE_ANIMAL, EntityType::ANIMAL,
                                                  EntityDataSerializers::STRING);
}

void TamableAnimal::fjDerivedCtorCalls()
{
    Mob::fjDerivedCtorCalls();
    reassessTameGoals();
}

void TamableAnimal::defineSynchedData()
{
    Animal::defineSynchedData();
    mEntityData->createDateItem(DATA_FLAGS, static_cast<unsigned char>(0));
    mEntityData->define(DATA_OWNER_UUID, std::wstring(L""));
}

void TamableAnimal::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    if (getOwnerUUID().empty())
        tag->putString(L"OwnerUUID", L"");
    else
        tag->putString(L"OwnerUUID", getOwnerUUID());

    tag->putBoolean(L"Sitting", isSitting());
}

void TamableAnimal::readAdditionalSaveData(CompoundTag* tag)
{
    Animal::readAdditionalSaveData(tag);
    std::wstring ownerUuid = L"";
    if (tag->contains(L"OwnerUUID"))
        ownerUuid = tag->getString(L"OwnerUUID");
    else
        ownerUuid = tag->getString(L"Owner");
    if (!ownerUuid.empty())
    {
        setOwnerUUID(ownerUuid);
        setTame(true);
    }

    if (mSitGoal != NULL)
    {
        mSitGoal->wantToSit(tag->getBoolean(L"Sitting"));
    }
    setSitting(tag->getBoolean(L"Sitting"));
}

bool TamableAnimal::canBeLeashed(const boost::shared_ptr<Player>& player)
{
    return isTame() && isOwnedBy(player);
}

void TamableAnimal::spawnTamingParticles(bool success)
{
    const ParticleType* particle = PARTICLE_HEART;
    if (!success)
        particle = PARTICLE_SMOKE;

    for (int i = 0; i < 7; ++i)
    {
        double xa = mRandom->nextGaussian() * 0.02;
        double ya = mRandom->nextGaussian() * 0.02;
        double za = mRandom->nextGaussian() * 0.02;
        mLevel->addParticle(particle, mX + mRandom->nextFloat() * mBbWidth * 2.0f - mBbWidth,
                            mY + 0.5 + mRandom->nextFloat() * mBbHeight,
                            mZ + mRandom->nextFloat() * mBbWidth * 2.0f - mBbWidth, xa, ya, za, arrayWithLength<int>());
    }
}

void TamableAnimal::handleEntityEvent(unsigned char event, int data)
{
    switch (event)
    {
    case EntityEvent::TAMING_SUCCEEDED:
        spawnTamingParticles(true);
        break;
    case EntityEvent::TAMING_FAILED:
        spawnTamingParticles(false);
        break;
    default:
        Animal::handleEntityEvent(event, data);
        break;
    }
}

bool TamableAnimal::isTame()
{
    return (mEntityData->get(DATA_FLAGS) & 4) != 0;
}

void TamableAnimal::setTame(bool tame)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    if (tame)
        mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags | 4), true);
    else
        mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags & 251), true);
    reassessTameGoals();
}

void TamableAnimal::reassessTameGoals()
{
}

bool TamableAnimal::removeWhenFarAway()
{
    return !isTame() && Animal::removeWhenFarAway();
}

bool TamableAnimal::isSitting()
{
    return (mEntityData->get(DATA_FLAGS) & 1) != 0;
}

void TamableAnimal::setSitting(bool sitting)
{
    unsigned char flags = mEntityData->get(DATA_FLAGS);
    if (sitting)
        mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags | 1), true);
    else
        mEntityData->set(DATA_FLAGS, static_cast<unsigned char>(flags & 254), true);
}

void TamableAnimal::setOwnerUUID(const std::wstring& uuid)
{
    mEntityData->set(DATA_OWNER_UUID, uuid, true);
}

void TamableAnimal::tame(boost::shared_ptr<Player> player)
{
    setTame(true);
    setOwnerUUID(player->getUUID());
}

bool TamableAnimal::isOwnedBy(boost::shared_ptr<LivingEntity> entity)
{
    return entity == getOwner();
}

SitGoal* TamableAnimal::getSitGoal()
{
    return mSitGoal;
}

bool TamableAnimal::wantsToAttack(boost::shared_ptr<LivingEntity> target, boost::shared_ptr<LivingEntity> owner)
{
    return true;
}

Team* TamableAnimal::getTeam()
{
    if (isTame())
    {
        boost::shared_ptr<Entity> owner = getOwner();
        if (owner != NULL)
            return owner->getTeam();
    }
    return LivingEntity::getTeam();
}

bool TamableAnimal::isAlliedTo(boost::shared_ptr<Entity> entity)
{
    if (isTame())
    {
        boost::shared_ptr<Entity> owner = getOwner();
        if (entity == owner)
        {
            return true;
        }
        if (owner != NULL)
        {
            return owner->isAlliedTo(entity);
        }
    }

    return Entity::isAlliedTo(entity);
}

void TamableAnimal::die(const DamageSource* source)
{
    if (!mLevel->mIsClientSide && mLevel->getGameRules()->getBoolean(GameRules::RULE_SHOWDEATHMESSAGES)
        && hasCustomName())
    {
        if (getOwner() != NULL && eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, getOwner()->GetType()))
        {
            boost::dynamic_pointer_cast<ServerPlayer>(getOwner())
                ->mConnection->send(getCombatTracker()->getDeathMessagePacket());
        }
    }

    LivingEntity::die(source);
}

InteractionResult::EInteractionResult TamableAnimal::interactAt(const boost::shared_ptr<Player>& player, Vec3* pos,
                                                                InteractionHand::EInteractionHand hand)
{
    return isTame() ? InteractionResult::eInteractionResult_Success : InteractionResult::eInteractionResult_Pass;
}
