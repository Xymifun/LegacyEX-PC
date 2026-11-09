#include "game/command/GameModeCommand.hpp"

#include <string>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/GameType.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Level.hpp"

EGameCommand GameModeCommand::getId()
{
    return eGameCommand_GameMode;
}

int GameModeCommand::getPermissionLevel()
{
    return 2;
}

void GameModeCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);

    PlayerUID uid = stream.readPlayerUID();
    unsigned char gameMode = stream.readByte();
    byteStream.reset();

    boost::shared_ptr<ServerPlayer> player = getPlayer(uid);

    if (player)
    {
        player->setGameMode(const_cast<GameType*>(GameType::byId(gameMode)));
        player->mFallDistance = 0.0f;

        if (sender->getCommandSenderWorld()->getGameRules()->getBoolean(GameRules::RULE_SENDCOMMANDFEEDBACK))
        {
            player->sendMessage(ClientboundChatPacket::eChatPacketMessage_GameModeChanged, NULL, 0, NULL, 0);
        }

        int intParams[1] = {gameMode};
        std::wstring stringParams[1];

        stringParams[0] = player->getName();

        if (player != sender)
        {
            logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_GameModeSuccessOther, intParams, 1,
                           stringParams, 1);
        }
        else
        {
            logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_GameModeSuccessSelf, intParams, 1,
                           stringParams, 1);
        }
    }
}

boost::shared_ptr<GameCommandPacket> GameModeCommand::preparePacket(PlayerUID uid, unsigned char gameMode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writePlayerUID(uid);
    stream.writeByte(gameMode);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_GameMode, byteStream.toByteArray()));
}
