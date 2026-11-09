#ifndef GAME_WORLD_CHUNK_PLAYERCHUNKMAP_HPP
#define GAME_WORLD_CHUNK_PLAYERCHUNKMAP_HPP

#include <stdint.h>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/world/block/BlockPos.hpp"

class Level;
class ServerLevel;
class ServerPlayer;
class Packet;
class PlayerChunkContainer;

struct LongKeyHash
{
    std::size_t operator()(long long value) const { return static_cast<std::size_t>(value ^ (value >> 32)); }
};

struct LongKeyEq
{
    bool operator()(long long left, long long right) const { return left == right; }
};

class PlayerChunkMap
{
    friend class PlayerChunkContainer;

public:
    struct PlayerChunkAddRequest
    {
        int x;
        int z;
        boost::shared_ptr<ServerPlayer> player;
        bool add;

        PlayerChunkAddRequest(int chunkX, int chunkZ, boost::shared_ptr<ServerPlayer> chunkPlayer, bool shouldAdd)
            : x(chunkX),
              z(chunkZ),
              player(chunkPlayer),
              add(shouldAdd)
        {
        }
    };

    typedef std::vector<boost::shared_ptr<ServerPlayer> > PlayerVector;
    typedef std::vector<PlayerChunkContainer*> ContainerVector;
    typedef std::vector<PlayerChunkAddRequest> RequestVector;
    typedef boost::unordered_map<long long, PlayerChunkContainer*, LongKeyHash, LongKeyEq> ContainerMap;

    PlayerChunkMap(ServerLevel* level, int dimension, int viewDistance);
    virtual ~PlayerChunkMap();

    virtual PlayerChunkContainer* getOrCreateChunk(int x, int z);
    virtual void getChunkAndAddPlayer(int x, int z, boost::shared_ptr<ServerPlayer> player, bool send);
    virtual void add(boost::shared_ptr<ServerPlayer> player);
    virtual void remove(boost::shared_ptr<ServerPlayer> player);
    virtual void removeOutsideArea(boost::shared_ptr<ServerPlayer> player, int minX, int minZ, int maxX, int maxZ);
    virtual void move(boost::shared_ptr<ServerPlayer> player);
    virtual void removeOutsideAreaForAllPlayers(int minX, int minZ, int maxX, int maxZ);
    virtual void reloadAllPlayers(int minX, int minZ, int maxX, int maxZ);
    virtual void onPlayerJoin(boost::shared_ptr<ServerPlayer> player);
    virtual void onPlayerLeave(boost::shared_ptr<ServerPlayer> player);

    void flagEntitiesToBeRemoved(unsigned int* count, bool* removed);
    Level* getLevel();
    long long tickAddRequests(boost::shared_ptr<ServerPlayer> player, bool force);
    bool chunkInRange(int x, int z, int centerX, int centerZ);
    bool shouldActuallyDropChunks();
    void setViewDistance(int distance);
    void tick();
    static long long getKeyForChunk(int x, int z);
    PlayerChunkContainer* getChunkIfLoaded(int x, int z);
    void getChunkAndRemovePlayer(int x, int z, boost::shared_ptr<ServerPlayer> player, bool send);
    void broadcastBlockUpdate(boost::shared_ptr<Packet> packet, int x, int y, int z);
    void blockChanged(const BlockPos& pos);
    bool isTrackingBlock(int x, int y, int z);
    void prioritiseBlockChanges(int x, int y, int z);
    bool isPlayerIn(boost::shared_ptr<ServerPlayer> player, int x, int z);
    static int convertChunkRangeToBlock(int range);
    void setRadius(int radius);
    bool hasChunk(int x, int z);
    int getMaxRange();
    int getBlockRegionPacketsToBeSent(boost::shared_ptr<ServerPlayer> player);
    void PauseBlockUpdatePackets(bool pause);
#ifdef MINECRAFT_PC
    int getPlayerRadius(const boost::shared_ptr<ServerPlayer>& player);
    void setPlayerViewDistance(boost::shared_ptr<ServerPlayer> player, int distance);
#endif

protected:
#ifdef MINECRAFT_PC
    static bool inRange(int x, int z, int centerX, int centerZ, int radius);
    void sendVisibleArea(boost::shared_ptr<ServerPlayer> player, int minX, int maxX, int minZ, int maxZ);
#endif
    PlayerVector mPlayers;
    ContainerMap mChunks;
    ContainerVector mChangedChunks;
    ContainerVector mAllChunks;
    RequestVector mAddRequests;
    int mTicksUntilNextUpdate;
    Level* mLevel;
    int mViewDistance;
    int mDimension;
    long long mLastUpdate;
    bool mShouldDropChunks;
    bool mPauseBlockUpdatePackets;
};

#endif
