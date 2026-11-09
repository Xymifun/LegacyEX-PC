#include "game/command/SetSpawnpointCommand.hpp"

#include <string>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"

EGameCommand SetSpawnpointCommand::getId()
{
    return eGameCommand_SetSpawnpoint;
}

int SetSpawnpointCommand::getPermissionLevel()
{
    return 2;
}

void SetSpawnpointCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);

    PlayerUID uid = stream.readPlayerUID();
    byteStream.reset();

    boost::shared_ptr<ServerPlayer> player = getPlayer(uid);

    if (sender->getCommandSenderWorld()->mDimension->getType() == DimensionType::OVERWORLD)
    {
        if (player.get() != NULL && player->mLevel != NULL)
        {
            BlockPos pos = convertSourceToPlayer(sender)->getCommandSenderBlockPosition();

            player->setRespawnPosition(new BlockPos(pos), true);

            int intParams[3] = {pos.x, pos.y, pos.z};
            std::wstring stringParams[1];

            stringParams[0] = player->getName();

            logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_SpawnpointSuccess, intParams, 3,
                           stringParams, 1);
        }
    }
    else
    {
        std::wstring stringParams[1];

        stringParams[0] = player->getName();

        logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_SpawnpointNotValid, NULL, 0, stringParams, 1);
    }
}

boost::shared_ptr<GameCommandPacket> SetSpawnpointCommand::preparePacket(PlayerUID uid)
{
    if (uid == PlayerUID(0))
    {
        return boost::shared_ptr<GameCommandPacket>();
    }

    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writePlayerUID(uid);

    return boost::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_SetSpawnpoint, byteStream.toByteArray()));
}
