#include <algorithm>
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Wolf.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

namespace
{
}


const EntityDataAccessor<int>* Animal::DATA_IN_LOVE = NULL;
const EntityDataAccessor<int>* Animal::DATA_IN_LOVE_TIME = NULL;

Animal::Animal(Level* level) : AgableMob(level)
{
    mSpawnableBlock = Blocks::grass;
    mLoveCause = L"";
    mBreedCooldown = 1200;
    setDespawnProtected();
    mCategories |= EntityCategory::eEntityCategory_Animal;
}

void Animal::staticCtor()
{
    DATA_IN_LOVE = SynchedEntityData::defineId(EntityType::ANIMAL, EntityType::AGABLE_MOB, EntityDataSerializers::INT);
    DATA_IN_LOVE_TIME = SynchedEntityData::defineId(EntityType::ANIMAL, EntityType::AGABLE_MOB,
                                                    EntityDataSerializers::INT);
}

void Animal::setDespawnProtected()
{
    if (mLevel != NULL && mLevel->mIsClientSide)
    {
        return;
    }

    short x = Mth::floor(mX);
    short z = Mth::floor(mZ);

    mMinX = x;
    mMaxX = x;
    mMinZ = z;
    mMaxZ = z;
    mDespawnProtected = true;
}

void Animal::defineSynchedData()
{
    AgableMob::defineSynchedData();
    mEntityData->createDateItem(DATA_IN_LOVE, 0);
    mEntityData->createDateItem(DATA_IN_LOVE_TIME, 0);
}

void Animal::setInLoveValue(int value)
{
    if (!mLevel->mIsClientSide)
    {
        mEntityData->set(DATA_IN_LOVE, value, true);
    }
}

void Animal::customServerAiStep()
{
    if (isBaby())
    {
        setInLoveValue(0);
    }

    Mob::customServerAiStep();
}

int Animal::getInLoveValue()
{
    return mEntityData->get(DATA_IN_LOVE);
}

void Animal::updateDespawnProtectedState()
{
    if (mLevel->mIsClientSide)
    {
        return;
    }

    if (!mDespawnProtected)
    {
        return;
    }

    short x = Mth::floor(mX);
    short z = Mth::floor(mZ);

    mMaxX = std::max(mMaxX, x);
    mMinX = std::min(mMinX, x);
    mMaxZ = std::max(mMaxZ, z);
    mMinZ = std::min(z, mMinZ);

    if (mMaxX - mMinX > 20 || mMaxZ - mMinZ > 20)
    {
        mDespawnProtected = false;
    }
}

void Animal::aiStep()
{
    AgableMob::aiStep();

    if (isBaby())
    {
        setInLoveValue(0);
    }

    if (getInLoveValue() > 0)
    {
        setInLoveValue(getInLoveValue() - 1);
        if (getInLoveValue() % 10 == 0)
        {
            double xa = mRandom->nextGaussian() * 0.02;
            double ya = mRandom->nextGaussian() * 0.02;
            double za = mRandom->nextGaussian() * 0.02;
            mLevel->addParticle(PARTICLE_HEART, mX + mRandom->nextFloat() * mBbWidth * 2.0f - mBbWidth,
                                mY + 0.5 + mRandom->nextFloat() * mBbHeight,
                                mZ + mRandom->nextFloat() * mBbWidth * 2.0f - mBbWidth, xa, ya, za,
                                arrayWithLength<int>());
        }
    }

    if (mEntityData->get(DATA_IN_LOVE_TIME) > 0)
    {
        mEntityData->set(DATA_IN_LOVE_TIME, mEntityData->get(DATA_IN_LOVE_TIME) - 1, true);
        setInLoveValue(0);
    }

    updateDespawnProtectedState();
}

