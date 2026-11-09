#include "game/command/TitleCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

EGameCommand TitleCommand::getId()
{
    return eGameCommand_Title;
}

int TitleCommand::getPermissionLevel()
{
    return 2;
}

void TitleCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    byteStream.reset();
}

boost::shared_ptr<GameCommandPacket> TitleCommand::preparePacket(unsigned char titleType)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket());
}
