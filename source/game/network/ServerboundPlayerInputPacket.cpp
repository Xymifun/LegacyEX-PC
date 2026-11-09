#include "game/network/ServerboundPlayerInputPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/Packet.hpp"
#include "game/network/PacketListener.hpp"

ServerboundPlayerInputPacket::ServerboundPlayerInputPacket()
{
    xxa = 0.0f;
    zza = 0.0f;
    jumping = false;
    sneaking = false;
}

ServerboundPlayerInputPacket::ServerboundPlayerInputPacket(float xxa, float zza, bool jumping, bool sneaking)
{
    this->xxa = xxa;
    this->zza = zza;
    this->jumping = jumping;
    this->sneaking = sneaking;
}

int ServerboundPlayerInputPacket::getPacketId()
{
    return 27;
}

void ServerboundPlayerInputPacket::read(DataInputStream* input)
{
    xxa = input->readFloat();
    zza = input->readFloat();

    int flags = input->readByte();
    jumping = (flags & 1) > 0;
    sneaking = (flags & 2) > 0;
}

void ServerboundPlayerInputPacket::write(DataOutputStream* output)
{
    output->writeFloat(xxa);
    output->writeFloat(zza);

    unsigned char flags = 0;
    if (jumping)
    {
        flags = 1;
    }
    if (sneaking)
    {
        flags |= 2;
    }

    output->writeByte(flags);
}

void ServerboundPlayerInputPacket::handle(PacketListener* listener)
{
    listener->handlePlayerInput(shared_from_this());
}

int ServerboundPlayerInputPacket::getEstimatedSize()
{
    return 10;
}

float ServerboundPlayerInputPacket::getXxa()
{
    return xxa;
}

float ServerboundPlayerInputPacket::getZza()
{
    return zza;
}

bool ServerboundPlayerInputPacket::isJumping()
{
    return jumping;
}

bool ServerboundPlayerInputPacket::isSneaking()
{
    return sneaking;
}
