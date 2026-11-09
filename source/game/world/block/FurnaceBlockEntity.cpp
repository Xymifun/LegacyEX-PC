#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/FurnaceFuelSlot.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/item/BoatItem.hpp"
#include "game/item/DiggerItem.hpp"
#include "game/item/DoorItem.hpp"
#include "game/item/HoeItem.hpp"
#include "game/item/SwordItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/ui/FurnaceMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FurnaceBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

static int furnaceUpSlots[] = {0};
static int furnaceDownSlots[] = {2, 1};
static int furnaceSideSlots[] = {1};

arrayWithLength<int> FurnaceBlockEntity::SLOTS_FOR_UP(furnaceUpSlots, 1);
arrayWithLength<int> FurnaceBlockEntity::SLOTS_FOR_DOWN(furnaceDownSlots, 2);
arrayWithLength<int> FurnaceBlockEntity::SLOTS_FOR_SIDES(furnaceSideSlots, 1);

FurnaceBlockEntity::FurnaceBlockEntity()
    : LockableContainerBlockEntity(),
      WorldlyContainer(),
      mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(3, ItemInstance::EMPTY)),
      mOpenersThisTick(),
      mOpeners()
{
    mLitTime = 0;
    mLitDuration = 0;
    mCookingProgress = 0;
    mCookingTotalTime = 0;
    mCharcoalUsed = false;
    mName = boost::shared_ptr<CensoredString>(new CensoredString());
}

FurnaceBlockEntity::~FurnaceBlockEntity()
{
    delete[] mItems.items.data;
}

BlockEntity* FurnaceBlockEntity::create()
{
    return new FurnaceBlockEntity();
}

std::wstring FurnaceBlockEntity::getName()
{
    if (hasCustomName())
        return mName->get();
    return std::wstring(L"");
}

std::wstring FurnaceBlockEntity::getCustomName()
{
    if (hasCustomName())
        return mName->get();
    return std::wstring(L"");
}

bool FurnaceBlockEntity::hasCustomName()
{
    if (!mName->checkParentalFilter())
        return false;
    return !mName->raw().empty();
}

unsigned int FurnaceBlockEntity::getContainerSize()
{
    return mItems.size();
}

bool FurnaceBlockEntity::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = mItems[i];
        if (!item->isEmpty())
            return false;
    }
    return true;
}

not_null_ptr<ItemInstance> FurnaceBlockEntity::getItem(unsigned int slot)
{
    return mItems[slot];
}

not_null_ptr<ItemInstance> FurnaceBlockEntity::removeItem(unsigned int slot, int count)
{
    return ContainerHelper::removeItem(mItems, slot, count);
}

not_null_ptr<ItemInstance> FurnaceBlockEntity::removeItemNoUpdate(int slot)
{
    return ContainerHelper::takeItem(mItems, slot);
}

int FurnaceBlockEntity::getTotalCookTime(not_null_ptr<ItemInstance> item)
{
    return 200;
}

void FurnaceBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    not_null_ptr<ItemInstance> previous = mItems[slot];
    bool same = !item->isEmpty() && item->sameItem(previous) && ItemInstance::tagMatches(item, previous);
    mItems[slot] = item;
    if (item->getCount() > getMaxStackSize())
        item->setCount(getMaxStackSize());
    if (slot == 0 && !same)
    {
        mCookingTotalTime = getTotalCookTime(item);
        mCookingProgress = 0;
        setChanged();
    }
}

int FurnaceBlockEntity::getMaxStackSize()
{
    return 64;
}

void FurnaceBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

bool FurnaceBlockEntity::stillValid(boost::shared_ptr<Player> player)
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

void FurnaceBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
    mOpeners.insert(player);
    mOpenersThisTick.insert(player);
}

void FurnaceBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
    mOpeners.erase(mOpeners.find(player));
}

