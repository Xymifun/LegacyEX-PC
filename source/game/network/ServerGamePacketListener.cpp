#include "game/network/ServerGamePacketListener.hpp"

#include "game/network/ServerboundChatAutoCompletePacket.hpp"
#include "game/network/ServerboundChatPacket.hpp"
#include "game/network/ServerboundClientCommandPacket.hpp"
#include "game/network/ServerboundClientInformationPacket.hpp"
#include "game/network/ServerboundContainerAckPacket.hpp"
#include "game/network/ServerboundContainerButtonClickPacket.hpp"
#include "game/network/ServerboundContainerClickPacket.hpp"
#include "game/network/ServerboundContainerClosePacket.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/network/ServerboundInteractPacket.hpp"
#include "game/network/ServerboundKeepAlivePacket.hpp"
#include "game/network/ServerboundMovePlayerPacket.hpp"
#include "game/network/ServerboundPlayerAbilitiesPacket.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "game/network/ServerboundPlayerCommandPacket.hpp"
#include "game/network/ServerboundPlayerInputPacket.hpp"
#include "game/network/ServerboundResourcePackPacket.hpp"
#include "game/network/ServerboundSetCarriedItemPacket.hpp"
#include "game/network/ServerboundSetCreativeModeSlotPacket.hpp"
#include "game/network/ServerboundSignUpdatePacket.hpp"
#include "game/network/ServerboundSwingPacket.hpp"
#include "game/network/ServerboundTeleportToEntityPacket.hpp"
#include "game/network/ServerboundUseItemPacket.hpp"

void ServerGamePacketListener::handleAnimate(boost::shared_ptr<ServerboundSwingPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleChat(boost::shared_ptr<ServerboundChatPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleChatAutoComplete(boost::shared_ptr<ServerboundChatAutoCompletePacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleClientCommand(boost::shared_ptr<ServerboundClientCommandPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleClientInformation(boost::shared_ptr<ServerboundClientInformationPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleContainerAck(boost::shared_ptr<ServerboundContainerAckPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleContainerButtonClick(
    boost::shared_ptr<ServerboundContainerButtonClickPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleContainerClick(boost::shared_ptr<ServerboundContainerClickPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleContainerClose(boost::shared_ptr<ServerboundContainerClosePacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleCustomPayload(boost::shared_ptr<ServerboundCustomPayloadPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleContainerSetSlot(boost::shared_ptr<ServerboundContainerClickPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleInteract(boost::shared_ptr<ServerboundInteractPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleMovePlayer(boost::shared_ptr<ServerboundMovePlayerPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handlePlayerAbilities(boost::shared_ptr<ServerboundPlayerAbilitiesPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handlePlayerAction(boost::shared_ptr<ServerboundPlayerActionPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handlePlayerCommand(boost::shared_ptr<ServerboundPlayerCommandPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handlePlayerInput(boost::shared_ptr<ServerboundPlayerInputPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleSetCarriedItem(boost::shared_ptr<ServerboundSetCarriedItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleSetCreativeModeSlot(boost::shared_ptr<ServerboundSetCreativeModeSlotPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleSignUpdate(boost::shared_ptr<ServerboundSignUpdatePacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleUseItem(boost::shared_ptr<ServerboundUseItemPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleTeleportToEntityPacket(boost::shared_ptr<ServerboundTeleportToEntityPacket> packet)
{
    onUnhandledPacket(packet);
}

void ServerGamePacketListener::handleResourcePackResponse(boost::shared_ptr<ServerboundResourcePackPacket> packet)
{
    onUnhandledPacket(packet);
}

bool ServerGamePacketListener::isServerPacketListener()
{
    return true;
}
