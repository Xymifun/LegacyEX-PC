#include "game/network/ServerboundAcceptTeleportationPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerboundAcceptTeleportationPacket::ServerboundAcceptTeleportationPacket()
{
}

ServerboundAcceptTeleportationPacket::ServerboundAcceptTeleportationPacket(int id)
{
    this->id = id;
}

int ServerboundAcceptTeleportationPacket::getPacketId()
{
    return 135;
}

void ServerboundAcceptTeleportationPacket::read(DataInputStream* input)
{
    id = input->readVarInt();
}

void ServerboundAcceptTeleportationPacket::write(DataOutputStream* output)
{
    output->writeVarInt(id);
}

void ServerboundAcceptTeleportationPacket::handle(PacketListener* listener)
{
    listener->handleAcceptTeleportPacket(shared_from_this());
}

int ServerboundAcceptTeleportationPacket::getId()
{
    return id;
}
