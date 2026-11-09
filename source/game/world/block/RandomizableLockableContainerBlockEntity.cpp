#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"

#include "game/core/LootTable.hpp"
#include "game/core/LootTableContext.hpp"
#include "game/core/LootTables.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Container.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

RandomizableLockableContainerBlockEntity::RandomizableLockableContainerBlockEntity() : LockableContainerBlockEntity()
{
    mLootTable = NULL;
    mLootTableSeed = 0;
    mCustomName = boost::shared_ptr<CensoredString>(new CensoredString());
}

RandomizableLockableContainerBlockEntity::~RandomizableLockableContainerBlockEntity()
{
}

ResourceLocation* RandomizableLockableContainerBlockEntity::getLootTable()
{
    return mLootTable;
}

long long RandomizableLockableContainerBlockEntity::getLootTableSeed()
{
    return mLootTableSeed;
}

bool RandomizableLockableContainerBlockEntity::hasCustomName()
{
    return !mCustomName->raw().empty() && mCustomName->checkParentalFilter() != 0;
}

not_null_ptr<ItemInstance> RandomizableLockableContainerBlockEntity::getItem(unsigned int slot)
{
    unpackLootTable(boost::shared_ptr<Player>());
    return getItems()[slot];
}

not_null_ptr<ItemInstance> RandomizableLockableContainerBlockEntity::removeItem(unsigned int slot, int count)
{
    unpackLootTable(boost::shared_ptr<Player>());
    not_null_ptr<ItemInstance> item = ContainerHelper::removeItem(getItems(), slot, static_cast<unsigned int>(count));
    if (!item->isEmpty())
    {
        setChanged();
    }
    return item;
}

not_null_ptr<ItemInstance> RandomizableLockableContainerBlockEntity::removeItemNoUpdate(int slot)
{
    unpackLootTable(boost::shared_ptr<Player>());
    return ContainerHelper::takeItem(getItems(), static_cast<unsigned int>(slot));
}

void RandomizableLockableContainerBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance)
{
    unpackLootTable(boost::shared_ptr<Player>());
    getItems()[slot] = itemInstance;
    if (itemInstance->getCount() > getMaxStackSize())
    {
        itemInstance->setCount(getMaxStackSize());
    }
    setChanged();
}

bool RandomizableLockableContainerBlockEntity::stillValid(boost::shared_ptr<Player> player)
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

void RandomizableLockableContainerBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
}

void RandomizableLockableContainerBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
}

bool RandomizableLockableContainerBlockEntity::canPlaceItem(int slot, not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}

int RandomizableLockableContainerBlockEntity::getContainerData(int id)
{
    return 0;
}

void RandomizableLockableContainerBlockEntity::setContainerData(int id, int value)
{
}

int RandomizableLockableContainerBlockEntity::getContainerDataCount()
{
    return 0;
}

void RandomizableLockableContainerBlockEntity::clearContent()
{
    unpackLootTable(boost::shared_ptr<Player>());
    getItems().clear();
}

InteractionObject::NameData RandomizableLockableContainerBlockEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(mCustomName->raw(), mCustomName->getUUID(), mCustomName->isUUIDLocal(),
                                       mCustomName->isChatCensored());
}

void RandomizableLockableContainerBlockEntity::unpackLootTable(const boost::shared_ptr<Player>& player)
{
    if (mLootTable == NULL)
    {
        return;
    }
    if (mLevel == NULL)
    {
        return;
    }

    LootTable* lootTable = static_cast<LootTables*>(mLevel->getLootTables())->get(mLootTable);
    if (lootTable == NULL)
    {
        return;
    }
    mLootTable = NULL;

    Random* random = mLootTableSeed == 0 ? new Random() : new Random(mLootTableSeed);

    LootTableContext::Builder* builder = new LootTableContext::Builder(static_cast<ServerLevel*>(mLevel));
    builder->withLootedEntity(player);
    if (player)
    {
        builder->withLuck(player->getLuck());
    }
    LootTableContext context = builder->build();
    lootTable->fill(this, random, context);
    delete builder;
}

void RandomizableLockableContainerBlockEntity::setLootTable(ResourceLocation* lootTable, long long seed)
{
    mLootTable = lootTable;
    mLootTableSeed = seed;
}

bool RandomizableLockableContainerBlockEntity::tryLoadLootTable(CompoundTag* tag)
{
    if (tag != NULL && tag->contains(std::wstring(L"LootTable"), 8))
    {
        mLootTable = new ResourceLocation(tag->getString(std::wstring(L"LootTable")));
        mLootTableSeed = tag->getLong(std::wstring(L"LootTableSeed"));
        return true;
    }
    return false;
}

bool RandomizableLockableContainerBlockEntity::trySaveLootTable(CompoundTag* tag)
{
    if (tag != NULL && mLootTable != NULL)
    {
        tag->putString(std::wstring(L"LootTable"), mLootTable->toString());
        if (mLootTableSeed != 0)
        {
            tag->putLong(std::wstring(L"LootTableSeed"), mLootTableSeed);
        }
        return true;
    }
    return false;
}

void RandomizableLockableContainerBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid,
                                                             bool chatCensored, bool uuidLocal)
{
    mCustomName->set(name, uuid, chatCensored, uuidLocal);
}
