#include "game/network/ClientboundAddPaintingPacket.hpp"

#include "game/core/Painting.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundAddPaintingPacket::ClientboundAddPaintingPacket()
{
    id = -1;
    pos = BlockPos();
    motive = L"";
}

ClientboundAddPaintingPacket::ClientboundAddPaintingPacket(boost::shared_ptr<Painting> painting)
{
    id = painting->getId();
    pos = painting->getPos();
    direction = painting->mDirection;
    motive = painting->motive->name;
}

int ClientboundAddPaintingPacket::getPacketId()
{
    return 25;
}

void ClientboundAddPaintingPacket::read(DataInputStream* input)
{
    id = input->readUnsignedShort();
    motive = readUtf(input, Painting::Motive::MAX_MOTIVE_NAME_LENGTH);
    pos = input->readBlockPos();
    direction = Direction::from2DDataValue(input->readUnsignedByte());
}

void ClientboundAddPaintingPacket::write(DataOutputStream* output)
{
    output->writeUnsignedShort(id);
    writeUtf(motive, output);
    output->writeBlockPos(pos);
    output->writeByte(direction->get2DDataValue());
}

void ClientboundAddPaintingPacket::handle(PacketListener* listener)
{
    listener->handleAddPainting(shared_from_this());
}

int ClientboundAddPaintingPacket::getEstimatedSize()
{
    return motive.length() + 17;
}

int ClientboundAddPaintingPacket::getId()
{
    return id;
}

BlockPos ClientboundAddPaintingPacket::getPos()
{
    return pos;
}

const Direction* ClientboundAddPaintingPacket::getDirection()
{
    return direction;
}

std::wstring ClientboundAddPaintingPacket::getMotive()
{
    return motive;
}
