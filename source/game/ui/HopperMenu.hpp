#ifndef GAME_UI_HOPPERMENU_HPP
#define GAME_UI_HOPPERMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/Container.hpp"
#include "game/ui/AbstractContainerMenu.hpp"

class Player;

class HopperMenu : public AbstractContainerMenu
{
public:
    HopperMenu(boost::shared_ptr<Container> playerContainer, boost::shared_ptr<Container> container,
               boost::shared_ptr<Player> player);

    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual void removed(boost::shared_ptr<Player> player);
    virtual bool stillValid(boost::shared_ptr<Player> player);
    boost::shared_ptr<Container> getContainer();

private:
    boost::shared_ptr<Container> m_container;
};

#endif
