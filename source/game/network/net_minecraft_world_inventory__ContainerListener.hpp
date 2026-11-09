#ifndef GAME_NETWORK_NET_MINECRAFT_WORLD_INVENTORY__CONTAINERLISTENER_HPP
#define GAME_NETWORK_NET_MINECRAFT_WORLD_INVENTORY__CONTAINERLISTENER_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"

class AbstractContainerMenu;
class Container;
class ItemInstance;

namespace net_minecraft_world_inventory
{

class ContainerListener
{
public:
    virtual ~ContainerListener() {}

    virtual void refreshContainer(AbstractContainerMenu* menu, std::vector<not_null_ptr<ItemInstance> >* items) = 0;
    virtual void slotChanged(AbstractContainerMenu* menu, int slotIndex, not_null_ptr<ItemInstance> item) = 0;
    virtual void setContainerData(AbstractContainerMenu* menu, int dataIndex, int value) = 0;
    virtual void setAllContainerData(AbstractContainerMenu* menu, boost::shared_ptr<Container> container) = 0;
};

}

#endif
