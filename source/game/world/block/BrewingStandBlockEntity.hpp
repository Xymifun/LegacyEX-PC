#ifndef GAME_WORLD_BLOCK_BREWINGSTANDBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_BREWINGSTANDBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/CensoredString.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/item/Container.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/WorldlyContainer.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/LockableContainerBlockEntity.hpp"

class CompoundTag;
class DataFixerUpper;
class Direction;
class Inventory;
class Item;
class ItemInstance;
class Player;
class BrewingStandMenu;
class AbstractContainerMenu;

class BrewingStandBlockEntity : public LockableContainerBlockEntity, public WorldlyContainer
{
public:
    BrewingStandBlockEntity();
    virtual ~BrewingStandBlockEntity();

    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance);
    virtual int getMaxStackSize();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> itemInstance);
    virtual arrayWithLength<int> getSlotsForFace(const Direction* face);
    virtual bool canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> itemInstance, const Direction* face);
    virtual bool canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> itemInstance, const Direction* face);
    virtual int getTypeId(bool large);
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();
    virtual InteractionObject::NameData GetNetworkNameData();

    void setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal);
    static void addDataWalkers(DataFixerUpper* fixer);
    bool isBrewable();
    arrayWithLength<unsigned char> getPotionBits();
    void doBrew();
    virtual void tick();
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual std::wstring getDisplayName();
    virtual void setChanged();
    static BlockEntity* create();
    virtual unsigned long long GetType();
    int getBrewTime();

private:
    static arrayWithLength<int> SLOTS_FOR_UP;
    static arrayWithLength<int> SLOTS_FOR_DOWN;
    static arrayWithLength<int> SLOTS_FOR_SIDES;

    NonNullList<not_null_ptr<ItemInstance> > mItems;
    int mBrewTime;
    arrayWithLength<unsigned char> mLastPotionBits;
    Item* mLastIngredientItem;
    int mFuel;
    boost::shared_ptr<CensoredString> mCustomName;
};

#endif