int FurnaceBlockEntity::getBurnDuration(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
        return 0;
    Item* itemType = item->getItem();
    if (itemType == Item::byBlock(Blocks::wooden_slab))
        return 150;
    if (itemType == Item::byBlock(Blocks::wool))
        return 100;
    if (itemType == Item::byBlock(Blocks::carpet))
        return 67;
    if (itemType != Item::byBlock(Blocks::ladder))
    {
        if (itemType == Item::byBlock(Blocks::wooden_button))
            return 100;
        if (itemType == Item::byBlock(Blocks::acacia_button))
            return 100;
        if (itemType == Item::byBlock(Blocks::birch_button))
            return 100;
        if (itemType == Item::byBlock(Blocks::dark_oak_button))
            return 100;
        if (itemType == Item::byBlock(Blocks::jungle_button))
            return 100;
        if (itemType == Item::byBlock(Blocks::spruce_button))
            return 100;
        Block* block = Block::byItem(itemType);
        const BlockState* state = block->defaultBlockState();
        Material* material = state->getMaterial();
        if (material != Material::WOOD)
        {
            if (itemType == Item::byBlock(Blocks::coal_block))
                return 16000;
            DiggerItem* digger = dynamic_cast<DiggerItem*>(itemType);
            if (digger != NULL && digger->getTier() == Item::Tier::WOOD)
                return 200;
            SwordItem* sword = dynamic_cast<SwordItem*>(itemType);
            if (sword != NULL && sword->getTier() == Item::Tier::WOOD)
                return 200;
            HoeItem* hoe = dynamic_cast<HoeItem*>(itemType);
            if (hoe != NULL && hoe->getTier() == Item::Tier::WOOD)
                return 200;
            if (itemType == Items::stick)
                return 100;
            if (itemType != Items::bow && itemType != Items::fishingRod)
            {
                if (itemType == Items::sign)
                    return 200;
                if (itemType == Items::coal)
                    return 1600;
                if (itemType == Items::lavaBucket)
                    return 20000;
                if (itemType != Item::byBlock(Blocks::sapling) && itemType != Items::bowl)
                {
                    if (itemType == Items::blazeRod)
                        return 2400;
                    if (itemType == Item::byBlock(Blocks::dried_kelp_block))
                        return 4000;
                    if (itemType == NULL)
                        return 0;
                    if (dynamic_cast<DoorItem*>(itemType) != NULL && itemType != Items::ironDoor)
                        return 200;
                    return dynamic_cast<BoatItem*>(itemType) != NULL ? 400 : 0;
                }
                return 100;
            }
            return 300;
        }
    }
    return 300;
}

bool FurnaceBlockEntity::isFuel(not_null_ptr<ItemInstance> item)
{
    return getBurnDuration(item) > 0;
}

bool FurnaceBlockEntity::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    if (slot == 2)
        return false;
    if (slot == 1)
    {
        not_null_ptr<ItemInstance> fuel = mItems[1];
        return isFuel(item) || (FurnaceFuelSlot::isBucket(item) && fuel->getItem() != Items::bucket);
    }
    return true;
}

int FurnaceBlockEntity::getContainerData(int id)
{
    switch (id)
    {
    case 0:
        return mLitTime;
    case 1:
        return mLitDuration;
    case 2:
        return mCookingProgress;
    case 3:
        return mCookingTotalTime;
    case 4:
        return mCharcoalUsed ? 1 : 0;
    }
    return 0;
}

void FurnaceBlockEntity::setContainerData(int id, int value)
{
    switch (id)
    {
    case 0:
        mLitTime = value;
        break;
    case 1:
        mLitDuration = value;
        break;
    case 2:
        mCookingProgress = value;
        break;
    case 3:
        mCookingTotalTime = value;
        break;
    }
}

int FurnaceBlockEntity::getContainerDataCount()
{
    return 6;
}

void FurnaceBlockEntity::clearContent()
{
    mItems.clear();
}

AbstractContainerMenu* FurnaceBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                      boost::shared_ptr<Player> player)
{
    return new FurnaceMenu(inventory, boost::static_pointer_cast<FurnaceBlockEntity>(shared_from_this()));
}

int FurnaceBlockEntity::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::FURNACE;
}

InteractionObject::NameData FurnaceBlockEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(mName->raw(), mName->getUUID(), mName->isUUIDLocal(), mName->isChatCensored());
}

std::wstring FurnaceBlockEntity::getDisplayName()
{
    return LockableContainerBlockEntity::getDisplayName();
}

arrayWithLength<int> FurnaceBlockEntity::getSlotsForFace(const Direction* face)
{
    if (face == Direction::DOWN)
        return SLOTS_FOR_DOWN;
    if (face == Direction::UP)
        return SLOTS_FOR_UP;
    return SLOTS_FOR_SIDES;
}

bool FurnaceBlockEntity::canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return canPlaceItem(slot, item);
}

bool FurnaceBlockEntity::canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    if (face == Direction::DOWN && slot == 1)
    {
        Item* itemType = item->getItem();
        if (itemType != Items::waterBucket && itemType != Items::bucket)
            return false;
    }
    return true;
}

void FurnaceBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored,
                                       bool uuidLocal)
{
    mName->set(name, uuid, chatCensored, uuidLocal);
}

void FurnaceBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                     (DataWalker*)new ItemListDataWalker(BlockEntityType::FURNACE, L"Items"));
}

void FurnaceBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    delete[] mItems.items.data;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    ContainerHelper::loadAllItems(tag, mItems);
    mLitTime = tag->getShort(L"BurnTime");
    mCookingProgress = tag->getShort(L"CookTime");
    mCookingTotalTime = tag->getShort(L"CookTimeTotal");
    mLitDuration = std::max(mLitTime, getBurnDuration(mItems[1]));
    mName->loadFromTag(tag);
    mCharcoalUsed = tag->getBoolean(L"CharcoalUsed");
}

CompoundTag* FurnaceBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putShort(L"BurnTime", (short)mLitTime);
    tag->putShort(L"CookTime", (short)mCookingProgress);
    tag->putShort(L"CookTimeTotal", (short)mCookingTotalTime);
    ContainerHelper::saveAllItems(tag, mItems);
    mName->saveToTag(tag);
    tag->putBoolean(L"CharcoalUsed", mCharcoalUsed);
    return tag;
}

