#ifndef GAME_UI_CHESTMENU_HPP
#define GAME_UI_CHESTMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"

class Container;

class ChestMenu : public AbstractContainerMenu
{
public:
    ChestMenu(boost::shared_ptr<Container> playerInventory, boost::shared_ptr<Container> container,
              boost::shared_ptr<Player> player);

    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual not_null_ptr<ItemInstance> clicked(int slot, int button, ClickType::eClickType type,
                                               boost::shared_ptr<Player> player);
    virtual void removed(boost::shared_ptr<Player> player);
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual boost::shared_ptr<Container> getContainer();

private:
    boost::shared_ptr<Container> m_container;
    int m_numRows;
};

#endif
