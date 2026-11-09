#include "game/item/ReplaceItemCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/GameCommandPacket.hpp"

EGameCommand ReplaceItemCommand::getId()
{
    return eGameCommand_ReplaceItem;
}

int ReplaceItemCommand::getPermissionLevel()
{
    return 2;
}

void ReplaceItemCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    byteStream.reset();
}

boost::shared_ptr<GameCommandPacket> ReplaceItemCommand::preparePacket(unsigned char mode)
{
    arrayWithLength<unsigned char> payload(1, false);
    payload.data[0] = mode;
    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_ReplaceItem, payload));
}
