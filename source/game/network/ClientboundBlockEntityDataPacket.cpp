#include "game/network/ClientboundBlockEntityDataPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/block/BlockPos.hpp"

ClientboundBlockEntityDataPacket::ClientboundBlockEntityDataPacket()
{
    _init();
    shouldDelay = true;
}

ClientboundBlockEntityDataPacket::ClientboundBlockEntityDataPacket(const BlockPos& pos, int type, int dimension,
                                                                   CompoundTag* tag)
{
    _init();
    shouldDelay = true;
    this->pos = pos;
    this->type = type;
    this->tag = tag;
    this->dimension = dimension;
}

ClientboundBlockEntityDataPacket::~ClientboundBlockEntityDataPacket()
{
    delete tag;
    tag = NULL;
}

void ClientboundBlockEntityDataPacket::_init()
{
    pos = BlockPos();
    type = 1;
    tag = NULL;
    dimension = 0;
}

int ClientboundBlockEntityDataPacket::getPacketId()
{
    return 132;
}

void ClientboundBlockEntityDataPacket::read(DataInputStream* input)
{
    pos = input->readBlockPos();
    type = input->readByte();
    dimension = input->readByte() - 127;
    tag = input->readCompressedNbt();
}

void ClientboundBlockEntityDataPacket::write(DataOutputStream* output)
{
    output->writeBlockPos(pos);
    output->writeByte(type);
    output->writeByte(dimension + 127);
    output->writeCompressedNbt(tag);
}

void ClientboundBlockEntityDataPacket::handle(PacketListener* listener)
{
    listener->handleBlockEntityData(shared_from_this());
}

int ClientboundBlockEntityDataPacket::getEstimatedSize()
{
    return 25;
}

BlockPos ClientboundBlockEntityDataPacket::getPos()
{
    return pos;
}

int ClientboundBlockEntityDataPacket::getType()
{
    return type;
}

CompoundTag* ClientboundBlockEntityDataPacket::getTag()
{
    return tag;
}

int ClientboundBlockEntityDataPacket::getDimension() const
{
    return dimension;
}
