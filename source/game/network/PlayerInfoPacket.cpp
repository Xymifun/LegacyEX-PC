#include "game/network/PlayerInfoPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/GameType.hpp"
#include "game/network/PacketListener.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"

PlayerInfoPacket::PlayerInfoPacket() : mPlayerId(0), mPlayerIndex(-1), mPrivileges(0), mEntityId(-1), mGameType(0)
{
}

PlayerInfoPacket::PlayerInfoPacket(unsigned char playerId, short playerIndex, unsigned int privileges,
                                   unsigned char gameType)
    : mPlayerId(playerId),
      mPlayerIndex(playerIndex),
      mPrivileges(privileges),
      mEntityId(-1),
      mGameType(gameType)
{
}

PlayerInfoPacket::PlayerInfoPacket(boost::shared_ptr<ServerPlayer> player)
{
    mPlayerId = 0;
    if (player->mConnection != NULL && player->mConnection->getNetworkPlayer() != NULL)
        mPlayerId = player->mConnection->getNetworkPlayer()->GetSmallId();
    mPlayerIndex = static_cast<short>(player->mPlayerIndex);
    mPrivileges = player->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All);
    mEntityId = player->getId();
    mGameType = static_cast<unsigned char>(player->GetGameType()->getId());
}

void PlayerInfoPacket::read(DataInputStream* input)
{
    mPlayerId = input->readByte();
    mPlayerIndex = input->readShort();
    mPrivileges = static_cast<unsigned int>(input->readInt());
    mEntityId = input->readInt();
    mGameType = input->readByte();
}

void PlayerInfoPacket::write(DataOutputStream* output)
{
    output->writeByte(static_cast<unsigned char>(mPlayerId));
    output->writeShort(mPlayerIndex);
    output->writeInt(static_cast<int>(mPrivileges));
    output->writeInt(mEntityId);
    output->writeByte(mGameType);
}

void PlayerInfoPacket::handle(PacketListener* listener)
{
    listener->handlePlayerInfo(shared_from_this());
}

int PlayerInfoPacket::getEstimatedSize()
{
    return 13;
}

int PlayerInfoPacket::getPacketId()
{
    return 201;
}
