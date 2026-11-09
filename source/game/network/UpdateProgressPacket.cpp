#include "game/network/UpdateProgressPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

UpdateProgressPacket::UpdateProgressPacket() : mProgress(0)
{
}

UpdateProgressPacket::UpdateProgressPacket(int progress) : mProgress(progress)
{
}

void UpdateProgressPacket::read(DataInputStream* input)
{
    mProgress = input->readByte();
}

void UpdateProgressPacket::write(DataOutputStream* output)
{
    output->writeByte(static_cast<unsigned char>(mProgress));
}

void UpdateProgressPacket::handle(PacketListener* listener)
{
    listener->handleUpdateProgress(shared_from_this());
}

int UpdateProgressPacket::getEstimatedSize()
{
    return 1;
}

int UpdateProgressPacket::getPacketId()
{
    return 156;
}
