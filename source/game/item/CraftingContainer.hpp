#ifndef GAME_ITEM_CRAFTINGCONTAINER_HPP
#define GAME_ITEM_CRAFTINGCONTAINER_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/item/Container.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"

class AbstractContainerMenu;
class ItemInstance;
class Player;

class CraftingContainer : public Container
{
public:
    CraftingContainer(AbstractContainerMenu* menu, unsigned int width, unsigned int height);
    virtual ~CraftingContainer();

    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::wstring getDisplayName();
    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    not_null_ptr<ItemInstance> getItem(unsigned int row, unsigned int column);
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

    int getHeight();
    int getWidth();

private:
    NonNullList<not_null_ptr<ItemInstance> > mItems;
    unsigned int mWidth;
    unsigned int mHeight;
    AbstractContainerMenu* mMenu;
};

#endif
