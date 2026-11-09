#include "game/network/KickPlayerPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

KickPlayerPacket::KickPlayerPacket() : reason(0)
{
}

KickPlayerPacket::KickPlayerPacket(unsigned char reason) : reason(reason)
{
}

void KickPlayerPacket::handle(PacketListener* listener)
{
    listener->handleKickPlayer(shared_from_this());
}

void KickPlayerPacket::read(DataInputStream* input)
{
    reason = input->readByte();
}

void KickPlayerPacket::write(DataOutputStream* output)
{
    output->writeByte(reason);
}

int KickPlayerPacket::getEstimatedSize()
{
    return 1;
}

int KickPlayerPacket::getPacketId()
{
    return 159;
}
