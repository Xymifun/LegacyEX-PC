#include "game/network/ClientboundSetBorderPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/WorldBorder.hpp"

ClientboundSetBorderPacket::ClientboundSetBorderPacket()
{
}

ClientboundSetBorderPacket::ClientboundSetBorderPacket(WorldBorder* border, Type packetType) : type(packetType)
{
    centerX = border->getCenterX();
    centerZ = border->getCenterZ();
    oldSize = border->getSize();
    newSize = border->getLerpTarget();
    lerpTime = border->getLerpRemainingTime();
    absoluteMaxSize = border->getAbsoluteMaxSize();
    warningBlocks = border->getWarningBlocks();
    warningTime = border->getWarningTime();
}

ClientboundSetBorderPacket::~ClientboundSetBorderPacket()
{
}

void ClientboundSetBorderPacket::handle(PacketListener* listener)
{
    listener->handleSetBorder(shared_from_this());
}

void ClientboundSetBorderPacket::read(DataInputStream* input)
{
    type = static_cast<Type>(input->readInt());
    switch (type)
    {
    case SET_SIZE:
        newSize = input->readDouble();
        break;
    case LERP_SIZE:
        oldSize = input->readDouble();
        newSize = input->readDouble();
        lerpTime = input->readVarLong();
        break;
    case SET_CENTER:
        centerX = input->readDouble();
        centerZ = input->readDouble();
        break;
    case SET_WARNING_BLOCKS:
        warningBlocks = input->readVarInt();
        break;
    case SET_WARNING_TIME:
        warningTime = input->readVarInt();
        break;
    case INITIALIZE:
        centerX = input->readDouble();
        centerZ = input->readDouble();
        oldSize = input->readDouble();
        newSize = input->readDouble();
        lerpTime = input->readVarLong();
        absoluteMaxSize = input->readVarInt();
        warningBlocks = input->readVarInt();
        warningTime = input->readVarInt();
        break;
    }
}

void ClientboundSetBorderPacket::write(DataOutputStream* output)
{
    output->writeInt(static_cast<int>(type));
    switch (type)
    {
    case SET_SIZE:
        output->writeDouble(newSize);
        break;
    case LERP_SIZE:
        output->writeDouble(oldSize);
        output->writeDouble(newSize);
        output->writeVarLong(lerpTime);
        break;
    case SET_CENTER:
        output->writeDouble(centerX);
        output->writeDouble(centerZ);
        break;
    case SET_WARNING_TIME:
        output->writeVarInt(warningTime);
        break;
    case SET_WARNING_BLOCKS:
        output->writeVarInt(warningBlocks);
        break;
    case INITIALIZE:
        output->writeDouble(centerX);
        output->writeDouble(centerZ);
        output->writeDouble(oldSize);
        output->writeDouble(newSize);
        output->writeVarLong(lerpTime);
        output->writeVarInt(absoluteMaxSize);
        output->writeVarInt(warningBlocks);
        output->writeVarInt(warningTime);
        break;
    }
}

void ClientboundSetBorderPacket::applyChanges(WorldBorder* border)
{
    switch (type)
    {
    case SET_SIZE:
        border->setSize(newSize);
        break;
    case LERP_SIZE:
        border->lerpSizeBetween(oldSize, newSize, lerpTime);
        break;
    case SET_CENTER:
        border->setCenter(centerX, centerZ);
        break;
    case INITIALIZE:
        border->setCenter(centerX, centerZ);
        if (lerpTime > 0)
        {
            border->lerpSizeBetween(oldSize, newSize, lerpTime);
        }
        else
        {
            border->setSize(newSize);
        }
        border->setAbsoluteMaxSize(absoluteMaxSize);
        border->setWarningBlocks(warningBlocks);
        border->setWarningTime(warningTime);
        break;
    case SET_WARNING_TIME:
        border->setWarningTime(warningTime);
        break;
    case SET_WARNING_BLOCKS:
        border->setWarningBlocks(warningBlocks);
        break;
    }
}

int ClientboundSetBorderPacket::getPacketId()
{
    return 217;
}

boost::shared_ptr<Packet> ClientboundSetBorderPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundSetBorderPacket());
}
