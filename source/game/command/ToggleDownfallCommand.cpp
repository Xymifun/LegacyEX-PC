#include "game/command/ToggleDownfallCommand.hpp"

#include <string>

#include "game/network/ClientboundChatPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"

EGameCommand ToggleDownfallCommand::getId()
{
    return eGameCommand_ToggleDownfall;
}

int ToggleDownfallCommand::getPermissionLevel()
{
    return 2;
}

void ToggleDownfallCommand::doToggleDownfall()
{
    LevelData levelData(gMinecraftServer->mLevels.data[0]->getLevelData());

    levelData.setRaining(!levelData.isRaining());
}

void ToggleDownfallCommand::execute(boost::shared_ptr<CommandSender> sender, arrayWithLength<unsigned char> data)
{
    doToggleDownfall();

    std::wstring stringParams[1];

    stringParams[0] = L"commands.downfall.success";

    logAdminAction(sender, ClientboundChatPacket::eChatPacketMessage_Text, NULL, 0, stringParams, 1);
}

boost::shared_ptr<GameCommandPacket> ToggleDownfallCommand::preparePacket()
{
    arrayWithLength<unsigned char> data;

    return boost::shared_ptr<GameCommandPacket>(new GameCommandPacket(eGameCommand_ToggleDownfall, data));
}
