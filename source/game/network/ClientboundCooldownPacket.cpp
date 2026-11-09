#include "game/network/ClientboundCooldownPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/item/Item.hpp"
#include "game/network/PacketListener.hpp"

ClientboundCooldownPacket::ClientboundCooldownPacket() : item(NULL), duration(0)
{
}

ClientboundCooldownPacket::ClientboundCooldownPacket(Item* item, int duration) : item(item), duration(duration)
{
}

int ClientboundCooldownPacket::getPacketId()
{
    return 236;
}

void ClientboundCooldownPacket::read(DataInputStream* input)
{
    item = Item::byId(input->readVarInt());
    duration = input->readVarInt();
}

void ClientboundCooldownPacket::write(DataOutputStream* output)
{
    output->writeVarInt(Item::getId(item));
    output->writeVarInt(duration);
}

void ClientboundCooldownPacket::handle(PacketListener* listener)
{
    listener->handleItemCooldown(boost::static_pointer_cast<ClientboundCooldownPacket>(shared_from_this()));
}

Item* ClientboundCooldownPacket::getItem()
{
    return item;
}

int ClientboundCooldownPacket::getDuration()
{
    return duration;
}
