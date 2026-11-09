#include "game/item/EffectCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/DataInputStream.hpp"

EGameCommand EffectCommand::getId()
{
    return eGameCommand_Effect;
}

int EffectCommand::getPermissionLevel()
{
    return 2;
}

void EffectCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream inputStream(&byteStream);
    std::wstring playerName = inputStream.readUTF();
    (void)playerName;

    byteStream.reset();
}
