#ifndef GAME_WORLD_BLOCK_RANDOMIZABLELOCKABLECONTAINERBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_RANDOMIZABLELOCKABLECONTAINERBLOCKENTITY_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/CensoredString.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/item/LockableContainer.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/RandomizableContainer.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/LockableContainerBlockEntity.hpp"

class CompoundTag;
class ItemInstance;
class Player;

class RandomizableLockableContainerBlockEntity : public LockableContainerBlockEntity, public RandomizableContainer
{
public:
    RandomizableLockableContainerBlockEntity();
    virtual ~RandomizableLockableContainerBlockEntity();

    virtual ResourceLocation* getLootTable();
    virtual long long getLootTableSeed();
    virtual bool hasCustomName();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance);
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> itemInstance);
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();
    virtual InteractionObject::NameData GetNetworkNameData();
    virtual NonNullList<not_null_ptr<ItemInstance> > getItems() = 0;
    virtual void setItems(NonNullList<not_null_ptr<ItemInstance> > items) = 0;

    using BlockEntity::setChanged;

    void unpackLootTable(const boost::shared_ptr<Player>& player);
    void setLootTable(ResourceLocation* lootTable, long long seed);
    bool tryLoadLootTable(CompoundTag* tag);
    bool trySaveLootTable(CompoundTag* tag);
    void setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal);

protected:
    ResourceLocation* mLootTable;
    long long mLootTableSeed;
    boost::shared_ptr<CensoredString> mCustomName;
};

#endif
