#ifndef GAME_ITEM_COMPLEXITEM_HPP
#define GAME_ITEM_COMPLEXITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class Level;
class Packet;
class Player;

class ComplexItem : public Item
{
public:
    ComplexItem();
    virtual ~ComplexItem() {}

    virtual bool isComplex();
    virtual boost::shared_ptr<Packet> getUpdatePacket(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                      boost::shared_ptr<Player> player);
};

#endif
