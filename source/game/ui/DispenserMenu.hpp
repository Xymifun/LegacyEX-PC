#ifndef GAME_UI_DISPENSERMENU_HPP
#define GAME_UI_DISPENSERMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"

class Container;

class DispenserMenu : public AbstractContainerMenu
{
public:
    DispenserMenu(boost::shared_ptr<Container> playerInventory, boost::shared_ptr<Container> dispenser);
    virtual ~DispenserMenu();

    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual bool stillValid(boost::shared_ptr<Player> player);

private:
    boost::shared_ptr<Container> m_dispenser;
};

#endif
