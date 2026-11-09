#ifndef GAME_ITEM_MERCHANTCONTAINER_HPP
#define GAME_ITEM_MERCHANTCONTAINER_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/item/Container.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class Merchant;
class MerchantRecipe;
class MerchantRecipeList;
class Player;

class MerchantContainer : public Container
{
public:
    MerchantContainer(boost::shared_ptr<Player> player, boost::shared_ptr<Merchant> merchant);
    virtual ~MerchantContainer();
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::wstring getDisplayName();
    void updateSellItem();
    void setSelectionHint(int hint);
    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> item);
    virtual int getMaxStackSize();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> item);
    virtual void setChanged();
    MerchantRecipe* getActiveRecipe();
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();
    bool isPaymentSlot(int slot);

private:
    boost::shared_ptr<Merchant> mMerchant;
    NonNullList<not_null_ptr<ItemInstance> > mItems;
    boost::shared_ptr<Player> mTradingPlayer;
    MerchantRecipe* mActiveRecipe;
    int mSelectionHint;
};

#endif
