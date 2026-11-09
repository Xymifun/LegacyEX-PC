#ifndef GAME_COMMAND_CHECKPOINTCOMMAND_HPP
#define GAME_COMMAND_CHECKPOINTCOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/CommandSender.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/core/StatsUID.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CheckpointCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    static boost::shared_ptr<GameCommandPacket> preparePacket(const StatsUID& uid, int checkpoint);
};

#endif
