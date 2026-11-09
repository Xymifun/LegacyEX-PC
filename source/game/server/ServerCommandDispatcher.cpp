#include "game/server/ServerCommandDispatcher.hpp"

#include "game/command/Command.hpp"
#include "game/command/ExperienceCommand.hpp"
#include "game/command/GameDifficultyCommand.hpp"
#include "game/command/KillCommand.hpp"
#include "game/command/CheckpointCommand.hpp"
#include "game/command/GameModeCommand.hpp"
#include "game/command/SetSpawnpointCommand.hpp"
#include "game/command/TimeCommand.hpp"
#include "game/item/GiveItemCommand.hpp"
#include "game/command/DefaultGameModeCommand.hpp"
#include "game/item/EnchantItemCommand.hpp"
#include "game/command/ToggleDownfallCommand.hpp"
#include "game/command/TeleportCommand.hpp"
#include "game/command/SetDefaultSpawnpointCommand.hpp"
#include "game/command/WeatherCommand.hpp"
#include "game/command/ChangeCollisionTypeCommand.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"

#include <vector>

extern MinecraftServer* gMinecraftServer;

ServerCommandDispatcher::ServerCommandDispatcher() : CommandDispatcher(), AdminLogCommand()
{
    addCommand(new TimeCommand());
    addCommand(new GameModeCommand());
    addCommand(new DefaultGameModeCommand());
    addCommand(new KillCommand());
    addCommand(new ToggleDownfallCommand());
    addCommand(new ExperienceCommand());
    addCommand(new TeleportCommand());
    addCommand(new GiveItemCommand());
    addCommand(new EnchantItemCommand());
    addCommand(new GameDifficultyCommand());
    addCommand(new SetDefaultSpawnpointCommand());
    addCommand(new WeatherCommand());
    addCommand(new SetSpawnpointCommand());
    addCommand(new CheckpointCommand());
    addCommand(new ChangeCollisionTypeCommand());
    Command::setLogger(this);
}

ServerCommandDispatcher::~ServerCommandDispatcher()
{
}

void ServerCommandDispatcher::logAdminCommand(boost::shared_ptr<CommandSender> sender,
                                              ClientboundChatPacket::EChatPacketMessage message, int* intParams,
                                              unsigned int numIntParams, std::wstring* stringParams,
                                              unsigned int numStringParams)
{
    PlayerList* players = gMinecraftServer->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> >* playerList = players->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it;
    for (it = playerList->begin(); it != playerList->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        if (message == ClientboundChatPacket::eChatPacketMessage_GameModeSuccessSelf && player != sender)
        {
            player->sendMessage(ClientboundChatPacket::eChatPacketMessage_GameModeSuccessOther, intParams, numIntParams,
                                stringParams, numStringParams);
            continue;
        }
        player->sendMessage(message, intParams, numIntParams, stringParams, numStringParams);
    }
}
