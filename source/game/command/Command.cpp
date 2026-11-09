#include "game/command/Command.hpp"

#include "game/command/AdminLogCommand.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"

static AdminLogCommand* sLogger = NULL;

Command::Command()
{
}

Command::~Command()
{
}

int Command::getPermissionLevel()
{
    return 4;
}

bool Command::canExecute(boost::shared_ptr<CommandSender> sender)
{
    bool allowed = true;

    if (getPermissionLevel() != 0)
    {
        if (!sender->hasPermission(getId()))
        {
            allowed = false;
        }
    }

    return allowed;
}

void Command::logAdminAction(boost::shared_ptr<CommandSender> sender, ClientboundChatPacket::EChatPacketMessage message,
                             int* intParams, unsigned int numIntParams, std::wstring* stringParams,
                             unsigned int numStringParams)
{
    if (sLogger != NULL)
    {
        sLogger->logAdminCommand(sender, message, intParams, numIntParams, stringParams, numStringParams);
    }
}

void Command::setLogger(AdminLogCommand* logger)
{
    sLogger = logger;
}

boost::shared_ptr<ServerPlayer> Command::convertSourceToPlayer(boost::shared_ptr<CommandSender> sender)
{
    boost::shared_ptr<ServerPlayer> player = boost::dynamic_pointer_cast<ServerPlayer>(sender);

    if (player.get() != NULL)
    {
        return player;
    }

    return boost::shared_ptr<ServerPlayer>();
}

boost::shared_ptr<ServerPlayer> Command::convertToPlayer(boost::shared_ptr<CommandSender> sender,
                                                         std::wstring playerName)
{
    (void)sender;
    (void)playerName;
    return boost::shared_ptr<ServerPlayer>();
}

std::wstring Command::convertToPlayerName(boost::shared_ptr<CommandSender> sender, std::wstring playerName)
{
    return convertToPlayer(sender, playerName)->getName();
}

std::wstring Command::convertToEntityName(boost::shared_ptr<CommandSender> sender, std::wstring entityName)
{
    return convertToPlayer(sender, entityName)->getName();
}

boost::shared_ptr<ServerPlayer> Command::getPlayer(PlayerUID uid)
{
    boost::shared_ptr<ServerPlayer> player = gMinecraftServer->getPlayers()->getPlayer(uid);

    if (player.get() == NULL)
    {
        return boost::shared_ptr<ServerPlayer>();
    }

    return player;
}
