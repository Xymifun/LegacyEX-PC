#include "game/network/ClientboundRemoveEntitiesPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundRemoveEntitiesPacket::ClientboundRemoveEntitiesPacket() : entityIds()
{
}

ClientboundRemoveEntitiesPacket::ClientboundRemoveEntitiesPacket(int entityId) : entityIds()
{
    entityIds = arrayWithLength<int>(1, true);
    entityIds.data[0] = entityId;
}

ClientboundRemoveEntitiesPacket::ClientboundRemoveEntitiesPacket(arrayWithLength<int> entityIds)
{
    this->entityIds = entityIds;
}

ClientboundRemoveEntitiesPacket::~ClientboundRemoveEntitiesPacket()
{
    delete entityIds.data;
    entityIds.data = NULL;
}

void ClientboundRemoveEntitiesPacket::read(DataInputStream* input)
{
    unsigned int count = static_cast<unsigned int>(input->readVarInt());
    entityIds = arrayWithLength<int>(count, true);
    for (unsigned int i = 0; i < entityIds.length; ++i)
        entityIds.data[i] = input->readInt();
}

void ClientboundRemoveEntitiesPacket::write(DataOutputStream* output)
{
    output->writeVarInt(static_cast<int>(entityIds.length));
    for (unsigned int i = 0; i < entityIds.length; ++i)
        output->writeInt(entityIds.data[i]);
}

void ClientboundRemoveEntitiesPacket::handle(PacketListener* listener)
{
    listener->handleRemoveEntity(shared_from_this());
}

int ClientboundRemoveEntitiesPacket::getEstimatedSize()
{
    return static_cast<int>(entityIds.length) * 4 + 1;
}

int ClientboundRemoveEntitiesPacket::getPacketId()
{
    return 29;
}
