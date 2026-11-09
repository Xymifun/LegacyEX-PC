#ifndef GAME_ITEM_RESULTCONTAINER_HPP
#define GAME_ITEM_RESULTCONTAINER_HPP

#include <string>

#include "game/item/Container.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;

class ResultContainer : public Container
{
public:
    ResultContainer();
    virtual ~ResultContainer();

    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance);
    virtual int getMaxStackSize();
    virtual void setChanged();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> itemInstance);
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();

    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::wstring getDisplayName();

private:
    NonNullList<not_null_ptr<ItemInstance> > items;
};

#endif
