#include "game/entity/mob/ItemEntity.hpp"
#include "game/core/EntityType.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Block.hpp"
#include "game/util/Vec3.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/Mth.hpp"
#include "game/core/MemSect.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GenericStats.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

#include <typeinfo>
#include <vector>

const EntityDataAccessor<not_null_ptr<ItemInstance> >* ItemEntity::DATA_ITEM = NULL;

ItemEntity::ItemEntity(Level* level, double x, double y, double z, not_null_ptr<ItemInstance> item)
    : Entity(level, true),
      mThrower(),
      mOwner()
{
    _init(level, x, y, z);
    setItem(item);
}

ItemEntity::ItemEntity(Level* level, double x, double y, double z) : Entity(level, true), mThrower(), mOwner()
{
    _init(level, x, y, z);
}

ItemEntity::ItemEntity(Level* level) : Entity(level, true), mThrower(), mOwner()
{
    _init();
    setItem(ItemInstance::EMPTY);
}

Entity* ItemEntity::create(Level* level)
{
    return new ItemEntity(level);
}

void ItemEntity::_init()
{
    fjDerivedCtorCalls();
    mAge = 0;
    mPickupDelay = 0;
    mHealth = 5;
    double bob = Math::random() * 3.1415927410125732;
    mBobOffs = (float)(bob + bob);
    setSize(0.25f, 0.25f);
    mCategories |= EntityCategory::eEntityCategory_Item;
}

void ItemEntity::_init(Level*, double x, double y, double z)
{
    _init();
    setPos(x, y, z);
    mYRot = (float)(Math::random() * 360.0);
    mXd = (double)(float)(Math::random() * 0.20000000298023224 - 0.10000000149011612);
    mYd = 0.20000000298023224;
    mZd = (double)(float)(Math::random() * 0.20000000298023224 - 0.10000000149011612);
}

void ItemEntity::setItem(not_null_ptr<ItemInstance> item)
{
    getEntityData()->set(DATA_ITEM, item, true);
    getEntityData()->markDirty(DATA_ITEM);
}

void ItemEntity::setDefaultPickUpDelay()
{
    mPickupDelay = 10;
}

void ItemEntity::addDataWalkers(DataFixerUpper* dataFixerUpper)
{
    dataFixerUpper->addWalker(DataFixTypes::eDataFixTypes_Entity, new ItemDataWalker(EntityType::ITEM_ENTITY, L"Item"));
}

void ItemEntity::staticCtor()
{
    DATA_ITEM = SynchedEntityData::defineId<not_null_ptr<ItemInstance> >(EntityType::ITEM_ENTITY, EntityType::ENTITY,
                                                                         EntityDataSerializers::ITEM_INSTANCE);
}

bool ItemEntity::makeStepSound()
{
    return false;
}

void ItemEntity::defineSynchedData()
{
    getEntityData()->define(DATA_ITEM, ItemInstance::EMPTY);
}

not_null_ptr<ItemInstance> ItemEntity::getItem()
{
    return getEntityData()->get(DATA_ITEM);
}

void ItemEntity::setUnderwaterMovement()
{
    mYd += 0.0005f;
    mYd = Mth::Lerp(mYd, 0.06, 0.01);
    mXd *= 0.99f;
    mZd *= 0.99f;
}

bool ItemEntity::merge(boost::shared_ptr<ItemEntity> entity)
{
    if (entity == shared_from_this())
        return false;
    if (!entity->isAlive() || !isAlive())
        return false;

    not_null_ptr<ItemInstance> thisItem = getItem();
    not_null_ptr<ItemInstance> otherItem = entity->getItem();
    if (mPickupDelay == 32767 || entity->mPickupDelay == 32767)
        return false;
    if (otherItem->getItem() != thisItem->getItem())
        return false;
    if (otherItem->hasTag() != thisItem->hasTag())
        return false;
    if (otherItem->hasTag() && !otherItem->getTag()->equals(thisItem->getTag()))
        return false;
    if (otherItem->getItem() == NULL)
        return false;
    if (otherItem->getItem()->isStackedByData() && otherItem->getAuxValue() != thisItem->getAuxValue())
        return false;
    if (otherItem->getCount() < thisItem->getCount())
        return entity->merge(boost::dynamic_pointer_cast<ItemEntity>(shared_from_this()));
    if (otherItem->getCount() + thisItem->getCount() > otherItem->getMaxStackSize())
        return false;

    otherItem->grow(thisItem->getCount());
    entity->mPickupDelay = mPickupDelay < entity->mPickupDelay ? entity->mPickupDelay : mPickupDelay;
    entity->mAge = entity->mAge < mAge ? entity->mAge : mAge;
    entity->setItem(otherItem);
    remove();
    return true;
}

