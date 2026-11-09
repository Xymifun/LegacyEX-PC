#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/core/EntityType.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/ListTag.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/core/BaseAttributeMap.hpp"
#include "game/core/DataWalker.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/item/Container.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/Items.hpp"
#include "game/item/SimpleContainer.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/client/audio/SoundEvents.hpp"

const EntityDataAccessor<bool>* AbstractChestedHorse::DATA_CHEST = NULL;

AbstractChestedHorse::AbstractChestedHorse(Level* level) : AbstractHorse(level)
{
    mCanGallop = false;
}

void AbstractChestedHorse::defineSynchedData()
{
    AbstractHorse::defineSynchedData();
    mEntityData->createDateItem(DATA_CHEST, false);
}

void AbstractChestedHorse::registerAttributes()
{
    AbstractHorse::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(generateRandomMaxHealth());
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.175f);
    getAttribute(JUMP_STRENGTH)->setBaseValue(0.5);
}

bool AbstractChestedHorse::hasChest()
{
    return mEntityData->get(DATA_CHEST);
}

void AbstractChestedHorse::setChest(bool chest)
{
    mEntityData->set(DATA_CHEST, chest, true);
}

int AbstractChestedHorse::getInventorySize()
{
    return hasChest() ? 17 : AbstractHorse::getInventorySize();
}

double AbstractChestedHorse::getRideHeight()
{
    return Entity::getRideHeight() - 0.25;
}

const SoundEvent* AbstractChestedHorse::getAngrySound()
{
    return SoundEvents::DONKEY_ANGRY;
}

void AbstractChestedHorse::die(const DamageSource* source)
{
    AbstractHorse::die(source);
    if (hasChest())
    {
        if (!mLevel->mIsClientSide)
        {
            spawnAtLocation(Item::byBlock(Blocks::chest), 1);
        }
        setChest(false);
    }
}

void AbstractChestedHorse::addDataWalkers(DataFixerUpper* fixer, unsigned long long type)
{
    AbstractHorse::addDataWalkers(fixer, type);
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity, (DataWalker*)new ItemListDataWalker(type, L"Items"));
}

void AbstractChestedHorse::staticCtor()
{
    DATA_CHEST = SynchedEntityData::defineId<bool>(EntityType::ABSTRACT_CHESTED_HORSE, EntityType::ABSTRACT_HORSE,
                                                   EntityDataSerializers::BOOLEAN);
}

void AbstractChestedHorse::addAdditonalSaveData(CompoundTag* tag)
{
    AbstractHorse::addAdditonalSaveData(tag);
    tag->putBoolean(L"ChestedHorse", hasChest());
    if (hasChest())
    {
        ListTag* items = new ListTag();
        for (int slot = 2; slot < mInventory->getContainerSize(); ++slot)
        {
            not_null_ptr<ItemInstance> item = mInventory->getItem((unsigned int)slot);
            if (!item->isEmpty())
            {
                CompoundTag* itemTag = new CompoundTag();
                itemTag->putByte(L"Slot", (unsigned char)slot);
                item->save(itemTag);
                items->add(itemTag);
            }
        }
        tag->put(L"Items", items);
    }
}

void AbstractChestedHorse::readAdditionalSaveData(CompoundTag* tag)
{
    AbstractHorse::readAdditionalSaveData(tag);
    setChest(tag->getBoolean(L"ChestedHorse"));
    if (hasChest())
    {
        ListTag* items = tag->getList(L"Items", Tag::TAG_COMPOUND);
        createInventory();
        for (int i = 0; i < items->size(); ++i)
        {
            CompoundTag* itemTag = items->getCompound(i);
            unsigned int slot = itemTag->getByte(L"Slot");
            if (slot >= 2 && slot < (unsigned int)mInventory->getContainerSize())
            {
                mInventory->setItem((unsigned int)slot, not_null_ptr<ItemInstance>(new ItemInstance(itemTag)));
            }
        }
    }
    updateEquipment();
}

bool AbstractChestedHorse::setSlot(int slot, const not_null_ptr<ItemInstance>& item)
{
    if (slot == 499)
    {
        if (hasChest() && item->isEmpty())
        {
            setChest(false);
            createInventory();
            return true;
        }
        if (!hasChest() && item->getItem() == Item::byBlock(Blocks::chest))
        {
            setChest(true);
            createInventory();
            return true;
        }
    }
    return AbstractHorse::setSlot(slot, item);
}

bool AbstractChestedHorse::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    if (item->getItem() == Items::spawnEgg)
        return AbstractHorse::mobInteract(player, hand);
    if (isLeashedToPlayer(player))
        return false;

    if (!isBaby())
    {
        if (isTamed() && player->isSneaking())
        {
            openInventory(player);
            return true;
        }
        if (isVehicle())
            return AbstractHorse::mobInteract(player, hand);
    }

    if (!item->isEmpty())
    {
        if (handleEating(player, item))
        {
            if (!player->mAbilities.mInstabuild)
                item->shrink(1);
            return true;
        }

        if (!isTamed())
        {
            if (!item->interactEnemy(player, boost::static_pointer_cast<LivingEntity>(shared_from_this()), hand))
                makeMad();
            return true;
        }

        if (!hasChest() && item->getItem() == Item::byBlock(Blocks::chest))
        {
            if (!mLevel->mIsClientSide)
            {
                setChest(true);
                playChestEquipsSound();
                createInventory();
            }
            else
            {
                playChestEquipsSound();
            }
            if (!player->mAbilities.mInstabuild)
                item->shrink(1);
            return true;
        }

        if (!isBaby() && !isSaddled() && item->getItem() == Items::saddle)
        {
            openInventory(player);
            return true;
        }
    }

    if (isBaby())
        return AbstractHorse::mobInteract(player, hand);

    if (!item->interactEnemy(player, boost::static_pointer_cast<LivingEntity>(shared_from_this()), hand))
        doPlayerRide(player);
    return true;
}

void AbstractChestedHorse::playChestEquipsSound()
{
    playSound(SoundEvents::DONKEY_CHEST, 1.0f, 1.0f + (mRandom->nextFloat() - mRandom->nextFloat()) * 0.2f);
}

int AbstractChestedHorse::getInventoryColumns()
{
    return 5;
}

bool AbstractChestedHorse::removeWhenFarAway()
{
    return AbstractHorse::removeWhenFarAway() && !hasChest();
}

unsigned long long AbstractChestedHorse::GetType()
{
    return EntityType::ABSTRACT_CHESTED_HORSE;
}
