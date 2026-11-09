#ifndef GAME_ITEM_SIMPLECONTAINER_HPP
#define GAME_ITEM_SIMPLECONTAINER_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/item/Container.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/network/net_minecraft_world__ContainerListener.hpp"

class ItemInstance;
class Player;

class SimpleContainer : public virtual Container
{
public:
    SimpleContainer(int id, const std::wstring& name, bool hasCustomName, int size);
    SimpleContainer(const std::wstring& name, int size);
    virtual ~SimpleContainer();

    virtual void addListener(net_minecraft_world::ContainerListener* listener);
    virtual void removeListener(net_minecraft_world::ContainerListener* listener);
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> addItem(not_null_ptr<ItemInstance> item);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> item);
    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual void setCustomName(const std::wstring& name);
    virtual std::wstring getDisplayName();
    virtual int getMaxStackSize();
    virtual void setChanged();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player) {}
    virtual void stopOpen(boost::shared_ptr<Player> player) {}
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> item);
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();

    void _init(int id, const std::wstring& name, bool hasCustomName, int size);

private:
    int mId;
    std::wstring mName;
    int mSize;
    NonNullList<not_null_ptr<ItemInstance> > mItems;
    std::vector<net_minecraft_world::ContainerListener*> mListeners;
    bool mHasCustomName;
};

#endif
