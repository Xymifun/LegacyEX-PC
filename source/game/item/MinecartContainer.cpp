#include "game/item/MinecartContainer.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/StringIds.hpp"

#include <boost/pointer_cast.hpp>

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/L10N.hpp"
#include "game/core/LootTable.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/Containers.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

MinecartContainer::MinecartContainer(Level* level, double x, double y, double z)
    : Minecart(level, x, y, z),
      LockableContainer(),
      mItems()
{
    _init();
}

MinecartContainer::MinecartContainer(Level* level) : Minecart(level), LockableContainer(), mItems()
{
    _init();
}

MinecartContainer::~MinecartContainer()
{
    delete[] mItems.items.data;
}

void MinecartContainer::_init()
{
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(36, ItemInstance::EMPTY);
    mDropContentsWhenDead = true;
    mLootTable = NULL;
    mLootTableSeed = 0;
}

std::wstring MinecartContainer::getName()
{
    if (hasCustomName())
    {
        return getCustomName();
    }
    return std::wstring(L10N::GetString(IDS_CONTAINER_MINECART));
}

std::wstring MinecartContainer::getCustomName()
{
    return Entity::getCustomName();
}

bool MinecartContainer::hasCustomName()
{
    return Entity::hasCustomName();
}

std::wstring MinecartContainer::getDisplayName()
{
    return Entity::getDisplayName();
}

void MinecartContainer::unpackLootTable(const boost::shared_ptr<Player>& player)
{
    if (mLootTable == NULL)
    {
        return;
    }

    LootTable* lootTable = BuiltInLootTables::getLootTable(mLootTable);
    if (lootTable == NULL)
    {
        return;
    }

    mLootTable = NULL;
    Random* random = mLootTableSeed == 0 ? new Random() : new Random(mLootTableSeed);
    LootTableContext::Builder* builder = new LootTableContext::Builder(static_cast<ServerLevel*>(mLevel));
    if (player)
    {
        builder->withLuck(player->getLuck());
    }
    LootTableContext context = builder->build();
    lootTable->fill(static_cast<Container*>(this), random, context);
    delete builder;
}

bool MinecartContainer::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = mItems[i];
        if (!item->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> MinecartContainer::getItem(unsigned int slot)
{
    unpackLootTable(boost::shared_ptr<Player>());
    return mItems[slot];
}

not_null_ptr<ItemInstance> MinecartContainer::removeItem(unsigned int slot, int count)
{
    unpackLootTable(boost::shared_ptr<Player>());
    return ContainerHelper::removeItem(mItems, slot, count);
}

not_null_ptr<ItemInstance> MinecartContainer::removeItemNoUpdate(int slot)
{
    unpackLootTable(boost::shared_ptr<Player>());
    not_null_ptr<ItemInstance> item = mItems[slot];
    if (item->isEmpty())
    {
        return ItemInstance::EMPTY;
    }
    mItems.set(slot, ItemInstance::EMPTY);
    return item;
}

void MinecartContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    unpackLootTable(boost::shared_ptr<Player>());
    mItems[slot] = item;
    if (!item->isEmpty() && item->getCount() > getMaxStackSize())
    {
        item->setCount(getMaxStackSize());
    }
}

int MinecartContainer::getMaxStackSize()
{
    return 64;
}

void MinecartContainer::setChanged()
{
    (void)mItems;
}

bool MinecartContainer::stillValid(boost::shared_ptr<Player> player)
{
    if (mRemoved)
    {
        return false;
    }
    if (player->distanceAsIfHitscanned(BlockPos(Vec3::newTemp(mX, mY, mZ)), *player->mBoundingBox)
        > SharedConstants::CONTAINER_INTERACT_DISTANCE_SQR)
    {
        return false;
    }
    return true;
}

bool MinecartContainer::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return true;
}

int MinecartContainer::getContainerData(int id)
{
    (void)id;
    return 0;
}

void MinecartContainer::setContainerData(int id, int value)
{
    (void)id;
    (void)value;
}

int MinecartContainer::getContainerDataCount()
{
    return 0;
}

