#include "game/network/ClientboundSetCarriedItemPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetCarriedItemPacket::ClientboundSetCarriedItemPacket()
{
    slot = 0;
}

ClientboundSetCarriedItemPacket::ClientboundSetCarriedItemPacket(int slot)
{
    this->slot = slot;
}

int ClientboundSetCarriedItemPacket::getPacketId()
{
    return 0x10;
}

void ClientboundSetCarriedItemPacket::read(DataInputStream* input)
{
    slot = input->readByte();
}

void ClientboundSetCarriedItemPacket::write(DataOutputStream* output)
{
    output->writeByte(slot);
}

void ClientboundSetCarriedItemPacket::handle(PacketListener* listener)
{
    listener->handleSetCarriedItem(shared_from_this());
}

int ClientboundSetCarriedItemPacket::getEstimatedSize()
{
    return 2;
}

bool ClientboundSetCarriedItemPacket::canBeInvalidated()
{
    return true;
}

bool ClientboundSetCarriedItemPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    return true;
}

int ClientboundSetCarriedItemPacket::getSlot()
{
    return slot;
}
