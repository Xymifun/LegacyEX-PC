#ifndef GAME_COMMAND_COMPARECOMMAND_HPP
#define GAME_COMMAND_COMPARECOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/network/GameCommandPacket.hpp"

class CompareCommand : public Command
{
public:
    virtual ~CompareCommand();

    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    static boost::shared_ptr<GameCommandPacket> preparePacket(unsigned char commandId);
};
#endif
