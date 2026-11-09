#include "game/network/ClientboundSetTimePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetTimePacket::ClientboundSetTimePacket()
{
    gameTime = 0;
    dayTime = 0;
}

ClientboundSetTimePacket::ClientboundSetTimePacket(long long gameTime, long long dayTime, bool daylightCycle)
{
    this->gameTime = gameTime;
    this->dayTime = dayTime;
}

int ClientboundSetTimePacket::getPacketId()
{
    return 4;
}

void ClientboundSetTimePacket::read(DataInputStream* input)
{
    gameTime = input->readLong();
    dayTime = input->readLong();
}

void ClientboundSetTimePacket::write(DataOutputStream* output)
{
    output->writeLong(gameTime);
    output->writeLong(dayTime);
}

void ClientboundSetTimePacket::handle(PacketListener* listener)
{
    listener->handleSetTime(shared_from_this());
}

int ClientboundSetTimePacket::getEstimatedSize()
{
    return 16;
}

bool ClientboundSetTimePacket::canBeInvalidated()
{
    return true;
}

bool ClientboundSetTimePacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    return true;
}

bool ClientboundSetTimePacket::isAync()
{
    return true;
}

long long ClientboundSetTimePacket::getGameTime()
{
    return gameTime;
}

long long ClientboundSetTimePacket::getDayTime()
{
    return dayTime;
}
