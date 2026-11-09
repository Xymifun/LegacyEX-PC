#include "game/command/AchievementCommand.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"

EGameCommand AchievementCommand::getId()
{
    return eGameCommand_Achievement;
}

int AchievementCommand::getPermissionLevel()
{
    return 2;
}

void AchievementCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    ByteArrayInputStream byteStream(data);
    DataInputStream stream(&byteStream);
    byteStream.reset();
}

bool AchievementCommand::isValidWildcardPlayerArgument(int argumentIndex)
{
    return argumentIndex == 2;
}

boost::shared_ptr<GameCommandPacket> AchievementCommand::preparePacket(unsigned char achievementId)
{
    ByteArrayOutputStream byteStream;
    DataOutputStream stream(&byteStream);
    (void)achievementId;
    (void)stream;
    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket());
}
