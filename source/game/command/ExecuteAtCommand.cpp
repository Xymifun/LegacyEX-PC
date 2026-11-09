#include "game/command/ExecuteAtCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

EGameCommand ExecuteAtCommand::getId()
{
    return eGameCommand_ExecuteAt;
}

int ExecuteAtCommand::getPermissionLevel()
{
    return 2;
}

void ExecuteAtCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    byteStream.reset();
}

boost::shared_ptr<GameCommandPacket> ExecuteAtCommand::preparePacket(unsigned char mode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket());
}

bool ExecuteAtCommand::isValidWildcardPlayerArgument(std::wstring argument, int argumentIndex)
{
    return argumentIndex == 0;
}
