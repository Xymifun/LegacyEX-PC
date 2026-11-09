#include "game/network/ClientboundSetEntityDataPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetEntityDataPacket::ClientboundSetEntityDataPacket()
{
    unpackedData = NULL;
    id = -1;
}

ClientboundSetEntityDataPacket::ClientboundSetEntityDataPacket(int id, int entityType,
                                                               boost::shared_ptr<SynchedEntityData> entityData,
                                                               bool sendAll)
{
    this->id = id;
    this->entityType = entityType;
    this->entityData = entityData;

    if (sendAll)
    {
        unpackedData = entityData->getAll();
        entityData->clearDirty();
    }
    else
    {
        unpackedData = entityData->packDirty();
    }
}

ClientboundSetEntityDataPacket::~ClientboundSetEntityDataPacket()
{
    if (unpackedData != NULL)
    {
        for (unsigned int i = 0; i < unpackedData->size(); i++)
        {
            delete unpackedData->at(i);
        }
    }

    delete unpackedData;
}

int ClientboundSetEntityDataPacket::getPacketId()
{
    return 40;
}

void ClientboundSetEntityDataPacket::read(DataInputStream* input)
{
    id = input->readVarInt();
    entityType = input->readInt();
    unpackedData = SynchedEntityData::unpack(input);
}

void ClientboundSetEntityDataPacket::write(DataOutputStream* output)
{
    output->writeVarInt(id);
    output->writeInt(entityType);
    SynchedEntityData::pack(unpackedData, output);
}

void ClientboundSetEntityDataPacket::handle(PacketListener* listener)
{
    listener->handleSetEntityData(shared_from_this());
}

int ClientboundSetEntityDataPacket::getEstimatedSize()
{
    return 5;
}

int ClientboundSetEntityDataPacket::getId()
{
    return id;
}

int ClientboundSetEntityDataPacket::getEntityType()
{
    return entityType;
}

std::vector<DataItem*>* ClientboundSetEntityDataPacket::getUnpackedData()
{
    return unpackedData;
}
