#ifndef GAME_NETWORK_SERVERGAMEPACKETLISTENER_HPP
#define GAME_NETWORK_SERVERGAMEPACKETLISTENER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/network/PacketListener.hpp"

class ServerboundChatAutoCompletePacket;
class ServerboundChatPacket;
class ServerboundClientCommandPacket;
class ServerboundClientInformationPacket;
class ServerboundContainerAckPacket;
class ServerboundContainerButtonClickPacket;
class ServerboundContainerClickPacket;
class ServerboundContainerClosePacket;
class ServerboundCustomPayloadPacket;
class ServerboundInteractPacket;
class ServerboundKeepAlivePacket;
class ServerboundMovePlayerPacket;
class ServerboundPlayerAbilitiesPacket;
class ServerboundPlayerActionPacket;
class ServerboundPlayerCommandPacket;
class ServerboundPlayerInputPacket;
class ServerboundResourcePackPacket;
class ServerboundSetCarriedItemPacket;
class ServerboundSetCreativeModeSlotPacket;
class ServerboundSignUpdatePacket;
class ServerboundSwingPacket;
class ServerboundTeleportToEntityPacket;
class ServerboundUseItemPacket;

class ServerGamePacketListener : public PacketListener
{
public:
    virtual ~ServerGamePacketListener() {}

    virtual void handleAnimate(boost::shared_ptr<ServerboundSwingPacket> packet);
    virtual void handleChat(boost::shared_ptr<ServerboundChatPacket> packet);
    virtual void handleChatAutoComplete(boost::shared_ptr<ServerboundChatAutoCompletePacket> packet);
    virtual void handleClientCommand(boost::shared_ptr<ServerboundClientCommandPacket> packet);
    virtual void handleClientInformation(boost::shared_ptr<ServerboundClientInformationPacket> packet);
    virtual void handleContainerAck(boost::shared_ptr<ServerboundContainerAckPacket> packet);
    virtual void handleContainerButtonClick(boost::shared_ptr<ServerboundContainerButtonClickPacket> packet);
    virtual void handleContainerClick(boost::shared_ptr<ServerboundContainerClickPacket> packet);
    virtual void handleContainerClose(boost::shared_ptr<ServerboundContainerClosePacket> packet);
    virtual void handleCustomPayload(boost::shared_ptr<ServerboundCustomPayloadPacket> packet);
    virtual void handleContainerSetSlot(boost::shared_ptr<ServerboundContainerClickPacket> packet);
    virtual void handleInteract(boost::shared_ptr<ServerboundInteractPacket> packet);
    virtual void handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket> packet);
    virtual void handleMovePlayer(boost::shared_ptr<ServerboundMovePlayerPacket> packet);
    virtual void handlePlayerAbilities(boost::shared_ptr<ServerboundPlayerAbilitiesPacket> packet);
    virtual void handlePlayerAction(boost::shared_ptr<ServerboundPlayerActionPacket> packet);
    virtual void handlePlayerCommand(boost::shared_ptr<ServerboundPlayerCommandPacket> packet);
    virtual void handlePlayerInput(boost::shared_ptr<ServerboundPlayerInputPacket> packet);
    virtual void handleSetCarriedItem(boost::shared_ptr<ServerboundSetCarriedItemPacket> packet);
    virtual void handleSetCreativeModeSlot(boost::shared_ptr<ServerboundSetCreativeModeSlotPacket> packet);
    virtual void handleSignUpdate(boost::shared_ptr<ServerboundSignUpdatePacket> packet);
    virtual void handleUseItem(boost::shared_ptr<ServerboundUseItemPacket> packet);
    virtual void handleTeleportToEntityPacket(boost::shared_ptr<ServerboundTeleportToEntityPacket> packet);
    virtual void handleResourcePackResponse(boost::shared_ptr<ServerboundResourcePackPacket> packet);
    virtual bool isServerPacketListener();
};

#endif
