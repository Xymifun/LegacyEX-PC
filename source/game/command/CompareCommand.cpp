#include "game/command/CompareCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

CompareCommand::~CompareCommand()
{
}

EGameCommand CompareCommand::getId()
{
    return eGameCommand_Compare;
}

int CompareCommand::getPermissionLevel()
{
    return 2;
}

void CompareCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    byteStream.reset();
}

boost::shared_ptr<GameCommandPacket> CompareCommand::preparePacket(unsigned char commandId)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writeByte(commandId);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Compare, byteStream.toByteArray()));
}
