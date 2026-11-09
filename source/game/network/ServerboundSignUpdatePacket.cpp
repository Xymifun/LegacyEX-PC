#include "game/network/ServerboundSignUpdatePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/block/BlockPos.hpp"

ServerboundSignUpdatePacket::ServerboundSignUpdatePacket()
{
    lines = NULL;
    createdByRestrictedPlayer = false;
}

ServerboundSignUpdatePacket::ServerboundSignUpdatePacket(const BlockPos& pos, std::wstring* lines,
                                                         bool createdByRestrictedPlayer)
{
    this->pos = pos;
    this->createdByRestrictedPlayer = createdByRestrictedPlayer;
    this->lines = new std::wstring[4];

    for (int i = 0; i < 4; ++i)
    {
        this->lines[i] = lines[i];
    }
}

ServerboundSignUpdatePacket::~ServerboundSignUpdatePacket()
{
    delete[] lines;
}

int ServerboundSignUpdatePacket::getPacketId()
{
    return 230;
}

void ServerboundSignUpdatePacket::read(DataInputStream* input)
{
    pos = input->readBlockPos();
    createdByRestrictedPlayer = input->readBoolean();
    lines = new std::wstring[4];

    for (int i = 0; i < 4; ++i)
    {
        lines[i] = Packet::readUtf(input, 32767);
    }
}

void ServerboundSignUpdatePacket::write(DataOutputStream* output)
{
    output->writeBlockPos(pos);
    output->writeBoolean(createdByRestrictedPlayer);

    for (int i = 0; i < 4; ++i)
    {
        Packet::writeUtf(lines[i], output);
    }
}

void ServerboundSignUpdatePacket::handle(PacketListener* listener)
{
    listener->handleSignUpdate(shared_from_this());
}

BlockPos ServerboundSignUpdatePacket::getPos()
{
    return pos;
}

std::wstring* ServerboundSignUpdatePacket::getLines()
{
    return lines;
}

bool ServerboundSignUpdatePacket::isCreatedByRestrictedPlayer()
{
    return createdByRestrictedPlayer;
}
