#ifndef GAME_UI_LOCALMENUCONTAINER_HPP
#define GAME_UI_LOCALMENUCONTAINER_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/item/SimpleContainer.hpp"
#include "game/item/LockableContainer.hpp"

class AbstractContainerMenu;
class Inventory;
class Player;
class LockCode;

class LocalMenuContainer : public SimpleContainer, public LockableContainer
{
public:
    LocalMenuContainer(int typeId, const std::wstring& name, int size);
    virtual ~LocalMenuContainer();

    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::wstring getDisplayName();
    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
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
    int mTypeId;
    boost::unordered_map<int, int> mContainerData;
};

#endif
