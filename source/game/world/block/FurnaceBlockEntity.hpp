#ifndef GAME_WORLD_BLOCK_FURNACEBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_FURNACEBLOCKENTITY_HPP

#include <set>
#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/CensoredString.hpp"
#include "game/core/Direction.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemListDataWalker.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/WorldlyContainer.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/LockableContainerBlockEntity.hpp"

class AbstractContainerMenu;
class Inventory;
class Player;
class FurnaceMenu;

class FurnaceBlockEntity : public LockableContainerBlockEntity, public WorldlyContainer
{
public:
    FurnaceBlockEntity();
    virtual ~FurnaceBlockEntity();
    virtual unsigned long long GetType();

    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> item);
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual int getMaxStackSize();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> item);
    virtual arrayWithLength<int> getSlotsForFace(const Direction* face);
    virtual bool canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face);
    virtual bool canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face);
    virtual int getTypeId(bool large);
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();
    virtual InteractionObject::NameData GetNetworkNameData();

    virtual void setChanged();
    virtual std::wstring getDisplayName();
    void setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal);
    int getTotalCookTime(not_null_ptr<ItemInstance> item);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    bool isLit();
    static bool isLit(boost::shared_ptr<Container> container);
    bool canBurn();
    void burn();
    virtual void tick();
    virtual boost::shared_ptr<BlockEntity> clone();
    static BlockEntity* create();
    static int getBurnDuration(not_null_ptr<ItemInstance> item);
    static bool isFuel(not_null_ptr<ItemInstance> item);
    bool wasCharcoalUsed();

private:
    static arrayWithLength<int> SLOTS_FOR_UP;
    static arrayWithLength<int> SLOTS_FOR_DOWN;
    static arrayWithLength<int> SLOTS_FOR_SIDES;

    NonNullList<not_null_ptr<ItemInstance> > mItems;
    bool mCharcoalUsed;
    std::set<boost::shared_ptr<Player> > mOpenersThisTick;
    std::set<boost::shared_ptr<Player> > mOpeners;
    int mLitTime;
    int mLitDuration;
    int mCookingProgress;
    int mCookingTotalTime;
    boost::shared_ptr<CensoredString> mName;
};

#endif
