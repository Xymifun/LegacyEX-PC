#ifndef GAME_UI_HORSEINVENTORYMENU_HPP
#define GAME_UI_HORSEINVENTORYMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/ui/AbstractContainerMenu.hpp"

class AbstractHorse;
class Container;
class Player;

class HorseInventoryMenu : public AbstractContainerMenu
{
private:
    boost::shared_ptr<Container> m_container;

public:
    HorseInventoryMenu(boost::shared_ptr<Container> playerInventory, boost::shared_ptr<Container> horseContainer,
                       boost::shared_ptr<AbstractHorse> horseEntity, boost::shared_ptr<Player> player);

    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual void removed(boost::shared_ptr<Player> player);
    virtual bool stillValid(boost::shared_ptr<Player> player);
    boost::shared_ptr<Container> getContainer();

    boost::shared_ptr<AbstractHorse> horse;
};

#endif
