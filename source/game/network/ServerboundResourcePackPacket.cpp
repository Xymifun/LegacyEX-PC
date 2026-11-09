#include "game/network/ServerboundResourcePackPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerboundResourcePackPacket::ServerboundResourcePackPacket()
{
}

ServerboundResourcePackPacket::ServerboundResourcePackPacket(Action action) : mAction(action)
{
}

ServerboundResourcePackPacket::~ServerboundResourcePackPacket()
{
}

void ServerboundResourcePackPacket::read(DataInputStream* input)
{
    mAction = static_cast<Action>(input->readInt());
}

void ServerboundResourcePackPacket::write(DataOutputStream* output)
{
    output->writeInt(static_cast<int>(mAction));
}

void ServerboundResourcePackPacket::handle(PacketListener* listener)
{
    listener->handleResourcePackResponse(shared_from_this());
}

int ServerboundResourcePackPacket::getPacketId()
{
    return 228;
}

ServerboundResourcePackPacket::Action ServerboundResourcePackPacket::getAction()
{
    return mAction;
}
