#include "game/network/DisconnectPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

DisconnectPacket::DisconnectPacket()
{
    this->reason = eDisconnectReason_None;
}

DisconnectPacket::DisconnectPacket(eDisconnectReason reason)
{
    this->reason = reason;
}

int DisconnectPacket::getPacketId()
{
    return 255;
}

void DisconnectPacket::read(DataInputStream* input)
{
    reason = (eDisconnectReason)input->readInt();
}

void DisconnectPacket::write(DataOutputStream* output)
{
    output->writeInt((int)reason);
}

void DisconnectPacket::handle(PacketListener* listener)
{
    listener->handleDisconnect(shared_from_this());
}

int DisconnectPacket::getEstimatedSize()
{
    return 4;
}

bool DisconnectPacket::canBeInvalidated()
{
    return true;
}

bool DisconnectPacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    return true;
}
