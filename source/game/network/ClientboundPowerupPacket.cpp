#include "game/network/ClientboundPowerupPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundPowerupPacket::ClientboundPowerupPacket()
{
    powerupID = -1;
    powerupTime = -1;
}

ClientboundPowerupPacket::ClientboundPowerupPacket(int powerupID, int powerupTime)
{
    this->powerupID = powerupID;
    this->powerupTime = powerupTime;
}

int ClientboundPowerupPacket::getPacketId()
{
    return 171;
}

void ClientboundPowerupPacket::read(DataInputStream* input)
{
    powerupID = input->readInt();
    powerupTime = input->readInt();
}

void ClientboundPowerupPacket::write(DataOutputStream* output)
{
    output->writeInt(powerupID);
    output->writeInt(powerupTime);
}

void ClientboundPowerupPacket::handle(PacketListener* listener)
{
    listener->handlePowerup(shared_from_this());
}

int ClientboundPowerupPacket::getEstimatedSize()
{
    return 2;
}

int ClientboundPowerupPacket::getPowerupID()
{
    return powerupID;
}

int ClientboundPowerupPacket::getPowerupTime()
{
    return powerupTime;
}
