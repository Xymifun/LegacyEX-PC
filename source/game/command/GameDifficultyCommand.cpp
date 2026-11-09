#include "game/command/GameDifficultyCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Difficulty.hpp"
#include "game/server/MinecraftServer.hpp"

extern MinecraftServer* gMinecraftServer;

EGameCommand GameDifficultyCommand::getId()
{
    return eGameCommand_GameDifficulty;
}

int GameDifficultyCommand::getPermissionLevel()
{
    return 2;
}

void GameDifficultyCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);

    int difficulty = stream.readInt();
    byteStream.reset();

    gMinecraftServer->setDifficulty(Difficulty::byId(difficulty));

    int intParams[1] = {difficulty};
    logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_DifficultySuccess, intParams, 1, NULL, 0);
}

boost::shared_ptr<GameCommandPacket> GameDifficultyCommand::preparePacket(int difficulty)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writeInt(difficulty);

    return boost::shared_ptr<GameCommandPacket>(
        new GameCommandPacket(eGameCommand_GameDifficulty, byteStream.toByteArray()));
}
