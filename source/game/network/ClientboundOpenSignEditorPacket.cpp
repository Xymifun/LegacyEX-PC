#include "game/network/ClientboundOpenSignEditorPacket.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundOpenSignEditorPacket::ClientboundOpenSignEditorPacket()
{
}

ClientboundOpenSignEditorPacket::ClientboundOpenSignEditorPacket(const BlockPos& pos)
{
    this->pos = pos;
}

ClientboundOpenSignEditorPacket::~ClientboundOpenSignEditorPacket()
{
}

int ClientboundOpenSignEditorPacket::getPacketId()
{
    return 212;
}

void ClientboundOpenSignEditorPacket::read(DataInputStream* input)
{
    pos = input->readBlockPos();
}

void ClientboundOpenSignEditorPacket::write(DataOutputStream* output)
{
    output->writeBlockPos(pos);
}

void ClientboundOpenSignEditorPacket::handle(PacketListener* listener)
{
    listener->handleOpenSignEditor(shared_from_this());
}

BlockPos ClientboundOpenSignEditorPacket::getPos()
{
    return pos;
}
