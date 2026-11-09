#include "game/network/ClientboundSetExperiencePacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ClientboundSetExperiencePacket::ClientboundSetExperiencePacket()
{
    experienceProgress = 0.0f;
    totalExperience = 0;
    experienceLevel = 0;
}

ClientboundSetExperiencePacket::ClientboundSetExperiencePacket(float experienceProgress, int totalExperience,
                                                               int experienceLevel)
{
    this->experienceProgress = experienceProgress;
    this->totalExperience = totalExperience;
    this->experienceLevel = experienceLevel;
}

int ClientboundSetExperiencePacket::getPacketId()
{
    return 43;
}

void ClientboundSetExperiencePacket::read(DataInputStream* input)
{
    experienceProgress = input->readFloat();
    experienceLevel = input->readVarInt();
    totalExperience = input->readVarInt();
}

void ClientboundSetExperiencePacket::write(DataOutputStream* output)
{
    output->writeFloat(experienceProgress);
    output->writeVarInt(experienceLevel);
    output->writeVarInt(totalExperience);
}

void ClientboundSetExperiencePacket::handle(PacketListener* listener)
{
    listener->handleSetExperience(shared_from_this());
}

int ClientboundSetExperiencePacket::getEstimatedSize()
{
    return 8;
}

bool ClientboundSetExperiencePacket::canBeInvalidated()
{
    return true;
}

bool ClientboundSetExperiencePacket::isInvalidatedBy(boost::shared_ptr<Packet> packet)
{
    return true;
}

float ClientboundSetExperiencePacket::getExperienceProgress()
{
    return experienceProgress;
}

int ClientboundSetExperiencePacket::getTotalExperience()
{
    return totalExperience;
}

int ClientboundSetExperiencePacket::getExperienceLevel()
{
    return experienceLevel;
}
