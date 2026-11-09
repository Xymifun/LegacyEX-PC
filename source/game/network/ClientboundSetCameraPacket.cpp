#include "game/network/ClientboundSetCameraPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetCameraPacket::ClientboundSetCameraPacket() : cameraId(0)
{
}

ClientboundSetCameraPacket::~ClientboundSetCameraPacket()
{
}

int ClientboundSetCameraPacket::getPacketId()
{
    return 218;
}

void ClientboundSetCameraPacket::read(DataInputStream* input)
{
    cameraId = input->readVarInt();
}

void ClientboundSetCameraPacket::write(DataOutputStream* output)
{
    output->writeVarInt(cameraId);
}

void ClientboundSetCameraPacket::handle(PacketListener* listener)
{
    listener->handleSetCamera(shared_from_this());
}

int ClientboundSetCameraPacket::getCameraId()
{
    return cameraId;
}
