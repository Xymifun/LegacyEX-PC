#include "game/network/ServerSettingsChangedPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"

ServerSettingsChangedPacket::ServerSettingsChangedPacket() : mSetting(0), mValue(1)
{
}

ServerSettingsChangedPacket::ServerSettingsChangedPacket(char setting, unsigned int value)
    : mSetting(setting),
      mValue(value)
{
}

ServerSettingsChangedPacket::~ServerSettingsChangedPacket()
{
}

int ServerSettingsChangedPacket::getPacketId()
{
    return 153;
}

void ServerSettingsChangedPacket::read(DataInputStream* input)
{
    mSetting = (char)input->readByte();
    mValue = (unsigned int)input->readInt();
}

void ServerSettingsChangedPacket::write(DataOutputStream* output)
{
    output->writeByte((unsigned char)mSetting);
    output->writeInt((int)mValue);
}

void ServerSettingsChangedPacket::handle(PacketListener* listener)
{
    listener->handleServerSettingsChanged(shared_from_this());
}

int ServerSettingsChangedPacket::getEstimatedSize()
{
    return 2;
}
