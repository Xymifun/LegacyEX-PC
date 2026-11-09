#ifndef GAME_ENTITY_MOB_ENTITYDATACOMMAND_HPP
#define GAME_ENTITY_MOB_ENTITYDATACOMMAND_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/command/Command.hpp"
#include "game/command/CommandSender.hpp"
#include "game/command/EGameCommand.hpp"
#include "game/network/GameCommandPacket.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class EntityDataCommand : public Command
{
public:
    virtual EGameCommand getId();
    virtual int getPermissionLevel();
    virtual void execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data);

    bool isValidWildcardPlayerArgument(std::wstring argument, int argumentIndex);

    static boost::shared_ptr<GameCommandPacket> preparePacket(unsigned char mode);
};

#endif
