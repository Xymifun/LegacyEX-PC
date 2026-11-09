#include "game/command/SummonCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

EGameCommand SummonCommand::getId()
{
    return eGameCommand_Summon;
}

int SummonCommand::getPermissionLevel()
{
    return 2;
}

void SummonCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    byteStream.reset();
}

boost::shared_ptr<GameCommandPacket> SummonCommand::preparePacket(unsigned char entityType)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket());
}
