#ifndef GAME_WORLD_BLOCK_DISPENSERBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_DISPENSERBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/item/Container.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"

class AbstractContainerMenu;
class CompoundTag;
class DataFixerUpper;
class Inventory;
class ItemInstance;
class Player;

class DispenserBlockEntity : public RandomizableLockableContainerBlockEntity
{
public:
    DispenserBlockEntity();
    virtual ~DispenserBlockEntity();

    virtual NonNullList<not_null_ptr<ItemInstance> > getItems();
    virtual void setItems(NonNullList<not_null_ptr<ItemInstance> > items);
    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual int getRandomSlot();
    virtual int addItem(not_null_ptr<ItemInstance> item);
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual int getMaxStackSize();
    virtual int getTypeId(bool large);
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);

    virtual void setChanged();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual unsigned long long GetType();

    static void addDataWalkers(DataFixerUpper* fixer);
    static BlockEntity* create();

private:
    NonNullList<not_null_ptr<ItemInstance> > mItems;
};

#endif
