#ifndef GAME_ITEM_REPLACEITEMCOMMAND_HPP
#define GAME_ITEM_REPLACEITEMCOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CommandSender;
class GameCommandPacket;

class ReplaceItemCommand : public Command
{
public:
    ReplaceItemCommand();

    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    static boost::shared_ptr<GameCommandPacket> preparePacket(unsigned char mode);
};

#endif
