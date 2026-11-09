#include "game/item/ComplexItem.hpp"

ComplexItem::ComplexItem() : Item()
{
}

bool ComplexItem::isComplex()
{
    return true;
}

boost::shared_ptr<Packet> ComplexItem::getUpdatePacket(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                       boost::shared_ptr<Player> player)
{
    return boost::shared_ptr<Packet>();
}
