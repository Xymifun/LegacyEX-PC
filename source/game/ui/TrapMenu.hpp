#ifndef GAME_UI_TRAPMENU_HPP
#define GAME_UI_TRAPMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"

class Container;
class Player;

class TrapMenu : public AbstractContainerMenu
{
public:
    TrapMenu(boost::shared_ptr<Container> playerInventory, boost::shared_ptr<Container> container);

    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual bool stillValid(boost::shared_ptr<Player> player);

private:
    boost::shared_ptr<Container> m_container;
};

#endif
