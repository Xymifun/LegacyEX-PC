#ifndef GAME_COMMAND_COMMANDDISPATCHER_HPP
#define GAME_COMMAND_COMMANDDISPATCHER_HPP

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CommandSender;

class CommandDispatcher
{
public:
    CommandDispatcher();
    virtual ~CommandDispatcher();

    unsigned int performCommand(boost::shared_ptr<CommandSender> sender, EGameCommand command,
                                arrayWithLength<unsigned char> data);
    Command* addCommand(Command* command);

private:
    boost::unordered_map<EGameCommand, Command*> mCommandsById;
    boost::unordered_set<Command*> mCommands;
};

#endif
