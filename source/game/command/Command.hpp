#ifndef GAME_COMMAND_COMMAND_HPP
#define GAME_COMMAND_COMMAND_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/command/AdminLogCommand.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/PlayerUID.hpp"

class AdminLogCommand;
class Entity;
class ServerPlayer;

class Command
{
public:
    Command();
    virtual ~Command();

    virtual EGameCommand getId() = 0;
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data) = 0;
    virtual bool canExecute(boost::shared_ptr<CommandSender> sender);

    boost::shared_ptr<ServerPlayer> getPlayer(PlayerUID uid);

    static void setLogger(AdminLogCommand* logger);
    static void logAdminAction(boost::shared_ptr<CommandSender> sender,
                               ClientboundChatPacket::EChatPacketMessage message, int* intParams,
                               unsigned int numIntParams, std::wstring* stringParams, unsigned int numStringParams);

    static boost::shared_ptr<ServerPlayer> convertSourceToPlayer(boost::shared_ptr<CommandSender> sender);
    static boost::shared_ptr<ServerPlayer> convertToPlayer(boost::shared_ptr<CommandSender> sender,
                                                           std::wstring playerName);
    static std::wstring convertToPlayerName(boost::shared_ptr<CommandSender> sender, std::wstring playerName);
    static boost::shared_ptr<Entity> convertToEntity(boost::shared_ptr<CommandSender> sender, std::wstring entityName);
    static std::wstring convertToEntityName(boost::shared_ptr<CommandSender> sender, std::wstring entityName);
    static std::vector<boost::shared_ptr<Entity> > convertToEntityList(boost::shared_ptr<CommandSender> sender,
                                                                       std::wstring entityName);
};

#endif
