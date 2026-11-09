#ifndef GAME_COMMAND_DEFAULTGAMEMODECOMMAND_HPP
#define GAME_COMMAND_DEFAULTGAMEMODECOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/GameModeCommand.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class GameType;

class DefaultGameModeCommand : public GameModeCommand
{
public:
    virtual EGameCommand getId();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    void doSetGameType(const GameType* gameType);

    static boost::shared_ptr<GameCommandPacket> preparePacket(unsigned char gameType);
};

#endif
