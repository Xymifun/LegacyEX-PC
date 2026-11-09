#include "game/network/ClientboundPlayerSleepPacket.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/PacketListener.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"

ClientboundPlayerSleepPacket::ClientboundPlayerSleepPacket()
{
    playerId = 0;
    pos = BlockPos();
}

ClientboundPlayerSleepPacket::ClientboundPlayerSleepPacket(boost::shared_ptr<Player> player, const BlockPos& pos)
{
    playerId = player->getId();
    this->pos = pos;
}

ClientboundPlayerSleepPacket::~ClientboundPlayerSleepPacket()
{
}

int ClientboundPlayerSleepPacket::getPacketId()
{
    return 17;
}

void ClientboundPlayerSleepPacket::read(DataInputStream* input)
{
    playerId = input->readVarInt();
    pos = input->readBlockPos();
}

void ClientboundPlayerSleepPacket::write(DataOutputStream* output)
{
    output->writeVarInt(playerId);
    output->writeBlockPos(pos);
}

void ClientboundPlayerSleepPacket::handle(PacketListener* listener)
{
    listener->handleEntityActionAtPosition(shared_from_this());
}

boost::shared_ptr<Player> ClientboundPlayerSleepPacket::getPlayer(Level* level)
{
    return boost::dynamic_pointer_cast<Player>(level->getEntity(playerId));
}

int ClientboundPlayerSleepPacket::getPlayerId()
{
    return playerId;
}

BlockPos ClientboundPlayerSleepPacket::getPos()
{
    return pos;
}
