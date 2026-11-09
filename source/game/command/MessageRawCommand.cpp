#include "game/command/MessageRawCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/GameCommandPacket.hpp"

EGameCommand MessageRawCommand::getId()
{
    return eGameCommand_MessageRaw;
}

int MessageRawCommand::getPermissionLevel()
{
    return 2;
}

void MessageRawCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    byteStream.reset();
}

boost::shared_ptr<GameCommandPacket> MessageRawCommand::preparePacket(unsigned char mode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);
    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket());
}

bool MessageRawCommand::isValidWildcardPlayerArgument(std::wstring argument, int argumentIndex)
{
    return argumentIndex == 0;
}