void ItemEntity::mergeWithNeighbours()
{
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(*this),
                                                                                   mBoundingBox->grow(0.5, 0.0, 0.5));
    std::vector<boost::shared_ptr<Entity> >::iterator it;
    for (it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<ItemEntity> item = boost::dynamic_pointer_cast<ItemEntity>(*it);
        merge(item);
    }
    delete entities;
}

void ItemEntity::tick()
{
    PIXBeginNamedEvent(0.0f, "Item entity tick");
    if (getItem()->isEmpty())
    {
        remove();
        PIXEndNamedEvent();
        return;
    }

    Entity::tick();
    if (mPickupDelay > 0 && mPickupDelay != 32767)
        --mPickupDelay;

    double oldZd = mZd;
    mZo = mZ;
    double oldXd = mXd;
    mXo = mX;
    double oldYd = mYd;
    mYo = mY;

    if (mLevel->getBlockState(BlockPos(shared_from_this()))->getBlock()->mMaterial == Material::LAVA)
    {
        mYd = 0.2f;
        mXd = (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f;
        mZd = (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f;
        MemSect(31);
        playSound(SoundEvents::GENERIC_BURN, 0.4f, 2.0f + mRandom->nextFloat() * 0.4f);
        MemSect(0);
    }

    if (isUnderLiquid(Material::WATER) && mLevel->getBlock(BlockPos(getPos(0.0f))) != Blocks::flowing_water)
    {
        setNoGravity(true);
        setUnderwaterMovement();
    }
    else
    {
        setNoGravity(false);
    }

    if (!mLevel->mIsClientSide && mAge % 25 == 0)
        mergeWithNeighbours();
    if (mLevel->mIsClientSide)
    {
        mNoPhysics = false;
    }
    else
    {
        mNoPhysics = checkInBlock(mX, (mBoundingBox->mMinY + mBoundingBox->mMaxY) * 0.5, mZ);
    }

    PIXBeginNamedEvent(0.0f, "move");
    Vec3* motion = Vec3::newTemp(mXd, mYd, mZd);
    if (!mOnGround || motion->lengthSqr() > 0.01 || isInWater() || (mAge & 3) == 0)
    {
        if (!isNoGravity())
            mYd -= 0.04f;
        move(eMoverType_Self, mXd, mYd, mZd, true);

        float friction = 0.98f;
        if ((mVerticalCollision && mYd <= 0.0) || mOnGround)
        {
            friction = 0.6f * 0.98f;
            Block* below = mLevel->getBlock(BlockPos(mX, mBoundingBox->mMinY - 1.0, mZ));
            if (below != Blocks::air)
                friction = below->mFriction * 0.98f;
        }
        mXd *= friction;
        mYd *= 0.98f;
        mZd *= friction;
    }
    PIXEndNamedEvent();

    updateInWaterState();
    bool clientSide = mLevel->mIsClientSide;
    if (!clientSide
        && (mXd - oldXd) * (mXd - oldXd) + (mYd - oldYd) * (mYd - oldYd) + (mZd - oldZd) * (mZd - oldZd) > 0.01)
    {
        mHasImpulse = true;
    }
    ++mAge;
    if (!clientSide && mAge >= 6000)
        remove();
    PIXEndNamedEvent();
}

void ItemEntity::setShortLifeTime()
{
    mAge = 4800;
}

bool ItemEntity::updateInWaterState()
{
    bool inWater = mLevel->checkAndHandleWater(mBoundingBox, Material::WATER, shared_from_this());
    if (!inWater)
    {
        mIsInWater = false;
    }
    else
    {
        if (!mIsInWater && !mFirstTick)
            doWaterSplashEffect();
        mIsInWater = true;
    }
    return mIsInWater;
}

void ItemEntity::burn(int damage)
{
    hurt(DamageSource::IN_FIRE, static_cast<float>(damage));
}

bool ItemEntity::hurt(const DamageSource* source, float damage)
{
    if (mLevel->mIsClientSide || isInvulnerableTo(source))
        return false;
    bool immune = !getItem()->isEmpty() && getItem()->getItem() == Items::netherStar && source->isExplosion();
    if (!immune)
    {
        markHurt();
        mHealth -= static_cast<int>(damage);
        if (mHealth <= 0)
            remove();
    }
    return false;
}

std::wstring ItemEntity::getThrower()
{
    return mThrower;
}

std::wstring ItemEntity::getOwner()
{
    return mOwner;
}

void ItemEntity::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putShort(L"Health", static_cast<short>(mHealth));
    tag->putShort(L"Age", static_cast<short>(mAge));
    tag->putShort(L"PickupDelay", static_cast<short>(mPickupDelay));
    if (!getThrower().empty())
        tag->putString(L"Thrower", mThrower);
    if (!getOwner().empty())
        tag->putString(L"Owner", mOwner);
    if (!getItem()->isEmpty())
        tag->put(L"Item", getItem()->save(new CompoundTag()));
}

void ItemEntity::readAdditionalSaveData(CompoundTag* tag)
{
    mHealth = tag->getShort(L"Health");
    mAge = tag->getShort(L"Age");
    if (tag->contains(L"PickupDelay"))
        mPickupDelay = tag->getShort(L"PickupDelay");
    if (tag->contains(L"Owner"))
        mOwner = tag->getString(L"Owner");
    if (tag->contains(L"Thrower"))
        mThrower = tag->getString(L"Thrower");
    setItem(not_null_ptr<ItemInstance>(new ItemInstance(tag->getCompound(L"Item"))));
    if (getItem()->isEmpty())
        remove();
}

void ItemEntity::playerTouch(boost::shared_ptr<Player> player)
{
    if (mLevel->mIsClientSide)
        return;

    not_null_ptr<ItemInstance> item = getItem();
    if (item->isEmpty())
    {
        remove();
        return;
    }

    Item* itemType = item->getItem();
    int count = item->getCount();
    bool replacedExisting = false;
    if (mPickupDelay == 0 && !(!mOwner.empty() && 6000 - mAge > 200 && mOwner.compare(player->getName()) != 0)
        && (player->AutoEquip(item, item, replacedExisting) || player->mInventory->add(item)))
    {
        if (itemType == Items::diamond)
        {
            player->awardStat(GenericStats::diamonds(), GenericStats::param_diamonds());
            if (!getThrower().empty())
            {
                boost::shared_ptr<Player> thrower = mLevel->getPlayerByName(getThrower());
                if (thrower && thrower != player)
                    thrower->awardStat(GenericStats::diamondsToYou(), GenericStats::param_diamondsToYou());
            }
        }
        if (itemType == Items::blazeRod)
            player->awardStat(GenericStats::blazeRod(), GenericStats::param_blazeRod());

        player->take(shared_from_this(), count);
        if (item->isEmpty())
        {
            remove();
            item->setCount(count);
        }
    }
}

std::wstring ItemEntity::getName()
{
    if (hasCustomName())
        return getCustomName();
    return L"";
}

boost::shared_ptr<Entity> ItemEntity::changeDimension(int dimension)
{
    boost::shared_ptr<Entity> newEntity = Entity::changeDimension(dimension);
    if (!mLevel->mIsClientSide && newEntity && eTYPE_DERIVED_FROM(EntityType::ITEM_ENTITY, newEntity->GetType()))
        boost::static_pointer_cast<ItemEntity>(newEntity)->mergeWithNeighbours();
    return newEntity;
}

bool ItemEntity::isAttackable()
{
    return false;
}

void ItemEntity::setOwner(const std::wstring& owner)
{
    mOwner = owner;
}

void ItemEntity::setThrower(const std::wstring& thrower)
{
    mThrower = thrower;
}

void ItemEntity::setNoPickUpDelay()
{
    mPickupDelay = 0;
}

void ItemEntity::setNeverPickUp()
{
    mPickupDelay = 32767;
}

void ItemEntity::setPickUpDelay(int delay)
{
    mPickupDelay = delay;
}

bool ItemEntity::hasPickUpDelay()
{
    return mPickupDelay > 0;
}

void ItemEntity::setExtendedLifetime()
{
    mAge = -6000;
}

void ItemEntity::makeFakeItem()
{
    setNeverPickUp();
    mAge = 5999;
}

unsigned long long ItemEntity::GetType()
{
    return EntityType::ITEM_ENTITY;
}