bool Animal::hurt(const DamageSource* source, float damage)
{
    if (isInvulnerableTo(source))
    {
        return false;
    }

    boost::shared_ptr<Entity> attacker = source->getEntity();

    if (attacker != NULL && eTYPE_DERIVED_FROM(EntityType::PLAYER, attacker->GetType())
        && !boost::static_pointer_cast<Player>(attacker)->isAllowedToAttackAnimals())
    {
        return false;
    }

    setInLoveValue(0);
    return LivingEntity::hurt(source, damage);
}

float Animal::getWalkTargetValue(const BlockPos& pos)
{
    if (mLevel->getBlockState(pos.below())->getBlock() == mSpawnableBlock)
    {
        return 10.0f;
    }

    return mLevel->getBrightness(pos) - 0.5f;
}

void Animal::addAdditonalSaveData(CompoundTag* tag)
{
    AgableMob::addAdditonalSaveData(tag);
    tag->putInt(L"InLove", getInLoveValue());

    if (!mLoveCause.empty())
    {
        tag->putString(L"LoveCause", mLoveCause);
    }
}

double Animal::getRidingHeight()
{
    return 0.14;
}

void Animal::readAdditionalSaveData(CompoundTag* tag)
{
    AgableMob::readAdditionalSaveData(tag);
    setInLoveValue(tag->getInt(L"InLove"));

    mLoveCause = tag->contains(L"LoveCause") ? tag->getString(L"LoveCause") : std::wstring(L"");

    setDespawnProtected();
}

bool Animal::checkSpawnRules()
{
    int x = Mth::floor(mX);
    int y = Mth::floor(mBoundingBox->mMinY);
    int z = Mth::floor(mZ);
    BlockPos pos(x, y, z);
    return mLevel->getBlockState(pos.below())->getBlock() == mSpawnableBlock && mLevel->getDaytimeRawBrightness(pos) > 8
           && PathfinderMob::checkSpawnRules();
}

int Animal::getAmbientSoundInterval()
{
    return 120;
}

bool Animal::removeWhenFarAway()
{
    return !isDespawnProtected() && !isLeashed() && !hasCustomName();
}

int Animal::getExperienceReward(boost::shared_ptr<Player> player)
{
    return mLevel->mRandom->nextInt(3) + 1;
}

bool Animal::isFood(const not_null_ptr<ItemInstance>& itemInstance)
{
    return itemInstance->getItem() == Items::wheat;
}

bool Animal::isBreedCoolingDown()
{
    return mEntityData->get(DATA_IN_LOVE_TIME) > 0;
}

void Animal::usePlayerItem(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance)
{
    if (!player->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
    }
}

bool Animal::AgeUpBaby(const boost::shared_ptr<Player>& player, not_null_ptr<ItemInstance> itemInstance)
{
    if (isBaby() && isFood(itemInstance))
    {
        usePlayerItem(player, itemInstance);
        ageUp(6 * 20, true);
        return true;
    }

    return false;
}

bool Animal::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!itemInstance->isEmpty())
    {
        if (isFood(itemInstance) && !isBaby() && getInLoveValue() <= 0 && !isBreedCoolingDown())
        {
            if (!mLevel->mIsClientSide)
            {
                switch (GetType())
                {
                case EntityType::CHICKEN:
                    if (!mLevel->canCreateMore(EntityType::CHICKEN, Level::eSpawnType_Breed))
                    {
                        player->displayClientMessage(MESSAGE_CHICKEN_LIMIT, true);
                        return false;
                    }
                    break;
                case EntityType::WOLF:
                    if (!mLevel->canCreateMore(EntityType::WOLF, Level::eSpawnType_Breed))
                    {
                        player->displayClientMessage(MESSAGE_WOLF_LIMIT, true);
                        return false;
                    }
                    break;
                case EntityType::MUSHROOM_COW:
                    if (!mLevel->canCreateMore(EntityType::MUSHROOM_COW, Level::eSpawnType_Breed))
                    {
                        player->displayClientMessage(MESSAGE_ANIMAL_LIMIT, true);
                        return false;
                    }
                    break;
                default:
                {
                    const bool isCreature = (GetType() & EntityType::CREATURE_CATEGORY) != 0;
                    const unsigned long long type = GetType();
                    if (isCreature && !mLevel->canCreateMore(type, Level::eSpawnType_Breed))
                    {
                        player->displayClientMessage(MESSAGE_MOB_LIMIT, true);
                        return false;
                    }
                    break;
                }
                }

                usePlayerItem(player, itemInstance);
                setInLove(player);
                return true;
            }

            return false;
        }

        if (AgeUpBaby(player, itemInstance))
        {
            return true;
        }
    }

    return AgableMob::mobInteract(player, hand);
}

