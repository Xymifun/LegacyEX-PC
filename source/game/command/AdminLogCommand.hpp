#ifndef GAME_COMMAND_ADMINLOGCOMMAND_HPP
#define GAME_COMMAND_ADMINLOGCOMMAND_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/network/ClientboundChatPacket.hpp"

class CommandSender;

class AdminLogCommand
{
public:
    virtual void logAdminCommand(boost::shared_ptr<CommandSender> sender,
                                 ClientboundChatPacket::EChatPacketMessage message, int* intParams,
                                 unsigned int numIntParams, std::wstring* stringParams,
                                 unsigned int numStringParams) = 0;
};

#endif
