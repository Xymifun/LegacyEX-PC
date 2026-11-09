#include "game/network/ClientboundLevelEventPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/block/BlockPos.hpp"

ClientboundLevelEventPacket::ClientboundLevelEventPacket()
{
    type = 0;
    data = 0;
    pos = BlockPos();
}

ClientboundLevelEventPacket::ClientboundLevelEventPacket(int type, const BlockPos& pos, int data, bool globalEvent)
{
    this->type = type;
    this->pos = pos;
    this->data = data;
    this->globalEvent = globalEvent;
}

int ClientboundLevelEventPacket::getPacketId()
{
    return 61;
}

void ClientboundLevelEventPacket::read(DataInputStream* input)
{
    type = input->readInt();
    pos = input->readBlockPos();
    data = input->readInt();
    globalEvent = input->readBoolean();
}

void ClientboundLevelEventPacket::write(DataOutputStream* output)
{
    output->writeInt(type);
    output->writeBlockPos(pos);
    output->writeInt(data);
    output->writeBoolean(globalEvent);
}

void ClientboundLevelEventPacket::handle(PacketListener* listener)
{
    listener->handleLevelEvent(shared_from_this());
}

int ClientboundLevelEventPacket::getEstimatedSize()
{
    return 21;
}

bool ClientboundLevelEventPacket::isGlobalEvent()
{
    return globalEvent;
}

int ClientboundLevelEventPacket::getType()
{
    return type;
}

int ClientboundLevelEventPacket::getData()
{
    return data;
}

BlockPos ClientboundLevelEventPacket::getPos()
{
    return pos;
}
