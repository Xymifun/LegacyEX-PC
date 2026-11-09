#ifndef GAME_UI_MERCHANTMENU_HPP
#define GAME_UI_MERCHANTMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"

class Inventory;
class Merchant;
class MerchantContainer;
class Level;

class MerchantMenu : public AbstractContainerMenu
{
public:
    MerchantMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Merchant> merchant, Level* level);

    boost::shared_ptr<MerchantContainer> getTradeContainer();
    virtual void slotsChanged();
    void setSelectionHint(int hint);
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual void removed(boost::shared_ptr<Player> player);
    boost::shared_ptr<Merchant> getMerchant();

private:
    boost::shared_ptr<Merchant> mMerchant;
    boost::shared_ptr<MerchantContainer> mTradeContainer;
    Level* mLevel;
};

#endif
