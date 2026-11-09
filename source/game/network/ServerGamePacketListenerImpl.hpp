#ifndef GAME_NETWORK_SERVERGAMEPACKETLISTENERIMPL_HPP
#define GAME_NETWORK_SERVERGAMEPACKETLISTENERIMPL_HPP

#include <map>
#include <stdint.h>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/core/CriticalSection.hpp"
#include "game/core/ConsoleInputSource.hpp"
#include "game/network/ServerGamePacketListener.hpp"
#include "game/network/Connection.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/network/ClientboundPlayerPositionPacket.hpp"
#include "game/world/chunk/Chunk.hpp"
#include "game/world/generation/PlayerUID.hpp"

class MinecraftServer;
class INetworkPlayer;
class ServerPlayer;
class Entity;
class Packet;
class ClientboundChatPacket;
class ServerboundAcceptTeleportationPacket;
class ServerboundContainerAckPacket;
class ServerboundContainerButtonClickPacket;
class ServerboundContainerClickPacket;
class ServerboundContainerClosePacket;
class ServerboundCustomPayloadPacket;
class ServerboundInteractPacket;
class ServerboundMoveVehiclePacket;
class ServerboundPaddleBoatPacket;
class ServerboundPlayerAbilitiesPacket;
class ServerboundPlayerInputPacket;
class ServerboundSetCarriedItemPacket;
class ServerboundSignUpdatePacket;
class ServerboundSwingPacket;
class ServerboundUseItemPacket;
class ServerboundMovePlayerPacket;
class ServerboundPlayerActionPacket;
class ServerboundUseItemOnPacket;
class ServerboundTeleportToEntityPacket;
class ServerboundPlayerCommandPacket;
class ServerboundClientCommandPacket;
class ServerboundSetCreativeModeSlotPacket;
class ServerboundKeepAlivePacket;
class PlayerInfoPacket;
class CraftItemPacket;
class TradeItemPacket;
class DebugOptionsPacket;
class TexturePacket;
class TextureAndGeometryPacket;
class TextureChangePacket;
class TextureAndGeometryChangePacket;
class ServerSettingsChangedPacket;
class KickPlayerPacket;
class GameCommandPacket;
class VotePacket;
class PlayerReadyPacket;
class DisconnectPacket;
class GameModePacket;
class ClientboundSetPlayerTeamPacket;
class ClientboundSoundPacket;
class ClientboundPlayerPositionPacket;
class Vec3;

class ServerGamePacketListenerImpl : public ServerGamePacketListener, public ConsoleInputSource
{
public:
    ServerGamePacketListenerImpl(MinecraftServer* server, Connection* connection,
                                 boost::shared_ptr<ServerPlayer> player);
    virtual ~ServerGamePacketListenerImpl();