void Animal::setInLove(boost::shared_ptr<Player> player)
{
    setInLoveValue(600);

    if (player != NULL)
    {
        mLoveCause = player->getUUID();
    }

    mLevel->broadcastEntityEvent(shared_from_this(), EntityEvent::IN_LOVE_HEARTS, 0);
}

boost::shared_ptr<ServerPlayer> Animal::getLoveCause()
{
    if (mLoveCause.empty())
    {
        return boost::shared_ptr<ServerPlayer>();
    }

    boost::shared_ptr<Player> player = mLevel->getPlayerByUUID(mLoveCause);
    if (player != NULL && eTYPE_DERIVED_FROM(EntityType::SERVER_PLAYER, player->GetType()))
    {
        return boost::static_pointer_cast<ServerPlayer>(player);
    }

    return boost::shared_ptr<ServerPlayer>();
}

bool Animal::isInLove()
{
    return mEntityData->get(DATA_IN_LOVE) > 0;
}

void Animal::resetLove()
{
    mEntityData->set(DATA_IN_LOVE, 0, true);
    mEntityData->set(DATA_IN_LOVE_TIME, mBreedCooldown, true);
}

bool Animal::canMate(const boost::shared_ptr<Animal>& other)
{
    if (other == shared_from_this())
    {
        return false;
    }

    if (GetType() != other->GetType())
    {
        return false;
    }

    return isInLove() && other->isInLove();
}

void Animal::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::IN_LOVE_HEARTS)
    {
        for (int i = 0; i < 7; ++i)
        {
            double xa = mRandom->nextGaussian() * 0.02;
            double ya = mRandom->nextGaussian() * 0.02;
            double za = mRandom->nextGaussian() * 0.02;
            mLevel->addParticle(PARTICLE_HEART, mX + mRandom->nextFloat() * mBbWidth * 2.0f - mBbWidth,
                                mY + 0.5 + mRandom->nextFloat() * mBbHeight,
                                mZ + mRandom->nextFloat() * mBbWidth * 2.0f - mBbWidth, xa, ya, za,
                                arrayWithLength<int>());
        }

        return;
    }

    Mob::handleEntityEvent(event, data);
}

unsigned int Animal::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    if (isFood(holder.m_pItemInstance))
    {
        if (isBaby())
        {
            return IDS_TOOLTIPS_FEED;
        }

        if (!isInLove() && !isBreedCoolingDown() && !isBaby())
        {
            return IDS_TOOLTIPS_LOVEMODE;
        }
    }

    if (Item::getId(holder.m_pItemInstance->getItem()) == ItemId::LEAD && !isLeashed())
    {
        return IDS_TOOLTIPS_LEASH;
    }

    return AgableMob::GetInteractTooltip(holder);
}

bool Animal::isDespawnProtected()
{
    return mDespawnProtected;
}

void Animal::dropLeash(bool dropLeash, bool suppressDrops)
{
    setDespawnProtected();
    Entity::dropLeash(dropLeash, suppressDrops);
}

boost::shared_ptr<Entity> Animal::changeDimension(int dimension)
{
    boost::shared_ptr<Entity> entity = Entity::changeDimension(dimension);

    if (Entity::instanceof(entity, EntityType::ANIMAL))
    {
        boost::static_pointer_cast<Animal>(entity)->setDespawnProtected();
    }
    else if (entity != NULL)
    {
        entity->GetType() == GetType();
    }

    return entity;
}
