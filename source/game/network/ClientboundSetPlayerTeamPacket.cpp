#include "game/network/ClientboundSetPlayerTeamPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/generation/PlayerTeam.hpp"

ClientboundSetPlayerTeamPacket::ClientboundSetPlayerTeamPacket() : mName(), mPlayers()
{
    mName = L"";
    mMethod = 0;
    mOptions = 0;
    mNameTagVisibility = 0;
}

ClientboundSetPlayerTeamPacket::ClientboundSetPlayerTeamPacket(PlayerTeam* team, int method) : mName(), mPlayers()
{
    mName = team->getName();
    mMethod = method;
    if (method == 0 || method == 2)
    {
        mOptions = team->packOptions();
        mNameTagVisibility = team->getNameTagVisibility();
        if (method == 0)
            mPlayers.insert(mPlayers.end(), team->getPlayers()->begin(), team->getPlayers()->end());
    }
}

ClientboundSetPlayerTeamPacket::ClientboundSetPlayerTeamPacket(PlayerTeam* team, std::vector<PlayerUID>* players,
                                                               int method)
    : mName(),
      mPlayers()
{
    mMethod = method;
    mName = team->getName();
    mPlayers.insert(mPlayers.end(), players->begin(), players->end());
}

std::wstring ClientboundSetPlayerTeamPacket::getName()
{
    return mName;
}

std::vector<PlayerUID>* ClientboundSetPlayerTeamPacket::getPlayers()
{
    return &mPlayers;
}

int ClientboundSetPlayerTeamPacket::getMethod()
{
    return mMethod;
}

int ClientboundSetPlayerTeamPacket::getOptions()
{
    return mOptions;
}

int ClientboundSetPlayerTeamPacket::getNameTagVisibility()
{
    return mNameTagVisibility;
}

int ClientboundSetPlayerTeamPacket::getPacketId()
{
    return 209;
}

void ClientboundSetPlayerTeamPacket::read(DataInputStream* input)
{
    mName = Packet::readUtf(input, 256);
    mMethod = input->readByte();
    if (mMethod == 0 || mMethod == 2)
    {
        mOptions = input->readInt();
        mNameTagVisibility = input->readByte();
    }
    if (mMethod == 0 || mMethod == 3 || mMethod == 4)
    {
        int count = input->readByte();
        for (int i = 0; i < count; ++i)
            mPlayers.push_back(input->readPlayerUID());
    }
}

void ClientboundSetPlayerTeamPacket::write(DataOutputStream* output)
{
    Packet::writeUtf(mName, output);
    output->writeByte((unsigned char)mMethod);
    if (mMethod == 0 || mMethod == 2)
    {
        output->writeInt(mOptions);
        output->writeByte((unsigned char)mNameTagVisibility);
    }
    if (mMethod == 0 || mMethod == 3 || mMethod == 4)
    {
        output->writeByte((unsigned char)mPlayers.size());
        std::vector<PlayerUID>::const_iterator it;
        for (it = mPlayers.begin(); it != mPlayers.end(); ++it)
            output->writePlayerUID(*it);
    }
}

void ClientboundSetPlayerTeamPacket::handle(PacketListener* listener)
{
    listener->handleClientboundSetPlayerTeamPacket(shared_from_this());
}

int ClientboundSetPlayerTeamPacket::getEstimatedSize()
{
    return (int)mName.size() + 3;
}
