#include "game/command/CommandDispatcher.hpp"

#include "game/command/CommandSender.hpp"

CommandDispatcher::CommandDispatcher() : mCommandsById(), mCommands()
{
}

CommandDispatcher::~CommandDispatcher()
{
    for (boost::unordered_set<Command*>::const_iterator it = mCommands.begin(), end = mCommands.end(); it != end; ++it)
    {
        delete *it;
    }
}

unsigned int CommandDispatcher::performCommand(boost::shared_ptr<CommandSender> sender, EGameCommand command,
                                               arrayWithLength<unsigned char> data)
{
    boost::unordered_map<EGameCommand, Command*>::iterator it = mCommandsById.find(command);
    if (it != mCommandsById.end())
    {
        Command* selected = it->second;
        if (selected->canExecute(sender))
        {
            selected->execute(sender, data);
        }
    }
    return 0;
}

Command* CommandDispatcher::addCommand(Command* command)
{
    mCommandsById[command->getId()] = command;
    mCommands.insert(command);
    return command;
}
