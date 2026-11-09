#include "game/command/CloneCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

EGameCommand CloneCommand::getId()
{
    return eGameCommand_Clone;
}

int CloneCommand::getPermissionLevel()
{
    return 2;
}

void CloneCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream inputStream(&byteStream);
    byteStream.reset();
}