bool FurnaceBlockEntity::isLit()
{
    return mLitTime > 0;
}

bool FurnaceBlockEntity::isLit(boost::shared_ptr<Container> container)
{
    return container->getContainerData(0) > 0;
}

bool FurnaceBlockEntity::canBurn()
{
    if (mItems[0]->isEmpty())
        return false;
    ItemInstance* result = FurnaceRecipes::getInstance()->getResult(mItems[0]);
    if (result->isEmpty())
        return false;
    not_null_ptr<ItemInstance> output = mItems[2];
    if (output->isEmpty())
        return true;
    if (!output->sameItem_not_shared(result))
        return false;
    if (output->getCount() < getMaxStackSize() && output->getCount() < output->getMaxStackSize())
        return true;
    if (output->getCount() < result->getMaxStackSize())
        return true;
    return false;
}

void FurnaceBlockEntity::burn()
{
    if (!canBurn())
        return;
    ItemInstance* result = FurnaceRecipes::getInstance()->getResult(mItems[0]);
    if (!mItems[2]->isEmpty())
    {
        if (mItems[2]->getItem() == result->getItem())
            mItems[2]->grow(1);
    }
    else
    {
        mItems[2] = result->copy();
    }
    if (mItems[0]->getItem() == Item::byBlock(Blocks::sponge) && mItems[0]->getAuxValue() == 1 && !mItems[1]->isEmpty()
        && mItems[1]->getItem() == Items::bucket)
    {
        mItems[1] = not_null_ptr<ItemInstance>(new ItemInstance(Items::waterBucket));
    }
    mItems[0]->shrink(1);
}

void FurnaceBlockEntity::tick()
{
    bool wasLit = isLit();
    bool changed = false;
    if (isLit())
        --mLitTime;
    if (mLevel == NULL)
        return;
    if (mLevel->mIsClientSide)
        return;

    {
        not_null_ptr<ItemInstance> fuel = mItems[1];
        if (isLit() || (!fuel->isEmpty() && !mItems[0]->isEmpty()))
        {
            if (!isLit() && canBurn())
            {
                mLitTime = getBurnDuration(fuel);
                mLitDuration = mLitTime;
                if (isLit())
                {
                    changed = true;
                    if (!fuel->isEmpty())
                    {
                        Item* fuelItem = fuel->getItem();
                        if (fuelItem == Items::coal && fuel->getAuxValue() == 1)
                        {
                            mCharcoalUsed = true;
                        }
                        else if (fuelItem == Items::lavaBucket)
                        {
                            for (std::set<boost::shared_ptr<Player> >::iterator it = mOpenersThisTick.begin();
                                 it != mOpenersThisTick.end(); ++it)
                            {
                                boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(*it);
                                player->awardStat(GenericStats::superFuel(), GenericStats::param_superFuel());
                            }
                        }
                        else if (fuelItem == Item::byBlock(Blocks::dried_kelp_block))
                        {
                            for (std::set<boost::shared_ptr<Player> >::iterator it = mOpenersThisTick.begin();
                                 it != mOpenersThisTick.end(); ++it)
                            {
                                boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(*it);
                                player->awardStat(GenericStats::alternativeFuel(),
                                                  GenericStats::param_alternativeFuel());
                            }
                        }
                        fuel->shrink(1);
                        if (fuel->isEmpty())
                        {
                            Item* remainder = fuelItem->getCraftingRemainingItem();
                            mItems[1] = (remainder == NULL) ? ItemInstance::EMPTY
                                                            : not_null_ptr<ItemInstance>(new ItemInstance(remainder));
                        }
                    }
                }
                mOpenersThisTick = mOpeners;
            }

            if (isLit() && canBurn())
            {
                ++mCookingProgress;
                if (mCookingProgress == mCookingTotalTime)
                {
                    mCookingProgress = 0;
                    mCookingTotalTime = getTotalCookTime(mItems[0]);
                    burn();
                    changed = true;
                }
            }
            else
            {
                mCookingProgress = 0;
            }
        }
        else if (!isLit() && mCookingProgress > 0)
        {
            mCookingProgress = Mth::clamp(mCookingProgress - 2, 0, mCookingTotalTime);
        }
        bool nowLit = isLit();
        if (wasLit != nowLit)
        {
            changed = true;
            FurnaceBlock::setLit(nowLit, mLevel, mPos);
        }
    }
    if (changed)
        setChanged();
}

boost::shared_ptr<BlockEntity> FurnaceBlockEntity::clone()
{
    boost::shared_ptr<FurnaceBlockEntity> result(new FurnaceBlockEntity());
    BlockEntity::clone(result);
    result->mCookingProgress = mCookingProgress;
    result->mCookingTotalTime = mCookingTotalTime;
    result->mLitTime = mLitTime;
    result->mLitDuration = mLitDuration;
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        result->mItems.set(i, mItems[i]->copy());
    }
    result->mName->set(mName->raw(), mName->getUUID(), mName->isChatCensored(), mName->isUUIDLocal());
    return result;
}

unsigned long long FurnaceBlockEntity::GetType()
{
    return BlockEntityType::FURNACE;
}
