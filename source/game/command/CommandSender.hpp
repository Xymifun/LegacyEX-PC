#ifndef GAME_COMMAND_COMMANDSENDER_HPP
#define GAME_COMMAND_COMMANDSENDER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/command/EGameCommand.hpp"
#include "game/command/CommandStats.hpp"
#include "game/network/ClientboundChatPacket.hpp"

class BlockPos;
class Entity;
class Level;
class MinecraftServer;
class Vec3;

class CommandSender
{
public:
    virtual ~CommandSender() {}

    virtual void sendMessage(ClientboundChatPacket::EChatPacketMessage message, int* intParams,
                             unsigned int numIntParams, std::wstring* stringParams, unsigned int numStringParams) = 0;
    virtual bool hasPermission(EGameCommand command) = 0;
    virtual std::wstring getDisplayName() = 0;
    virtual BlockPos getCommandSenderBlockPosition() = 0;
    virtual Vec3* getCommandSenderWorldPosition() = 0;
    virtual Level* getCommandSenderWorld() = 0;
    virtual boost::shared_ptr<Entity> getCommandSenderEntity();
    virtual bool isSendMessageEnabled() = 0;
    virtual void setCommandStats(CommandStats::CommandStatType statType, int value) = 0;
    virtual MinecraftServer* getServer() = 0;
};

#endif
