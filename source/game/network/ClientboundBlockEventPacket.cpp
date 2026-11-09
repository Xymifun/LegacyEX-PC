#include "game/network/ClientboundBlockEventPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

ClientboundBlockEventPacket::ClientboundBlockEventPacket()
{
    pos = BlockPos();
    b0 = 0;
    b1 = 0;
    block = NULL;
}

ClientboundBlockEventPacket::ClientboundBlockEventPacket(const BlockPos& pos, Block* block, int b0, int b1)
{
    this->pos = pos;
    this->b0 = b0;
    this->b1 = b1;
    this->block = block;
}

int ClientboundBlockEventPacket::getPacketId()
{
    return 54;
}

void ClientboundBlockEventPacket::read(DataInputStream* input)
{
    pos = input->readBlockPos();
    b0 = input->readUnsignedByte();
    b1 = input->readUnsignedByte();
    block = Block::byId(input->readShort() & 0xFFF);
}

void ClientboundBlockEventPacket::write(DataOutputStream* output)
{
    output->writeBlockPos(pos);
    output->writeByte(b0);
    output->writeByte(b1);
    output->writeShort(Block::getId(block) & 0xFFF);
}

void ClientboundBlockEventPacket::handle(PacketListener* listener)
{
    listener->handleBlockEvent(shared_from_this());
}

int ClientboundBlockEventPacket::getEstimatedSize()
{
    return 14;
}

BlockPos ClientboundBlockEventPacket::getPos()
{
    return pos;
}

int ClientboundBlockEventPacket::getB0()
{
    return b0;
}

int ClientboundBlockEventPacket::getB1()
{
    return b1;
}

Block* ClientboundBlockEventPacket::getBlock()
{
    return block;
}
