#include "game/world/block/BlockDataCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/world/block/BlockPos.hpp"

EGameCommand BlockDataCommand::getId()
{
    return eGameCommand_BlockData;
}

int BlockDataCommand::getPermissionLevel()
{
    return 2;
}

void BlockDataCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    BlockPos pos;
    byteStream.reset();
}

boost::shared_ptr<GameCommandPacket> BlockDataCommand::preparePacket(unsigned char mode)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket());
}
