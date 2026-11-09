#ifndef GAME_ITEM_REPAIRCONTAINER_HPP
#define GAME_ITEM_REPAIRCONTAINER_HPP

#include <boost/enable_shared_from_this.hpp>

#include "game/item/SimpleContainer.hpp"

class AnvilMenu;

class RepairContainer : public SimpleContainer, public boost::enable_shared_from_this<RepairContainer>
{
public:
    RepairContainer(AnvilMenu* menu, int id, bool hasCustomName, int size);
    virtual ~RepairContainer();

    virtual void setChanged();

private:
    AnvilMenu* mMenu;
};

#endif
