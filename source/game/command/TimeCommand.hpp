#ifndef GAME_COMMAND_TIMECOMMAND_HPP
#define GAME_COMMAND_TIMECOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class TimeCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    void doSetTime(boost::shared_ptr<CommandSender> sender, long long time);
    void doAddTime(boost::shared_ptr<CommandSender> sender, long long time);

    static boost::shared_ptr<GameCommandPacket> preparePacket(long long time);
};

#endif
