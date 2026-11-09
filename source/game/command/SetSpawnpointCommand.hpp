#ifndef GAME_COMMAND_SETSPAWNPOINTCOMMAND_HPP
#define GAME_COMMAND_SETSPAWNPOINTCOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/PlayerUID.hpp"

class SetSpawnpointCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    static boost::shared_ptr<GameCommandPacket> preparePacket(PlayerUID uid);
};

#endif
