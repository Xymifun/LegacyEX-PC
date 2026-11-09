#ifndef GAME_UI_SHULKERBOXMENU_HPP
#define GAME_UI_SHULKERBOXMENU_HPP

#include <boost/shared_ptr.hpp>

#include "game/ui/AbstractContainerMenu.hpp"

class Container;
class Player;

class ShulkerBoxMenu : public AbstractContainerMenu
{
public:
    ShulkerBoxMenu(boost::shared_ptr<Container> inventory, boost::shared_ptr<Container> container,
                   boost::shared_ptr<Player> player);

    virtual not_null_ptr<ItemInstance> quickMoveStack(boost::shared_ptr<Player> player, int index);
    virtual void removed(boost::shared_ptr<Player> player);
    virtual bool stillValid(boost::shared_ptr<Player> player);

private:
    boost::shared_ptr<Container> mContainer;
};

#endif
