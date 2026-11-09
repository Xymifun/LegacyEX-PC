#ifndef GAME_COMMAND_SETDEFAULTSPAWNPOINTCOMMAND_HPP
#define GAME_COMMAND_SETDEFAULTSPAWNPOINTCOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class SetDefaultSpawnpointCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    static boost::shared_ptr<GameCommandPacket> preparePacket();
};

#endif
