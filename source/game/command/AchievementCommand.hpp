#ifndef GAME_COMMAND_ACHIEVEMENTCOMMAND_HPP
#define GAME_COMMAND_ACHIEVEMENTCOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CommandSender;

class AchievementCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    bool isValidWildcardPlayerArgument(int argumentIndex);

    static boost::shared_ptr<GameCommandPacket> preparePacket(unsigned char achievementId);
};

#endif