    void send(boost::shared_ptr<Packet> packet);
    INetworkPlayer* getNetworkPlayer();
    virtual void info(const std::wstring& message);
    virtual void warn(const std::wstring& message);
    virtual std::wstring getConsoleName();
    void disconnect(DisconnectPacket::eDisconnectReason reason);
    void setShowOnMaps(bool show);
    bool isLocal();
    int countDelayedPackets();
    void queueSend(boost::shared_ptr<Packet> packet);
    void resetPosition();
    void teleport(
        double x, double y, double z, float yRot, float xRot,
        const boost::unordered_set<const ClientboundPlayerPositionPacket::RelativeArgument*>& relativeArguments,
        bool sendPacket, bool includeRotation);
    void teleport(double x, double y, double z, float yRot, float xRot, bool sendPacket, bool includeRotation);
    void tick();
    virtual void handlePlayerInput(boost::shared_ptr<ServerboundPlayerInputPacket> packet);
    bool containsInvalidValues(boost::shared_ptr<ServerboundMoveVehiclePacket> packet);
    virtual void handleMoveVehicle(boost::shared_ptr<ServerboundMoveVehiclePacket> packet);
    virtual void handleAcceptTeleportPacket(boost::shared_ptr<ServerboundAcceptTeleportationPacket> packet);
    virtual void handleMovePlayer(boost::shared_ptr<ServerboundMovePlayerPacket> packet);
    virtual void handlePlayerAction(boost::shared_ptr<ServerboundPlayerActionPacket> packet);
    virtual void handleUseItemOn(boost::shared_ptr<ServerboundUseItemOnPacket> packet);
    virtual void handleUseItem(boost::shared_ptr<ServerboundUseItemPacket> packet);
    virtual void handleTeleportToEntityPacket(boost::shared_ptr<ServerboundTeleportToEntityPacket> packet);
    virtual void handlePaddleBoat(boost::shared_ptr<ServerboundPaddleBoatPacket> packet);
    virtual void onDisconnect(DisconnectPacket::eDisconnectReason reason, void* userData);
    virtual void handleSetCarriedItem(boost::shared_ptr<ServerboundSetCarriedItemPacket> packet);
    virtual void handleChat(boost::shared_ptr<ClientboundChatPacket> packet);
#ifdef MINECRAFT_PC
    virtual void handleChat(boost::shared_ptr<ServerboundChatPacket> packet);
#endif
    virtual void handleAnimate(boost::shared_ptr<ServerboundSwingPacket> packet);
    virtual void handlePlayerCommand(boost::shared_ptr<ServerboundPlayerCommandPacket> packet);
    virtual void handleInteract(boost::shared_ptr<ServerboundInteractPacket> packet);
    virtual void handleClientCommand(boost::shared_ptr<ServerboundClientCommandPacket> packet);
    virtual void handleContainerClose(boost::shared_ptr<ServerboundContainerClosePacket> packet);
    virtual void handleContainerClick(boost::shared_ptr<ServerboundContainerClickPacket> packet);
    virtual void handleContainerButtonClick(boost::shared_ptr<ServerboundContainerButtonClickPacket> packet);
    virtual void handleSetCreativeModeSlot(boost::shared_ptr<ServerboundSetCreativeModeSlotPacket> packet);
    virtual void handleContainerAck(boost::shared_ptr<ServerboundContainerAckPacket> packet);
    virtual void handleSignUpdate(boost::shared_ptr<ServerboundSignUpdatePacket> packet);
    uint64_t getMilliseconds();
    virtual void handleKeepAlive(boost::shared_ptr<ServerboundKeepAlivePacket> packet);
    virtual void handlePlayerInfo(boost::shared_ptr<PlayerInfoPacket> packet);
    virtual void handlePlayerAbilities(boost::shared_ptr<ServerboundPlayerAbilitiesPacket> packet);
    void NameItemOnAnvil(const boost::shared_ptr<ServerboundCustomPayloadPacket>& packet, bool useName);
    virtual void handleCustomPayload(boost::shared_ptr<ServerboundCustomPayloadPacket> packet);
    virtual bool isDisconnected();
    virtual void handleCraftItem(boost::shared_ptr<CraftItemPacket> packet);
    virtual void handleTradeItem(boost::shared_ptr<TradeItemPacket> packet);
    virtual void handleDebugOptions(boost::shared_ptr<DebugOptionsPacket> packet);
    void handleTextureReceived(const std::wstring& name);
    virtual void handleTexture(boost::shared_ptr<TexturePacket> packet);
    void handleTextureAndGeometryReceived(const std::wstring& name);
    virtual void handleTextureAndGeometry(boost::shared_ptr<TextureAndGeometryPacket> packet);
    virtual void handleTextureChange(boost::shared_ptr<TextureChangePacket> packet);
    virtual void handleTextureAndGeometryChange(boost::shared_ptr<TextureAndGeometryChangePacket> packet);
    virtual void handleServerSettingsChanged(boost::shared_ptr<ServerSettingsChangedPacket> packet);
    virtual void handleKickPlayer(boost::shared_ptr<KickPlayerPacket> packet);
    virtual void handleGameCommand(boost::shared_ptr<GameCommandPacket> packet);
    virtual void handleVote(boost::shared_ptr<VotePacket> packet);
    virtual void handlePlayerReady(boost::shared_ptr<PlayerReadyPacket> packet);
    virtual void handleDisconnect(boost::shared_ptr<DisconnectPacket> packet);
    virtual void handleGameMode(boost::shared_ptr<GameModePacket> packet);
    virtual void handleClientboundSetPlayerTeamPacket(boost::shared_ptr<ClientboundSetPlayerTeamPacket> packet);
    virtual void handleSoundEvent(boost::shared_ptr<ClientboundSoundPacket> packet);
    bool isGuest();
    Connection* getConnection();
    bool containsInvalidValues(boost::shared_ptr<ServerboundMovePlayerPacket> packet);
    void handleCommand(const std::wstring& command);

private:
    friend class PlayerList;
    friend class ServerPlayer;
    friend class ServerConnection;
    friend class ServerConnectionListener;
    static const int TELEPORT_RESEND_TICKS = 100;
    static const int DROP_SPAM_LIMIT = 200;
    static const int DROP_SPAM_STEP = 20;

public:
    Connection* mConnection;
#ifdef MINECRAFT_PC
    int mViewDistance;
    int mRequestedViewDistance;
    static const std::wstring VIEW_DISTANCE_PACKET;
#endif

private:
    bool mDisconnected;
    OSFastMutex mDisconnectMutex;
    PlayerUID mPlayerUID;
    PlayerUID mOldPlayerUID;
    bool mFriendsOnly;
    MinecraftServer* mServer;
    boost::shared_ptr<ServerPlayer> mPlayer;
    int mTickCount;
    int mKeepAliveId;
    uint64_t mKeepAliveTime;
    int mKeepAlivePending;
    int mChatSpamTickCount;
    int mDropSpamTickCount;
    double mFirstGoodX;
    double mFirstGoodY;
    double mFirstGoodZ;
    double mLastGoodX;
    double mLastGoodY;
    double mLastGoodZ;
    boost::weak_ptr<Entity> mLastVehicle;
    double mVehicleFirstGoodX;
    double mVehicleFirstGoodY;
    double mVehicleFirstGoodZ;
    double mVehicleLastGoodX;
    double mVehicleLastGoodY;
    double mVehicleLastGoodZ;
    Vec3* mPendingTeleportPosition;
    int mTeleportId;
    int mTeleportTime;
    bool mClientIsFloating;
    int mAboveGroundTickCount;
    bool mClientVehicleIsFloating;
    int mAboveGroundVehicleTickCount;
    int mReceivedMovePacketCount;
    int mKnownMovePacketCount;
    boost::unordered_map<int, short, IntKeyHash, IntKeyEq> mContainerType;
    bool mConnectionClosed;
    std::vector<std::wstring> mNames;
    bool mUseAltLeaveMessage;
#ifdef MINECRAFT_PC
    bool pcChecksMovement();
    void pcReport(const char* what);
    void pcPullBack(double x, double y, double z, const char* what);
    bool mPcSupportKnown;
    double mPcSupportX;
    double mPcSupportY;
    double mPcSupportZ;
    bool mPcHovering;
    int mPcHoverTicks;
    int mPcReportTick;
#endif
};

INetworkPlayer* getNetworkPlayerFromIndex(int index);

#endif
