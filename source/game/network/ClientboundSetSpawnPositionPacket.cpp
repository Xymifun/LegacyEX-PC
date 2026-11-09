#include "game/network/ClientboundSetSpawnPositionPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetSpawnPositionPacket::ClientboundSetSpawnPositionPacket()
{
    mPos = BlockPos();
}
ClientboundSetSpawnPositionPacket::ClientboundSetSpawnPositionPacket(int x, int y, int z)
{
    mPos = BlockPos(x, y, z);
}
ClientboundSetSpawnPositionPacket::ClientboundSetSpawnPositionPacket(const BlockPos& pos)
{
    mPos = pos;
}
void ClientboundSetSpawnPositionPacket::read(DataInputStream* input)
{
    mPos = input->readBlockPos();
}
void ClientboundSetSpawnPositionPacket::write(DataOutputStream* output)
{
    output->writeBlockPos(mPos);
}
void ClientboundSetSpawnPositionPacket::handle(PacketListener* listener)
{
    listener->handleSetSpawn(shared_from_this());
}
int ClientboundSetSpawnPositionPacket::getEstimatedSize()
{
    return 12;
}
bool ClientboundSetSpawnPositionPacket::canBeInvalidated()
{
    return true;
}
bool ClientboundSetSpawnPositionPacket::isInvalidatedBy(boost::shared_ptr<Packet>)
{
    return true;
}
bool ClientboundSetSpawnPositionPacket::isAync()
{
    return false;
}
int ClientboundSetSpawnPositionPacket::getPacketId()
{
    return 6;
}
boost::shared_ptr<Packet> ClientboundSetSpawnPositionPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundSetSpawnPositionPacket());
}
