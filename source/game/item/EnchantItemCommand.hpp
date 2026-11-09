#ifndef GAME_ITEM_ENCHANTITEMCOMMAND_HPP
#define GAME_ITEM_ENCHANTITEMCOMMAND_HPP

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Player;

class EnchantItemCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    static boost::shared_ptr<GameCommandPacket> preparePacket(boost::shared_ptr<Player> player, int enchantmentId,
                                                              int level);
};

#endif
