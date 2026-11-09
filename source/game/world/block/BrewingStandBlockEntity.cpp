#include "game/world/block/BrewingStandBlockEntity.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include <cstring>

#include "game/core/Direction.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/core/DataWalker.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Containers.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/PotionBrewing.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BrewingStandMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BrewingStandBlock.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/Player.hpp"

static int brewingUpSlots[] = {3};
static int brewingDownSlots[] = {0, 1, 2, 3};
static int brewingSideSlots[] = {0, 1, 2, 4};

arrayWithLength<int> BrewingStandBlockEntity::SLOTS_FOR_UP(brewingUpSlots, 1);
arrayWithLength<int> BrewingStandBlockEntity::SLOTS_FOR_DOWN(brewingDownSlots, 4);
arrayWithLength<int> BrewingStandBlockEntity::SLOTS_FOR_SIDES(brewingSideSlots, 4);

BrewingStandBlockEntity::BrewingStandBlockEntity()
    : mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(5, ItemInstance::EMPTY)),
      mLastPotionBits()
{
    mLastPotionBits = arrayWithLength<unsigned char>();
    mLastIngredientItem = NULL;
    mBrewTime = 0;
    mFuel = 0;
    mCustomName = boost::shared_ptr<CensoredString>(new CensoredString());
}

BrewingStandBlockEntity::~BrewingStandBlockEntity()
{
    delete[] mLastPotionBits.data;
    delete[] mItems.items.data;
}

std::wstring BrewingStandBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"");
}

std::wstring BrewingStandBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName->get();
    }
    return std::wstring(L"");
}

bool BrewingStandBlockEntity::hasCustomName()
{
    if (!mCustomName->checkParentalFilter())
    {
        return false;
    }
    return !mCustomName->raw().empty();
}

unsigned int BrewingStandBlockEntity::getContainerSize()
{
    return mItems.size();
}

bool BrewingStandBlockEntity::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        not_null_ptr<ItemInstance> stack = mItems[i];
        if (!stack->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> BrewingStandBlockEntity::getItem(unsigned int slot)
{
    if (slot < mItems.size())
    {
        return mItems[slot];
    }
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> BrewingStandBlockEntity::removeItem(unsigned int slot, int count)
{
    return ContainerHelper::removeItem(mItems, slot, count);
}

not_null_ptr<ItemInstance> BrewingStandBlockEntity::removeItemNoUpdate(int slot)
{
    return ContainerHelper::takeItem(mItems, (unsigned int)slot);
}

void BrewingStandBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance)
{
    if (slot < mItems.size())
    {
        mItems[slot] = itemInstance;
    }
}

int BrewingStandBlockEntity::getMaxStackSize()
{
    return 64;
}

bool BrewingStandBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (mLevel->getBlockEntity(mPos) != shared_from_this())
    {
        return false;
    }
    if (player->distanceAsIfHitscanned(mPos, *player->mBoundingBox) > SharedConstants::CONTAINER_INTERACT_DISTANCE_SQR)
    {
        return false;
    }
    return true;
}

void BrewingStandBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
}

void BrewingStandBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
}

bool BrewingStandBlockEntity::canPlaceItem(int slot, not_null_ptr<ItemInstance> itemInstance)
{
    if (slot == 3)
    {
        return PotionBrewing::isIngredient(itemInstance);
    }
    Item* item = itemInstance->getItem();
    if (slot == 4)
    {
        return item == Items::blazePowder;
    }
    return (item == Items::potion || item == Items::splashPotion || item == Items::lingeringPotion
            || item == Items::glassBottle)
           && getItem(slot)->isEmpty();
}

int BrewingStandBlockEntity::getContainerData(int id)
{
    switch (id)
    {
    case 0:
        return mBrewTime;
    case 1:
        return mFuel;
    default:
        return 0;
    }
}

void BrewingStandBlockEntity::setContainerData(int id, int value)
{
    switch (id)
    {
    case 0:
        mBrewTime = value;
        break;
    case 1:
        mFuel = value;
        break;
    }
}

int BrewingStandBlockEntity::getContainerDataCount()
{
    return 2;
}

void BrewingStandBlockEntity::clearContent()
{
    mItems.clear();
}

AbstractContainerMenu* BrewingStandBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                           boost::shared_ptr<Player> player)
{
    return new BrewingStandMenu(inventory, boost::dynamic_pointer_cast<Container>(shared_from_this()));
}

int BrewingStandBlockEntity::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::BREWING_STAND;
}

InteractionObject::NameData BrewingStandBlockEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(mCustomName->raw(), mCustomName->getUUID(), mCustomName->isUUIDLocal(),
                                       mCustomName->isChatCensored());
}

arrayWithLength<int> BrewingStandBlockEntity::getSlotsForFace(const Direction* face)
{
    if (face == Direction::UP)
    {
        return SLOTS_FOR_UP;
    }
    if (face == Direction::DOWN)
    {
        return SLOTS_FOR_DOWN;
    }
    return SLOTS_FOR_SIDES;
}

bool BrewingStandBlockEntity::canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> itemInstance,
                                                      const Direction* face)
{
    return canPlaceItem(slot, itemInstance);
}

bool BrewingStandBlockEntity::canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> itemInstance,
                                                     const Direction* face)
{
    if (slot == 3)
    {
        return itemInstance->getItem() == Items::glassBottle;
    }
    return true;
}

void BrewingStandBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored,
                                            bool uuidLocal)
{
    mCustomName->set(name, uuid, chatCensored, uuidLocal);
}

void BrewingStandBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(BlockEntityType::BREWING_STAND, L"Items"));
}

