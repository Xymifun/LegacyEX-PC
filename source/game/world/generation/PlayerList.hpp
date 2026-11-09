#ifndef GAME_WORLD_GENERATION_PLAYERLIST_HPP
#define GAME_WORLD_GENERATION_PLAYERLIST_HPP

#include <stdint.h>
#include <string>
#include <vector>
#include <deque>

#include <boost/shared_ptr.hpp>

#include "game/core/CriticalSection.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/world/LevelRules.hpp"

class BlockPos;
class Connection;
class CompoundTag;
class Dimension;
class Entity;
class GameType;
class Level;
class LevelStorage;
class PlayerIO;
class MasterGameMode;
class MinecraftServer;
class Packet;
class Player;
class ProgressListener;
class ServerConnectionListener;
class ServerGamePacketListenerImpl;
class ServerLevel;
class ServerPlayer;
class ServerScoreboard;
class StatsUID;
class ClientboundLoginPacket;

template <typename T> class arrayWithLength;

class PlayerList
{
public:
    PlayerList(MinecraftServer* server, MasterGameMode* gameMode);
    ~PlayerList();

    bool canReceiveAllPackets(const boost::shared_ptr<ServerPlayer>& player);
    bool isOp(boost::shared_ptr<ServerPlayer> player);
    int getViewDistance();
    boost::shared_ptr<ServerPlayer> findAlivePlayerOnSystem(boost::shared_ptr<ServerPlayer> player);
    int removePlayerFromReceiving(boost::shared_ptr<ServerPlayer> player, bool usePlayerDimension, int dimension);
    int addPlayerToReceiving(boost::shared_ptr<ServerPlayer> player, bool force);
    void broadcastAll(boost::shared_ptr<Packet> packet);
    void save(boost::shared_ptr<ServerPlayer> player);
    void saveAll(ProgressListener* listener, bool announce);
    void remove(boost::shared_ptr<ServerPlayer> player);
    void removeAll();
#ifdef MINECRAFT_PC
    void applyViewDistanceCap();
#endif
    CompoundTag* load(boost::shared_ptr<ServerPlayer> player);
    void validatePlayerSpawnPosition(boost::shared_ptr<ServerPlayer> player);
    void updatePlayerGameMode(boost::shared_ptr<ServerPlayer> oldPlayer, boost::shared_ptr<ServerPlayer> newPlayer,
                              Level* level);
    int getMaxPlayers();
    void updateEntireScoreboard(ServerScoreboard* scoreboard, boost::shared_ptr<ServerPlayer> player);
    void sendLevelInfo(boost::shared_ptr<ServerPlayer> player, ServerLevel* level);
    void changeDimension(boost::shared_ptr<ServerPlayer> player, ServerLevel* level);
    void add(boost::shared_ptr<ServerPlayer> player);
    void placeNewPlayer(Connection* connection, boost::shared_ptr<ServerPlayer> player,
                        boost::shared_ptr<ClientboundLoginPacket> login);
    void setLevel(arrayWithLength<ServerLevel*> levels);
    int getMaxRange();
    void move(boost::shared_ptr<ServerPlayer> player);
    boost::shared_ptr<ServerPlayer> getPlayerForLogin(ServerConnectionListener* listener, const std::wstring& name,
                                                      PlayerUID uid, PlayerUID oldUid);
    std::vector<boost::shared_ptr<ServerPlayer> >* getPlayers();
    boost::shared_ptr<ServerPlayer> respawn(boost::shared_ptr<ServerPlayer> player, int dimension, bool conqueredEnd);
    void repositionAcrossDimension(boost::shared_ptr<Entity> entity, int lastDimension, ServerLevel* oldLevel,
                                   ServerLevel* newLevel);
    void sendAllPlayerInfo(boost::shared_ptr<ServerPlayer> player);
    void toggleDimension(boost::shared_ptr<ServerPlayer> player, int dimension);
    void tick();
    bool isTrackingBlock(const BlockPos& pos, const Dimension* dimension);
    void prioritiseBlockChanges(const BlockPos& pos, int priority);
    void broadcastAll(boost::shared_ptr<Packet> packet, int dimension);
    boost::shared_ptr<ServerPlayer> getPlayer(PlayerUID uid);
    void broadcastToTeam(boost::shared_ptr<Player> source, boost::shared_ptr<Packet> packet);
    void broadcastToAllExceptTeam(boost::shared_ptr<Player> source, boost::shared_ptr<Packet> packet);
    void broadcastPlaylistInfo(std::vector<LevelRules::LevelRulesetGroup*>* groups);
    std::wstring getPlayerNames();
    boost::shared_ptr<ServerPlayer> getPlayerByName(const std::wstring& name);
    boost::shared_ptr<ServerPlayer> getPlayerFromStatsUID(const StatsUID& uid);
    void broadcast(boost::shared_ptr<Player> source, double x, double y, double z, double radius, int dimension,
                   boost::shared_ptr<Packet> packet);
    void broadcast(double x, double y, double z, double radius, int dimension, boost::shared_ptr<Packet> packet);
    int getPlayerCount();
    int getPlayerCount(ServerLevel* level);
    void kickPlayerByShortId(unsigned char id);
    void closeServerGamePacketListenerImplBySmallId(unsigned char id);
    bool isXuidBanned(PlayerUID uid);
    void resetAllChunkFlagsOutsideArea(int minX, int minZ, int maxX, int maxZ, int flags);

    void updatePlayerPermissions(boost::shared_ptr<ServerPlayer> player);
    bool isWhiteListed(const std::wstring& name);
    void sendDebugMessage(const std::wstring& message, const std::wstring& detail);
    void sendDebugMessage(const std::wstring& message);
    void whiteList(const std::wstring& name);
    void blackList(const std::wstring& name);
    void reloadWhitelist();
    MinecraftServer* getServer();
    void setOverrideGameMode(GameType* gameMode);
    void setAllowCheatsForAllPlayers(bool allow);
    void setViewDistance(int distance);

private:
    friend class MinecraftServer;
    friend class PlayerChunkMap;
    friend class PlayerChunkContainer;
    friend class EntityTracker;
    friend class MapItemSavedData;
    friend class TrackedEntity;
    friend class DefaultGameModeCommand;
    std::vector<boost::shared_ptr<ServerPlayer> > mPlayers;
    MinecraftServer* mServer;
    MasterGameMode* mGameMode;
    int mMaxPlayers;
    std::vector<PlayerUID> mBannedPlayers;
    std::deque<unsigned char> mPlayersToKick;
    OSFastMutex mPlayersToKickLock;
    std::deque<unsigned char> mListenersToClose;
    OSFastMutex mListenersToCloseLock;
    PlayerIO* mPlayerIO;
    bool mUnknownState;
    GameType* mOverrideGameMode;
    bool mAllowCheatsForAllPlayers;
    int mViewDistance;
    int mMaxRange;
    std::vector<boost::shared_ptr<ServerPlayer> > mReceivingPlayers[3];
};

#endif
