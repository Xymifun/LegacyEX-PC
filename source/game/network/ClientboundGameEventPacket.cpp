#include "game/network/ClientboundGameEventPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ClientboundGameEventPacket::ClientboundGameEventPacket() : mEvent(0), mParam(0.0f), mPlayerIndex(-1)
{
}

ClientboundGameEventPacket::ClientboundGameEventPacket(int event, float param)
    : mEvent(event),
      mParam(param),
      mPlayerIndex(-1)
{
}

ClientboundGameEventPacket::ClientboundGameEventPacket(int event, float param, int playerIndex)
    : mEvent(event),
      mParam(param),
      mPlayerIndex(playerIndex)
{
}

int ClientboundGameEventPacket::getPacketId()
{
    return 70;
}

void ClientboundGameEventPacket::read(DataInputStream* input)
{
    mEvent = input->readUnsignedByte();
    mParam = input->readFloat();
    mPlayerIndex = input->readInt();
}

void ClientboundGameEventPacket::write(DataOutputStream* output)
{
    output->writeByte((unsigned char)mEvent);
    output->writeFloat(mParam);
    output->writeInt(mPlayerIndex);
}

void ClientboundGameEventPacket::handle(PacketListener* listener)
{
    listener->handleGameEvent(shared_from_this());
}

int ClientboundGameEventPacket::getEstimatedSize()
{
    return 3;
}

int ClientboundGameEventPacket::getEvent()
{
    return mEvent;
}

float ClientboundGameEventPacket::getParam()
{
    return mParam;
}

int ClientboundGameEventPacket::getPlayerIndex()
{
    return mPlayerIndex;
}
