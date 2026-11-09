#include "game/network/XZPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

const int XZPacket::STRONGHOLD = 0;

XZPacket::XZPacket() : mMode(0), mX(0), mZ(0)
{
}

XZPacket::XZPacket(char mode, int x, int z) : mMode(mode), mX(x), mZ(z)
{
}

XZPacket::~XZPacket()
{
}

void XZPacket::handle(PacketListener* listener)
{
    listener->handleXZ(shared_from_this());
}

void XZPacket::read(DataInputStream* input)
{
    mMode = static_cast<char>(input->readChar());
    mX = input->readInt();
    mZ = input->readInt();
}

void XZPacket::write(DataOutputStream* output)
{
    output->writeChar(static_cast<wchar_t>(mMode));
    output->writeInt(mX);
    output->writeInt(mZ);
}

int XZPacket::getEstimatedSize()
{
    return 10;
}

int XZPacket::getPacketId()
{
    return 166;
}
