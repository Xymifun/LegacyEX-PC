#include "game/network/ClientboundChangeDifficultyPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Difficulty.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundChangeDifficultyPacket::ClientboundChangeDifficultyPacket()
{
}

ClientboundChangeDifficultyPacket::ClientboundChangeDifficultyPacket(const Difficulty* difficulty, bool locked)
{
    this->difficulty = difficulty;
    this->locked = locked;
}

ClientboundChangeDifficultyPacket::~ClientboundChangeDifficultyPacket()
{
}

int ClientboundChangeDifficultyPacket::getPacketId()
{
    return 253;
}

void ClientboundChangeDifficultyPacket::read(DataInputStream* input)
{
    difficulty = Difficulty::byId(input->readUnsignedByte());
}

void ClientboundChangeDifficultyPacket::write(DataOutputStream* output)
{
    output->writeByte(difficulty->getId());
}

void ClientboundChangeDifficultyPacket::handle(PacketListener* listener)
{
    listener->handleChangeDifficulty(shared_from_this());
}

bool ClientboundChangeDifficultyPacket::isLocked()
{
    return locked;
}

const Difficulty* ClientboundChangeDifficultyPacket::getDifficulty()
{
    return difficulty;
}
