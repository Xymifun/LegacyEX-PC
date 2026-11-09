#include "game/network/ClientboundBlockDestructionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/block/BlockPos.hpp"

ClientboundBlockDestructionPacket::ClientboundBlockDestructionPacket()
{
    id = 0;
    pos = BlockPos();
    progress = 0;
}

ClientboundBlockDestructionPacket::ClientboundBlockDestructionPacket(int id, const BlockPos& pos, int progress)
{
    this->id = id;
    this->pos = pos;
    this->progress = progress;
}

int ClientboundBlockDestructionPacket::getPacketId()
{
    return 55;
}

void ClientboundBlockDestructionPacket::read(DataInputStream* input)
{
    id = input->readInt();
    pos = input->readBlockPos();
    progress = input->readUnsignedByte();
}

void ClientboundBlockDestructionPacket::write(DataOutputStream* output)
{
    output->writeInt(id);
    output->writeBlockPos(pos);
    output->writeByte((unsigned char)progress);
}

void ClientboundBlockDestructionPacket::handle(PacketListener* listener)
{
    listener->handleBlockDestruction(shared_from_this());
}

int ClientboundBlockDestructionPacket::getEstimatedSize()
{
    return 13;
}

int ClientboundBlockDestructionPacket::getId()
{
    return id;
}

BlockPos ClientboundBlockDestructionPacket::getPos()
{
    return pos;
}

int ClientboundBlockDestructionPacket::getProgress()
{
    return progress;
}
