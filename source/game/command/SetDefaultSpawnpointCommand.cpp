#include "game/command/SetDefaultSpawnpointCommand.hpp"

#include "game/command/CommandSender.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/ClientboundSetSpawnPositionPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/PlayerList.hpp"

EGameCommand SetDefaultSpawnpointCommand::getId()
{
    return eGameCommand_SetDefaultSpawnpoint;
}

int SetDefaultSpawnpointCommand::getPermissionLevel()
{
    return 2;
}

void SetDefaultSpawnpointCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    if (sender->getCommandSenderWorld()->mDimension->getType() == DimensionType::OVERWORLD)
    {
        BlockPos pos = convertSourceToPlayer(sender)->getCommandSenderBlockPosition();

        sender->getCommandSenderWorld()->setSpawnPos(pos);

        gMinecraftServer->getPlayers()->broadcastAll(boost::shared_ptr<ClientboundSetSpawnPositionPacket>(
            new ClientboundSetSpawnPositionPacket(pos.x, pos.y, pos.z)));

        int intParams[3] = {pos.x, pos.y, pos.z};

        logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_DefaultSpawnpointSuccess, intParams, 3, NULL,
                       0);
    }
    else
    {
        logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_DefaultSpawnpointNotValid, NULL, 0, NULL, 0);
    }
}

boost::shared_ptr<GameCommandPacket> SetDefaultSpawnpointCommand::preparePacket()
{
    arrayWithLength<unsigned char> data;

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_SetDefaultSpawnpoint, data));
}
