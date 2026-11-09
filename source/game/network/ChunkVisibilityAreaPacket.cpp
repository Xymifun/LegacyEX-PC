#include "game/network/ChunkVisibilityAreaPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ChunkVisibilityAreaPacket::ChunkVisibilityAreaPacket()
    : minX(0),
      maxX(0),
      minZ(0),
      maxZ(0),
      visible(true),
      loaded(false),
      outside(false)
{
}

ChunkVisibilityAreaPacket::ChunkVisibilityAreaPacket(int minX, int maxX, int minZ, int maxZ, bool visible, bool loaded,
                                                     bool outside)
    : minX(minX),
      maxX(maxX),
      minZ(minZ),
      maxZ(maxZ),
      visible(visible),
      loaded(loaded),
      outside(outside)
{
}

void ChunkVisibilityAreaPacket::read(DataInputStream* input)
{
    unsigned int first = static_cast<unsigned int>(input->readInt());
    unsigned int second = static_cast<unsigned int>(input->readInt());
    minX = static_cast<int>(first << 2) >> 17;
    minZ = static_cast<int>(first << 17) >> 17;
    visible = ((first >> 30) & 1) != 0;
    loaded = (first >> 31) & 1;
    maxX = static_cast<int>(second << 2) >> 17;
    maxZ = static_cast<int>(second << 17) >> 17;
    outside = ((second >> 30) & 1) != 0;
}

void ChunkVisibilityAreaPacket::write(DataOutputStream* output)
{
    unsigned int first = ((minX & 0x7fff) << 15) | (minZ & 0x7fff) | (visible << 30) | (loaded << 31);
    unsigned int second = ((maxX & 0x7fff) << 15) | (maxZ & 0x7fff) | (outside << 30);
    output->writeInt(static_cast<int>(first));
    output->writeInt(static_cast<int>(second));
}

void ChunkVisibilityAreaPacket::handle(PacketListener* listener)
{
    listener->handleChunkVisibilityArea(shared_from_this());
}

int ChunkVisibilityAreaPacket::getEstimatedSize()
{
    return 8;
}

int ChunkVisibilityAreaPacket::getPacketId()
{
    return 155;
}
