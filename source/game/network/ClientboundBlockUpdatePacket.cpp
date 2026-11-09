#include "game/network/ClientboundBlockUpdatePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"

ClientboundBlockUpdatePacket::ClientboundBlockUpdatePacket()
{
    shouldDelay = true;
    extraBlock = NULL;
}

ClientboundBlockUpdatePacket::ClientboundBlockUpdatePacket(Level* level, const BlockPos& pos)
{
    shouldDelay = true;
    this->pos = pos;
    block = level->getBlockState(pos);
    extraBlock = level->getExtraBlockState(pos);

    if (level->mDimension->getType() == DimensionType::OVERWORLD)
    {
        levelIdx = 0;
    }
    else if (level->mDimension->getType() == DimensionType::NETHER)
    {
        levelIdx = 1;
    }
    else
    {
        levelIdx = 2;
    }
}

void ClientboundBlockUpdatePacket::read(DataInputStream* input)
{
#ifdef MINECRAFT_PC
    levelIdx = input->readUnsignedByte() & 0xF;
    pos = input->readBlockPos();
#else
    unsigned int packedPos = input->readInt();
    pos = BlockPos(static_cast<int>(packedPos) >> 22, (packedPos >> 14) & 0xFF,
                   static_cast<int>(packedPos << 18) >> 22);
#endif
    short blockId = input->readShort();
    IdMapper<const BlockState*>* registry = Block::blockStateRegistry;
    block = registry->byId(blockId);
    extraBlock = registry->byId(input->readShort());
#ifndef MINECRAFT_PC
    levelIdx = packedPos & 0xF;
#endif
}

void ClientboundBlockUpdatePacket::write(DataOutputStream* output)
{
#ifdef MINECRAFT_PC
    output->writeByte(static_cast<unsigned char>(levelIdx & 0xF));
    output->writeBlockPos(pos);
#else
    unsigned int packedPos =
        (levelIdx & 0xF)
        | (((static_cast<unsigned int>(pos.z) & 0x3FF) << 4)
           | (((static_cast<unsigned int>(pos.y) & 0xFF) << 14) | (static_cast<unsigned int>(pos.x) << 22)));
    output->writeInt(packedPos);
#endif
    IdMapper<const BlockState*>* registry = Block::blockStateRegistry;
    output->writeShort(static_cast<short>(registry->getId(block)));
    output->writeShort(static_cast<short>(registry->getId(extraBlock)));
}

void ClientboundBlockUpdatePacket::handle(PacketListener* listener)
{
    listener->handleBlockUpdate(shared_from_this());
}

int ClientboundBlockUpdatePacket::getEstimatedSize()
{
    return 8;
}

BlockPos ClientboundBlockUpdatePacket::getPos()
{
    return pos;
}

const BlockState* ClientboundBlockUpdatePacket::getBlock()
{
    return block;
}

const BlockState* ClientboundBlockUpdatePacket::getExtraBlock()
{
    return extraBlock;
}

unsigned int ClientboundBlockUpdatePacket::getLevelIdx()
{
    return levelIdx;
}

int ClientboundBlockUpdatePacket::getPacketId()
{
    return 53;
}
