#include "game/command/TimeCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Level.hpp"

EGameCommand TimeCommand::getId()
{
    return eGameCommand_Time;
}

int TimeCommand::getPermissionLevel()
{
    return 2;
}

void TimeCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);

    long long time = stream.readLong();
    byteStream.reset();

    doSetTime(sender, time);

    int intParams[1] = {(int)time};

    logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_TimeSuccess, intParams, 1, NULL, 0);
}

void TimeCommand::doSetTime(boost::shared_ptr<CommandSender> sender, long long time)
{
    for (unsigned int i = 0; i < gMinecraftServer->mLevels.length; i++)
    {
        gMinecraftServer->mLevels.data[i]->forceDayTime(time);
    }
}

boost::shared_ptr<GameCommandPacket> TimeCommand::preparePacket(long long time)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);

    stream.writeLong(time);

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_Time, byteStream.toByteArray()));
}
