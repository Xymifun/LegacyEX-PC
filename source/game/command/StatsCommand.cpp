#include "game/command/StatsCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

EGameCommand StatsCommand::getId()
{
    return eGameCommand_Stats;
}

int StatsCommand::getPermissionLevel()
{
    return 2;
}

void StatsCommand::execute(boost::shared_ptr<CommandSender> commandSender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    byteStream.reset();
}

boost::shared_ptr<GameCommandPacket> StatsCommand::preparePacket(unsigned char commandId)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket());
}
