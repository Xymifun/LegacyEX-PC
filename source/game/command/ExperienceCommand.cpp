#include "game/command/ExperienceCommand.hpp"

EGameCommand ExperienceCommand::getId()
{
    return eGameCommand_Experience;
}

int ExperienceCommand::getPermissionLevel()
{
    return 2;
}

void ExperienceCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
}

bool ExperienceCommand::isValidWildcardPlayerArgument(std::wstring argument, int argumentIndex)
{
    return argumentIndex == 0;
}
