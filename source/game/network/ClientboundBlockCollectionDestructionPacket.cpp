#include "game/network/ClientboundBlockCollectionDestructionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundBlockCollectionDestructionPacket::ClientboundBlockCollectionDestructionPacket()
{
    progress = 0;
    sourceId = 0;
}

ClientboundBlockCollectionDestructionPacket::~ClientboundBlockCollectionDestructionPacket()
{
    delete[] positions.data;
}

void ClientboundBlockCollectionDestructionPacket::read(DataInputStream* input)
{
    sourceId = input->readInt();
    progress = input->readUnsignedByte();
    unsigned short count = input->readUnsignedShort();
    if (count > 0)
    {
        positions = arrayWithLength<BlockPos>(count, true);
        for (unsigned short i = 0; i < count; i++)
        {
            positions[i] = input->readBlockPos();
        }
    }
}

void ClientboundBlockCollectionDestructionPacket::write(DataOutputStream* output)
{
    output->writeInt(sourceId);
    output->writeByte(static_cast<unsigned char>(progress));
    output->writeShort(static_cast<unsigned short>(positions.length));
    if (positions.length > 0)
    {
        for (unsigned short i = 0; i < positions.length; i++)
        {
            output->writeBlockPos(positions[i]);
        }
    }
}

void ClientboundBlockCollectionDestructionPacket::handle(PacketListener* listener)
{
    listener->handleBlockCollectionDestruction(shared_from_this());
}

int ClientboundBlockCollectionDestructionPacket::getEstimatedSize()
{
    return positions.length * 8 + 7;
}

int ClientboundBlockCollectionDestructionPacket::getPacketId()
{
    return 235;
}
