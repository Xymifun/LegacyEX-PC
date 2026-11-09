#include "game/command/SayCommand.hpp"


EGameCommand SayCommand::getId()
{
    return eGameCommand_Say;
}

int SayCommand::getPermissionLevel()
{
    return 1;
}

void SayCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
}

boost::shared_ptr<GameCommandPacket> SayCommand::preparePacket(unsigned char mode)
{
    arrayWithLength<unsigned char> payload(1, false);
    payload.data[0] = mode;
    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Say, payload));
}
