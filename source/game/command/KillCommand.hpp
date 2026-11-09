#ifndef GAME_COMMAND_KILLCOMMAND_HPP
#define GAME_COMMAND_KILLCOMMAND_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CommandSender;

class KillCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    bool isValidWildcardPlayerArgument(std::wstring playerName, int argumentIndex);

    static boost::shared_ptr<GameCommandPacket> preparePacket();
};

#endif
