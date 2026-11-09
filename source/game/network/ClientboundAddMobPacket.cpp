#include "game/network/ClientboundAddMobPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"


ClientboundAddMobPacket::ClientboundAddMobPacket()
{
    id = -1;
    uuid = L"";
    type = 0;
    x = 0;
    y = 0;
    z = 0;
    yRot = 0;
    xRot = 0;
    entityData = boost::shared_ptr<SynchedEntityData>();
    unpackedData = NULL;
    additionalContructionDataTag = NULL;
}

ClientboundAddMobPacket::ClientboundAddMobPacket(boost::shared_ptr<LivingEntity> entity, int yRot, int xRot, int x,
                                                 int y, int z, int yHeadRot)
{
    init(entity, yRot, xRot, x, y, z, yHeadRot, NULL);
}

ClientboundAddMobPacket::ClientboundAddMobPacket(boost::shared_ptr<LivingEntity> entity, int yRot, int xRot, int x,
                                                 int y, int z, int yHeadRot, CompoundTag* additionalContructionDataTag)
{
    init(entity, yRot, xRot, x, y, z, yHeadRot, additionalContructionDataTag);
}

ClientboundAddMobPacket::~ClientboundAddMobPacket()
{
    if (unpackedData != NULL)
    {
        for (unsigned int i = 0; i < unpackedData->size(); ++i)
        {
            delete unpackedData->at(i);
        }

        delete unpackedData;
    }

    delete additionalContructionDataTag;
}

void ClientboundAddMobPacket::init(boost::shared_ptr<LivingEntity> entity, int yRot, int xRot, int x, int y, int z,
                                   int yHeadRot, CompoundTag* additionalContructionDataTag)
{
    id = entity->getId();
    uuid = entity->getUUID();
    type = EntityIO::sRegistry.getId(entity->GetType());

    this->yRot = yRot;
    this->xRot = xRot;
    this->x = x;
    this->y = y;
    this->z = z;
    this->yHeadRot = yHeadRot;

    double xd = entity->mXd;
    double yd = entity->mYd;
    double zd = entity->mZd;

    if (xd < -3.9)
    {
        xd = -3.9;
    }

    if (yd < -3.9)
    {
        yd = -3.9;
    }

    if (zd < -3.9)
    {
        zd = -3.9;
    }

    if (xd > 3.9)
    {
        xd = 3.9;
    }

    if (yd > 3.9)
    {
        yd = 3.9;
    }

    if (zd > 3.9)
    {
        zd = 3.9;
    }

    this->xd = (int)(xd * 8000.0);
    this->yd = (int)(yd * 8000.0);
    this->zd = (int)(zd * 8000.0);

    std::vector<boost::shared_ptr<Entity> >* subEntities = entity->getSubEntities();

    if (subEntities != NULL)
    {
        subIds.reserve(subEntities->size());

        for (std::vector<boost::shared_ptr<Entity> >::iterator it = subEntities->begin(); it != subEntities->end();
             ++it)
        {
            subIds.push_back((*it)->getId());
        }
    }

    entityData = entity->getEntityData();
    unpackedData = NULL;
    this->additionalContructionDataTag = additionalContructionDataTag;
}

int ClientboundAddMobPacket::getPacketId()
{
    return 24;
}

void ClientboundAddMobPacket::read(DataInputStream* input)
{
    id = input->readShort();
    uuid = input->readUTF();
    type = input->readVarInt();
#ifdef MINECRAFT_PC
    x = input->readInt();
    y = input->readInt();
    z = input->readInt();
#else
    x = input->readShort();
    y = input->readShort();
    z = input->readShort();
#endif
    yRot = input->readByte();
    xRot = input->readByte();
    yHeadRot = input->readByte();
    xd = input->readShort();
    yd = input->readShort();
    zd = input->readShort();

    int subIdCount = input->readByte();

    if (subIdCount != -1)
    {
        subIds.reserve(subIdCount);

        for (int i = 0; i < subIdCount; ++i)
        {
            int subId = input->readInt();

            subIds.push_back(subId);
        }
    }

    MemSect(1);

    unpackedData = SynchedEntityData::unpack(input);

    MemSect(0);

    additionalContructionDataTag = input->readCompressedNbt();
}

void ClientboundAddMobPacket::write(DataOutputStream* output)
{
    output->writeShort(id);
    output->writeUTF(uuid);
    output->writeVarInt(type);
#ifdef MINECRAFT_PC
    output->writeInt(x);
    output->writeInt(y);
    output->writeInt(z);
#else
    output->writeShort(x);
    output->writeShort(y);
    output->writeShort(z);
#endif
    output->writeByte(yRot);
    output->writeByte(xRot);
    output->writeByte(yHeadRot);
    output->writeShort(xd);
    output->writeShort(yd);
    output->writeShort(zd);
    output->writeByte(subIds.size());

    for (std::vector<int>::iterator it = subIds.begin(); it != subIds.end(); it++)
    {
        output->writeInt(*it);
    }

    entityData->packAll(output);
    output->writeCompressedNbt(additionalContructionDataTag);
}

void ClientboundAddMobPacket::handle(PacketListener* listener)
{
    listener->handleAddMob(shared_from_this());
}

int ClientboundAddMobPacket::getEstimatedSize()
{
    int size = subIds.size() * 4 + 11;

    if (entityData)
    {
        size += entityData->getItemCount() * 8;
    }

    return size + DataOutputStream::GetCompressedNbtSize(additionalContructionDataTag);
}

std::vector<DataItem*>* ClientboundAddMobPacket::getUnpackedData()
{
    return unpackedData;
}

int ClientboundAddMobPacket::getId()
{
    return id;
}

std::wstring ClientboundAddMobPacket::getUUID()
{
    return uuid;
}

int ClientboundAddMobPacket::getType()
{
    return type;
}

int ClientboundAddMobPacket::getX()
{
    return x;
}

int ClientboundAddMobPacket::getY()
{
    return y;
}

int ClientboundAddMobPacket::getZ()
{
    return z;
}

int ClientboundAddMobPacket::getXd()
{
    return xd;
}

int ClientboundAddMobPacket::getYd()
{
    return yd;
}

int ClientboundAddMobPacket::getZd()
{
    return zd;
}

unsigned char ClientboundAddMobPacket::getyRot()
{
    return yRot;
}

unsigned char ClientboundAddMobPacket::getxRot()
{
    return xRot;
}

unsigned char ClientboundAddMobPacket::getyHeadRot()
{
    return yHeadRot;
}

std::vector<int> ClientboundAddMobPacket::getSubIds()
{
    return subIds;
}

CompoundTag* ClientboundAddMobPacket::getAdditionalContructionDataTag()
{
    return additionalContructionDataTag;
}
