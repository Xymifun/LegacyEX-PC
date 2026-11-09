#ifndef GAME_COMMAND_TELEPORTCOMMAND_HPP
#define GAME_COMMAND_TELEPORTCOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/PlayerUID.hpp"

class CommandSender;

class TeleportCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);
    static boost::shared_ptr<GameCommandPacket> preparePacket(PlayerUID source, PlayerUID target);
    static boost::shared_ptr<GameCommandPacket> preparePacket(PlayerUID target, float x, float y, float z);
};

#endif
