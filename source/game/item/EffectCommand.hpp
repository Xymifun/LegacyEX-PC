#ifndef GAME_ITEM_EFFECTCOMMAND_HPP
#define GAME_ITEM_EFFECTCOMMAND_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/CommandSender.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class EffectCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);
    std::wstring getUsage(CommandSender* sender);
    std::vector<std::wstring> getPlayerNames();
    bool isValidWildcardPlayerArgument(std::wstring playerName, int argumentIndex);
};

#endif
