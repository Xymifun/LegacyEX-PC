#include "game/network/ClientboundPlayerPositionPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

const ClientboundPlayerPositionPacket::RelativeArgument ClientboundPlayerPositionPacket::RelativeArgument::X(0);
const ClientboundPlayerPositionPacket::RelativeArgument ClientboundPlayerPositionPacket::RelativeArgument::Y(1);
const ClientboundPlayerPositionPacket::RelativeArgument ClientboundPlayerPositionPacket::RelativeArgument::Z(2);
const ClientboundPlayerPositionPacket::RelativeArgument ClientboundPlayerPositionPacket::RelativeArgument::Y_ROT(3);
const ClientboundPlayerPositionPacket::RelativeArgument ClientboundPlayerPositionPacket::RelativeArgument::X_ROT(4);

ClientboundPlayerPositionPacket::RelativeArgument::RelativeArgument(int bit)
{
    this->bit = bit;
}

int ClientboundPlayerPositionPacket::RelativeArgument::getMask() const
{
    return 1 << bit;
}

bool ClientboundPlayerPositionPacket::RelativeArgument::isSet(int flags) const
{
    return (flags & getMask()) != 0;
}

boost::unordered_set<const ClientboundPlayerPositionPacket::RelativeArgument*> ClientboundPlayerPositionPacket::
    RelativeArgument::unpack(int flags)
{
    boost::unordered_set<const RelativeArgument*> result;
    const RelativeArgument* arguments[5] = {&X, &Y, &Z, &Y_ROT, &X_ROT};
    for (int i = 0; i < 5; ++i)
    {
        if (arguments[i]->isSet(flags))
            result.insert(arguments[i]);
    }
    return result;
}

int ClientboundPlayerPositionPacket::RelativeArgument::pack(
    const boost::unordered_set<const ClientboundPlayerPositionPacket::RelativeArgument*>& relativeArguments)
{
    int flags = 0;
    boost::unordered_set<const RelativeArgument*>::const_iterator it;
    for (it = relativeArguments.begin(); it != relativeArguments.end(); ++it)
    {
        if (*it != NULL)
            flags |= (*it)->getMask();
    }
    return flags;
}

ClientboundPlayerPositionPacket::ClientboundPlayerPositionPacket()
{
    x = 0.0;
    y = 0.0;
    z = 0.0;
    yRot = 0.0f;
    xRot = 0.0f;
    hasRotation = false;
    id = 0;
}

ClientboundPlayerPositionPacket::ClientboundPlayerPositionPacket(
    double x, double y, double z, float yRot, float xRot,
    const boost::unordered_set<const RelativeArgument*>& relativeArguments, int id)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->yRot = yRot;
    this->xRot = xRot;
    this->hasRotation = true;
    this->relativeArguments = relativeArguments;
    this->id = id;
}

ClientboundPlayerPositionPacket::ClientboundPlayerPositionPacket(
    double x, double y, double z, const boost::unordered_set<const RelativeArgument*>& relativeArguments, int id)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->yRot = 0.0f;
    this->xRot = 0.0f;
    this->hasRotation = false;
    this->relativeArguments = relativeArguments;
    this->id = id;
}

ClientboundPlayerPositionPacket::~ClientboundPlayerPositionPacket()
{
}

int ClientboundPlayerPositionPacket::getPacketId()
{
    return 214;
}

void ClientboundPlayerPositionPacket::read(DataInputStream* input)
{
    x = input->readDouble();
    y = input->readDouble();
    z = input->readDouble();
    hasRotation = input->readBoolean();
    if (hasRotation)
    {
        yRot = input->readFloat();
        xRot = input->readFloat();
    }

    id = input->readVarInt();
}

void ClientboundPlayerPositionPacket::write(DataOutputStream* output)
{
    output->writeDouble(x);
    output->writeDouble(y);
    output->writeDouble(z);
    output->writeBoolean(hasRotation);
    if (hasRotation)
    {
        output->writeFloat(yRot);
        output->writeFloat(xRot);
    }

    output->writeVarInt(id);
}

void ClientboundPlayerPositionPacket::handle(PacketListener* listener)
{
    listener->handleMovePlayer(shared_from_this());
}

double ClientboundPlayerPositionPacket::getX()
{
    return x;
}

double ClientboundPlayerPositionPacket::getY()
{
    return y;
}

double ClientboundPlayerPositionPacket::getZ()
{
    return z;
}

float ClientboundPlayerPositionPacket::getYRot()
{
    return yRot;
}

float ClientboundPlayerPositionPacket::getXRot()
{
    return xRot;
}

int ClientboundPlayerPositionPacket::getId()
{
    return id;
}

bool ClientboundPlayerPositionPacket::hasRot()
{
    return hasRotation;
}

const boost::unordered_set<const ClientboundPlayerPositionPacket::RelativeArgument*>& ClientboundPlayerPositionPacket::
    getRelativeArguments()
{
    return relativeArguments;
}
