#include "game/network/ClientboundSetEntityMotionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetEntityMotionPacket::ClientboundSetEntityMotionPacket()
{
    _init(0, 0.0, 0.0, 0.0);
}

ClientboundSetEntityMotionPacket::ClientboundSetEntityMotionPacket(boost::shared_ptr<Entity> entity)
{
    _init(entity->getId(), entity->mXd, entity->mYd, entity->mZd);
}

ClientboundSetEntityMotionPacket::ClientboundSetEntityMotionPacket(int id, double xa, double ya, double za)
{
    _init(id, xa, ya, za);
}

void ClientboundSetEntityMotionPacket::_init(int id, double xa, double ya, double za)
{
    this->id = id;

    if (xa < -3.9)
    {
        xa = -3.9;
    }

    if (ya < -3.9)
    {
        ya = -3.9;
    }

    if (za < -3.9)
    {
        za = -3.9;
    }

    if (xa > 3.9)
    {
        xa = 3.9;
    }

    if (ya > 3.9)
    {
        ya = 3.9;
    }

    if (za > 3.9)
    {
        za = 3.9;
    }

    int scaledX = (int)(xa * 8000.0);
    int scaledY = (int)(ya * 8000.0);
    int scaledZ = (int)(za * 8000.0);
    this->xa = scaledX;
    this->ya = scaledY;
    this->za = scaledZ;

    if (scaledX >= -2048 && scaledY >= -2048 && scaledZ >= -2048 && scaledX < 2048 && scaledY < 2048 && scaledZ < 2048)
    {
        compact = true;
    }
    else
    {
        compact = false;
    }
}

int ClientboundSetEntityMotionPacket::getPacketId()
{
    return 28;
}

void ClientboundSetEntityMotionPacket::read(DataInputStream* input)
{
#ifdef MINECRAFT_PC
    id = input->readInt();
    const bool compactData = input->readBoolean();
    if (compactData)
#else
    short header = input->readShort();

    id = header & 0x7FF;

    if ((header & 0x800) != 0)
#endif
    {
        xa = input->readByte();
        ya = input->readByte();
        int rawZ = input->readByte();
        xa = (xa << 24) >> 20;
        ya = (ya << 24) >> 20;
        za = (rawZ << 24) >> 20;
        compact = true;
    }
    else
    {
        xa = input->readShort();
        ya = input->readShort();
        za = input->readShort();
        compact = false;
    }
}

void ClientboundSetEntityMotionPacket::write(DataOutputStream* output)
{
#ifdef MINECRAFT_PC
    output->writeInt(id);
    output->writeBoolean(compact);
#endif
    if (compact)
    {
#ifndef MINECRAFT_PC
        output->writeShort(id | 0x800);
#endif
        output->writeByte(xa / 16);
        output->writeByte(ya / 16);
        output->writeByte(za / 16);
    }
    else
    {
#ifndef MINECRAFT_PC
        output->writeShort(id);
#endif
        output->writeShort(xa);
        output->writeShort(ya);
        output->writeShort(za);
    }
}

void ClientboundSetEntityMotionPacket::handle(PacketListener* listener)
{
    listener->handleSetEntityMotion(shared_from_this());
}

int ClientboundSetEntityMotionPacket::getEstimatedSize()
{
    if (compact)
    {
        return 5;
    }

    return 8;
}

bool ClientboundSetEntityMotionPacket::canBeInvalidated()
{
    return true;
}

bool ClientboundSetEntityMotionPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    boost::shared_ptr<ClientboundSetEntityMotionPacket> other =
        boost::dynamic_pointer_cast<ClientboundSetEntityMotionPacket>(packet);

    return other->id == id;
}

int ClientboundSetEntityMotionPacket::getId()
{
    return id;
}

int ClientboundSetEntityMotionPacket::getXa()
{
    return xa;
}

int ClientboundSetEntityMotionPacket::getYa()
{
    return ya;
}

int ClientboundSetEntityMotionPacket::getZa()
{
    return za;
}
