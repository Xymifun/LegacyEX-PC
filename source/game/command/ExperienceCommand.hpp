#ifndef GAME_COMMAND_EXPERIENCECOMMAND_HPP
#define GAME_COMMAND_EXPERIENCECOMMAND_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class ExperienceCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    bool isValidWildcardPlayerArgument(std::wstring argument, int argumentIndex);
};

#endif
