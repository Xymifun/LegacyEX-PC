#include "game/network/ChunkVisibilityPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ChunkVisibilityPacket::ChunkVisibilityPacket()
{
    shouldDelay = false;
    x = 0;
    z = 0;
    visible = false;
    loaded = false;
}

ChunkVisibilityPacket::ChunkVisibilityPacket(int x, int z, bool visible, bool loaded)
{
    shouldDelay = false;
    this->x = x;
    this->z = z;
    this->visible = visible;
    this->loaded = loaded;
}

void ChunkVisibilityPacket::read(DataInputStream* input)
{
    int packed = input->readInt();
    x = (packed << 2) >> 17;
    z = (packed << 17) >> 17;
    visible = ((packed >> 30) & 1) != 0;
    loaded = ((packed >> 31) & 1) != 0;
}

void ChunkVisibilityPacket::write(DataOutputStream* output)
{
    unsigned int packed = ((static_cast<unsigned int>(x) & 0x7fff) << 15) | (static_cast<unsigned int>(z) & 0x7fff)
                          | (static_cast<unsigned int>(visible) << 30) | (static_cast<unsigned int>(loaded) << 31);
    output->writeInt(static_cast<int>(packed));
}

void ChunkVisibilityPacket::handle(PacketListener* listener)
{
    listener->handleChunkVisibility(shared_from_this());
}

int ChunkVisibilityPacket::getEstimatedSize()
{
    return 4;
}

int ChunkVisibilityPacket::getPacketId()
{
    return 50;
}
