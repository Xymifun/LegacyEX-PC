#ifndef GAME_SERVER_SERVERCOMMANDDISPATCHER_HPP
#define GAME_SERVER_SERVERCOMMANDDISPATCHER_HPP

#include "game/command/CommandDispatcher.hpp"

class ServerCommandDispatcher : public CommandDispatcher, public AdminLogCommand
{
public:
    ServerCommandDispatcher();
    virtual ~ServerCommandDispatcher();

    virtual void logAdminCommand(boost::shared_ptr<CommandSender> sender,
                                 ClientboundChatPacket::EChatPacketMessage message, int* intParams,
                                 unsigned int numIntParams, std::wstring* stringParams, unsigned int numStringParams);
};

#endif