void MinecartContainer::clearContent()
{
    unpackLootTable(boost::shared_ptr<Player>());
    mItems.clear();
}

bool MinecartContainer::isLocked()
{
    return false;
}

void MinecartContainer::setLocked(LockCode* lockCode)
{
    (void)lockCode;
}

LockCode* MinecartContainer::getLockKeyCode()
{
    return LockCode::NO_LOCK;
}

void MinecartContainer::unlock()
{
    (void)mLootTable;
}

ResourceLocation* MinecartContainer::getLootTable()
{
    return mLootTable;
}

long long MinecartContainer::getLootTableSeed()
{
    return mLootTableSeed;
}

void MinecartContainer::addDataWalkers(DataFixerUpper* fixer, unsigned long long entityType)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity, (DataWalker*)new ItemListDataWalker(entityType, L"Items"));
}

void MinecartContainer::destroy(const DamageSource* source)
{
    Minecart::destroy(source);
    if (mLevel->getGameRules()->getBoolean(GameRules::RULE_DOENTITYDROPS))
    {
        Containers::dropContents(mLevel, shared_from_this(),
                                 boost::static_pointer_cast<MinecartContainer>(shared_from_this()));
    }
}

boost::shared_ptr<Entity> MinecartContainer::changeDimension(int dimension)
{
    mDropContentsWhenDead = false;
    return Entity::changeDimension(dimension);
}

void MinecartContainer::remove()
{
    if (mDropContentsWhenDead)
    {
        Containers::dropContents(mLevel, shared_from_this(),
                                 boost::static_pointer_cast<MinecartContainer>(shared_from_this()));
    }
    Entity::remove();
}

void MinecartContainer::addAdditonalSaveData(CompoundTag* tag)
{
    Minecart::addAdditonalSaveData(tag);
    if (mLootTable != NULL && StructureFeatureIO::getVersion() >= 2)
    {
        tag->putString(L"LootTable", mLootTable->toString());
        if (mLootTableSeed != 0)
        {
            tag->putLong(L"LootTableSeed", mLootTableSeed);
        }
    }
    else
    {
        ContainerHelper::saveAllItems(tag, mItems);
    }
}

void MinecartContainer::readAdditionalSaveData(CompoundTag* tag)
{
    Minecart::readAdditionalSaveData(tag);
    delete[] mItems.items.data;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(getContainerSize(), ItemInstance::EMPTY);
    if (tag->contains(L"LootTable", Tag::TAG_STRING) && StructureFeatureIO::getVersion() >= 2)
    {
        mLootTable = new ResourceLocation(tag->getString(L"LootTable"));
        mLootTableSeed = tag->getLong(L"LootTableSeed");
    }
    else
    {
        ContainerHelper::loadAllItems(tag, mItems);
    }
}

bool MinecartContainer::interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    if (!player->isAllowedToInteract(shared_from_this()))
    {
        return false;
    }

    if (!mLevel->mIsClientSide)
    {
        player->openContainer(boost::static_pointer_cast<MinecartContainer>(shared_from_this()));
    }
    return true;
}

void MinecartContainer::applyNaturalSlowdown()
{
    float slowdown = 0.98f;
    if (mLootTable == NULL)
    {
        boost::shared_ptr<Container> container = boost::static_pointer_cast<MinecartContainer>(shared_from_this());
        int signal = Block::SIGNAL_MAX - AbstractContainerMenu::getRedstoneSignalFromContainer(container);
        slowdown += signal * 0.001f;
    }
    mXd *= slowdown;
    mYd *= 0.0;
    mZd *= slowdown;
}

void MinecartContainer::setDropContainerContent(bool dropContents)
{
    mDropContentsWhenDead = dropContents;
}

void MinecartContainer::setLootTable(ResourceLocation* lootTable, long long seed)
{
    mLootTable = lootTable;
    mLootTableSeed = seed;
}

unsigned int MinecartContainer::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    return tooltipData.m_bAllowedToInteract ? IDS_TOOLTIPS_OPEN : IDS_NONE;
}
