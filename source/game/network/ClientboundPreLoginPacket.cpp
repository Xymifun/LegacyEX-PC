#include "game/network/ClientboundPreLoginPacket.hpp"
#include "game/core/SharedConstants.hpp"

#include <string.h>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/generation/PlayerUID.hpp"

ClientboundPreLoginPacket::ClientboundPreLoginPacket()
{
    mName = L"";
    mPlayers = 0;
    mPlayerCount = 0;
    mGameMode = 0;
    mSeed = 0;
    memset(mUniqueMapName, 0, 14);
    mProtocolVersion = 0;
    mGameType = 0;
    mMiniGameId = eMiniGameId_Any;
    mDifficulty = 0;
    mHostPlayer = 0;
    mHardcore = false;
}

ClientboundPreLoginPacket::ClientboundPreLoginPacket(std::wstring name, PlayerUID* players, unsigned int playerCount,
                                                     unsigned char gameMode, unsigned int seed, char* uniqueMapName,
                                                     unsigned int hostPlayer, unsigned int maxPlayers,
                                                     unsigned char difficulty, unsigned int gameType, bool hardcore,
                                                     EMiniGameId miniGameId)
{
    mName = name;
    mPlayers = players;
    mPlayerCount = playerCount;
    mGameMode = gameMode;
    mSeed = seed;
    memcpy(mUniqueMapName, uniqueMapName, 14);
    mHostPlayer = hostPlayer;
    mDifficulty = difficulty;
    mGameType = gameType;
    mProtocolVersion = 0;
    mHardcore = hardcore;
    mMiniGameId = miniGameId;
}

ClientboundPreLoginPacket::~ClientboundPreLoginPacket()
{
    if (mPlayers != 0)
        delete[] mPlayers;
}

void ClientboundPreLoginPacket::read(DataInputStream* input)
{
    mProtocolVersion = input->readShort();
    mName = Packet::readUtf(input, 32);
    mGameMode = input->readByte();
    mSeed = input->readInt();
    int playerCount = input->readByte();
    mPlayerCount = playerCount;
    if (playerCount != 0)
    {
        mPlayers = new PlayerUID[playerCount];
        for (unsigned int i = 0; i < mPlayerCount; ++i)
            mPlayers[i] = input->readPlayerUID();
    }
    for (int i = 0; i < 14; ++i)
        mUniqueMapName[i] = input->readByte();
    mHostPlayer = input->readInt();
    mDifficulty = input->readByte();
    mGameType = input->readInt();
    mHardcore = input->readBoolean();
    mMiniGameId = static_cast<EMiniGameId>(input->readInt());
    gApp.SetUniqueMapName(mUniqueMapName);
}

void ClientboundPreLoginPacket::write(DataOutputStream* output)
{
    output->writeShort(SharedConstants::NETWORK_PROTOCOL_VERSION);
    Packet::writeUtf(mName, output);
    output->writeByte(mGameMode);
    output->writeInt(mSeed);
    output->writeByte(static_cast<unsigned char>(mPlayerCount));
    for (unsigned int i = 0; i < mPlayerCount; ++i)
        output->writePlayerUID(mPlayers[i]);
    for (int i = 0; i < 14; ++i)
        output->writeByte(static_cast<unsigned char>(mUniqueMapName[i]));
    output->writeInt(mHostPlayer);
    output->writeByte(mDifficulty);
    output->writeInt(mGameType);
    output->writeBoolean(mHardcore);
    output->writeInt(static_cast<unsigned int>(mMiniGameId));
}

void ClientboundPreLoginPacket::handle(PacketListener* listener)
{
    listener->handlePreLogin(shared_from_this());
}

int ClientboundPreLoginPacket::getEstimatedSize()
{
    return static_cast<int>(mName.length()) + 0x28;
}

boost::shared_ptr<Packet> ClientboundPreLoginPacket::create()
{
    return boost::shared_ptr<Packet>(new ClientboundPreLoginPacket());
}

int ClientboundPreLoginPacket::getPacketId()
{
    return 2;
}
