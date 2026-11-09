#include "game/command/KillCommand.hpp"

#include <string>

#include <boost/pointer_cast.hpp>

#include "game/command/CommandSender.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/network/ClientboundChatPacket.hpp"

EGameCommand KillCommand::getId()
{
    return eGameCommand_Kill;
}

int KillCommand::getPermissionLevel()
{
    return 2;
}

void KillCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    boost::shared_ptr<Entity> entity = boost::dynamic_pointer_cast<Entity>(sender);

    entity->kill();

    std::wstring message(L"Ouch. That look like it hurt.");

    sender->sendMessage(ClientboundChatPacket::eChatPacketMessage_Text, NULL, 0, &message, 1);
}

boost::shared_ptr<GameCommandPacket> KillCommand::preparePacket()
{
    arrayWithLength<unsigned char> data;

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Kill, data));
}
