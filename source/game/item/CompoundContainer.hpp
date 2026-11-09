#ifndef GAME_ITEM_COMPOUNDCONTAINER_HPP
#define GAME_ITEM_COMPOUNDCONTAINER_HPP

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/item/LockableContainer.hpp"

class AbstractContainerMenu;
class ChestMenu;
class Inventory;
class ItemInstance;
class Player;

class CompoundContainer : public LockableContainer, public boost::enable_shared_from_this<CompoundContainer>
{
public:
    CompoundContainer(int id, boost::shared_ptr<LockableContainer> first, boost::shared_ptr<LockableContainer> second);
    virtual ~CompoundContainer();

    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual bool contains(boost::shared_ptr<Container> container);
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::wstring getDisplayName();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> item);
    virtual int getMaxStackSize();
    virtual void setChanged();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> item);
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();

    virtual bool isLocked();
    virtual void setLocked(LockCode* lockCode);
    virtual LockCode* getLockKeyCode();
    virtual void unlock();
    virtual int getTypeId(bool large);
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);

private:
    int mId;
    boost::shared_ptr<LockableContainer> mFirst;
    boost::shared_ptr<LockableContainer> mSecond;
};

#endif
