#include "game/network/ServerboundClientCommandPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerboundClientCommandPacket::ServerboundClientCommandPacket() : action(PERFORM_RESPAWN)
{
}

ServerboundClientCommandPacket::ServerboundClientCommandPacket(Action action) : action(action)
{
}

int ServerboundClientCommandPacket::getPacketId()
{
    return 205;
}

void ServerboundClientCommandPacket::read(DataInputStream* input)
{
    action = (Action)input->readInt();
}

void ServerboundClientCommandPacket::write(DataOutputStream* output)
{
    output->writeInt(action);
}

void ServerboundClientCommandPacket::handle(PacketListener* listener)
{
    listener->handleClientCommand(boost::dynamic_pointer_cast<ServerboundClientCommandPacket>(shared_from_this()));
}

int ServerboundClientCommandPacket::getEstimatedSize()
{
    return 1;
}

ServerboundClientCommandPacket::Action ServerboundClientCommandPacket::getAction()
{
    return action;
}
