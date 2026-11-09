#include "game/network/PlayerReadyPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

PlayerReadyPacket::PlayerReadyPacket() : mPlayerId(0), mReady(false)
{
}

PlayerReadyPacket::PlayerReadyPacket(unsigned int playerId, bool ready) : mPlayerId(playerId), mReady(ready)
{
}

PlayerReadyPacket::~PlayerReadyPacket()
{
}

int PlayerReadyPacket::getPacketId()
{
    return 170;
}

void PlayerReadyPacket::read(DataInputStream* input)
{
    mPlayerId = input->readInt();
    mReady = input->readBoolean();
}

void PlayerReadyPacket::write(DataOutputStream* output)
{
    output->writeInt(mPlayerId);
    output->writeBoolean(mReady);
}

void PlayerReadyPacket::handle(PacketListener* listener)
{
    listener->handlePlayerReady(shared_from_this());
}

int PlayerReadyPacket::getEstimatedSize()
{
    return 5;
}
