#ifndef GAME_ITEM_CONTAINER_HPP
#define GAME_ITEM_CONTAINER_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Nameable.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class Player;

class Container : public virtual Nameable
{
public:
    Container();
    virtual ~Container() {}

    virtual unsigned int getContainerSize() = 0;
    virtual bool isEmpty() = 0;
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot) = 0;
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count) = 0;
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot) = 0;
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance) = 0;
    virtual int getMaxStackSize() = 0;
    virtual void setChanged() = 0;
    virtual bool stillValid(boost::shared_ptr<Player> player) = 0;
    virtual void startOpen(boost::shared_ptr<Player> player) = 0;
    virtual void stopOpen(boost::shared_ptr<Player> player) = 0;
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> itemInstance) = 0;
    virtual int getContainerData(int id) = 0;
    virtual void setContainerData(int id, int value) = 0;
    virtual int getContainerDataCount() = 0;
    virtual void clearContent() = 0;

    bool isFull();
};

#endif