bool BrewingStandBlockEntity::isBrewable()
{
    not_null_ptr<ItemInstance> ingredient = mItems[3];
    if (ingredient->isEmpty())
    {
        return false;
    }
    if (!PotionBrewing::isIngredient(ingredient))
    {
        return false;
    }
    for (unsigned int i = 0; i < 3; ++i)
    {
        not_null_ptr<ItemInstance> potion = mItems[i];
        if (potion->isEmpty())
        {
            continue;
        }
        if (PotionBrewing::hasMix(potion, ingredient))
        {
            return true;
        }
    }
    return false;
}

arrayWithLength<unsigned char> BrewingStandBlockEntity::getPotionBits()
{
    arrayWithLength<unsigned char> bits(3, true);
    for (unsigned int i = 0; i < 3; ++i)
    {
        bits.data[i] = mItems[i]->isEmpty() ? 0 : 1;
    }
    return bits;
}

void BrewingStandBlockEntity::doBrew()
{
    not_null_ptr<ItemInstance> ingredient = mItems[3];
    for (int i = 0; i < 3; ++i)
    {
        mItems.set(i, PotionBrewing::mix(ingredient, mItems[i]));
    }
    ingredient->shrink(1);
    BlockPos pos = getBlockPos();
    if (ingredient->getItem()->hasCraftingRemainingItem())
    {
        not_null_ptr<ItemInstance> remaining(new ItemInstance(ingredient->getItem()->getCraftingRemainingItem()));
        if (ingredient->isEmpty())
        {
            ingredient = remaining;
        }
        else
        {
            Containers::dropItemStack(mLevel, (double)pos.x, (double)pos.y, (double)pos.z, remaining);
        }
    }
    mItems[3] = ingredient;
    mLevel->levelEvent(LevelEvent::SOUND_BREWING_STAND_BREW, pos, 0);
    --mFuel;
}

void BrewingStandBlockEntity::tick()
{
    not_null_ptr<ItemInstance> fuel = mItems[4];
    if (mFuel < 1)
    {
        if (fuel->getItem() != NULL && fuel->getItem() == Items::blazePowder)
        {
            mFuel = 20;
            fuel->shrink(1);
            setChanged();
        }
    }

    bool brewable = isBrewable();
    bool brewing = mBrewTime > 0;
    not_null_ptr<ItemInstance> ingredient = mItems[3];
    if (brewing)
    {
        if (!brewable)
        {
            mBrewTime = 0;
            setChanged();
        }
        else
        {
            if (--mBrewTime == 0)
            {
                doBrew();
                setChanged();
            }
            else if (mItems[3]->getItem() != mLastIngredientItem)
            {
                mBrewTime = 0;
                setChanged();
            }
        }
    }
    else if (brewable && mFuel > 0)
    {
        mBrewTime = 400;
        mLastIngredientItem = ingredient->getItem();
        setChanged();
    }

    if (!mLevel->mIsClientSide)
    {
        arrayWithLength<unsigned char> potionBits = getPotionBits();
        if (potionBits.length == mLastPotionBits.length
            && memcmp(potionBits.data, mLastPotionBits.data, potionBits.length) == 0)
        {
            delete[] potionBits.data;
            return;
        }

        delete[] mLastPotionBits.data;
        mLastPotionBits = potionBits;
        const BlockState* state = mLevel->getBlockState(getBlockPos());
        if (dynamic_cast<BrewingStandBlock*>(state->getBlock()) != NULL)
        {
            TypedBoxed<bool> bottle0(potionBits.data[0] != 0);
            state = state->setBoxedValue(BrewingStandBlock::HAS_BOTTLE_0, &bottle0);
            TypedBoxed<bool> bottle1(potionBits.data[1] != 0);
            state = state->setBoxedValue(BrewingStandBlock::HAS_BOTTLE_1, &bottle1);
            TypedBoxed<bool> bottle2(potionBits.data[2] != 0);
            state = state->setBoxedValue(BrewingStandBlock::HAS_BOTTLE_2, &bottle2);
            mLevel->setBlock(mPos, state, 2, false);
        }
    }
}

void BrewingStandBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mCustomName->loadFromTag(tag);
    delete[] mItems.items.data;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    ContainerHelper::loadAllItems(tag, mItems);
    mBrewTime = tag->getShort(L"BrewTime");
    mFuel = tag->getByte(L"Fuel");
}

CompoundTag* BrewingStandBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mCustomName->saveToTag(tag);
    tag->putShort(L"BrewTime", (short)mBrewTime);
    ContainerHelper::saveAllItems(tag, mItems);
    tag->putByte(L"Fuel", (unsigned char)mFuel);
    return tag;
}

boost::shared_ptr<BlockEntity> BrewingStandBlockEntity::clone()
{
    boost::shared_ptr<BrewingStandBlockEntity> copy(new BrewingStandBlockEntity());
    BlockEntity::clone(copy);
    copy->mBrewTime = mBrewTime;
    copy->mLastPotionBits = mLastPotionBits;
    copy->mLastIngredientItem = mLastIngredientItem;
    copy->mFuel = mFuel;
    copy->mCustomName->set(mCustomName->raw(), std::wstring(mCustomName->getUUID()), mCustomName->isChatCensored(),
                           mCustomName->isUUIDLocal());
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        copy->mItems[i] = mItems[i]->copy();
    }
    return copy;
}

std::wstring BrewingStandBlockEntity::getDisplayName()
{
    return LockableContainerBlockEntity::getDisplayName();
}

void BrewingStandBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

BlockEntity* BrewingStandBlockEntity::create()
{
    return new BrewingStandBlockEntity();
}

unsigned long long BrewingStandBlockEntity::GetType()
{
    return BlockEntityType::BREWING_STAND;
}
