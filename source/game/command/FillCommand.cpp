#include "game/command/FillCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

EGameCommand FillCommand::getId()
{
    return eGameCommand_Fill;
}

int FillCommand::getPermissionLevel()
{
    return 2;
}

void FillCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    byteStream.reset();
}

boost::shared_ptr<GameCommandPacket> FillCommand::preparePacket(unsigned char mode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writeByte(mode);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Fill, byteStream.toByteArray()));
}
