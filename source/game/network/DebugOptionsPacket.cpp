#include "game/network/DebugOptionsPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

DebugOptionsPacket::DebugOptionsPacket() : options(0)
{
}

DebugOptionsPacket::DebugOptionsPacket(unsigned int options) : options(options)
{
}

DebugOptionsPacket::~DebugOptionsPacket()
{
}

int DebugOptionsPacket::getPacketId()
{
    return 152;
}

void DebugOptionsPacket::read(DataInputStream* input)
{
    options = input->readInt();
}

void DebugOptionsPacket::write(DataOutputStream* output)
{
    output->writeInt(options);
}

void DebugOptionsPacket::handle(PacketListener* listener)
{
    listener->handleDebugOptions(shared_from_this());
}

int DebugOptionsPacket::getEstimatedSize()
{
    return 4;
}
