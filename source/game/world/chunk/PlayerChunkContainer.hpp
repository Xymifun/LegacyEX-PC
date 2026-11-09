#ifndef GAME_WORLD_CHUNK_PLAYERCHUNKCONTAINER_HPP
#define GAME_WORLD_CHUNK_PLAYERCHUNKCONTAINER_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkPos.hpp"

class BlockEntity;
class LevelChunk;
class Packet;
class PlayerChunkMap;
class ServerPlayer;

class PlayerChunkContainer
{
    friend class PlayerChunkMap;

public:
    static const int MAX_BLOCK_CHANGES = 64;

    PlayerChunkContainer(PlayerChunkMap* map, int x, int z);
    ~PlayerChunkContainer();

    void sendToPlayer(boost::shared_ptr<ServerPlayer> player);
    void add(boost::shared_ptr<ServerPlayer> player, bool send);
    bool hasPlayer(boost::shared_ptr<ServerPlayer> player);
    void updateInhabitedTime();
    void remove(boost::shared_ptr<ServerPlayer> player, bool send);
    bool broadcast(boost::shared_ptr<Packet> packet);
    void broadcast(boost::shared_ptr<BlockEntity> blockEntity);
    bool broadcastChanges(bool allowRegionUpdate);
    void blockChanged(int x, int y, int z);
    void prioritiseBlockChanges();
    ChunkPos getPos();
    double getDistanceSqrToClosestPlayer();
    bool updatePlayerReference(boost::shared_ptr<ServerPlayer> player);
    static void removeAllPlayers(PlayerChunkContainer* container, bool send);
    bool IsGoingToSendBRUP();
    bool hasSentToPlayers();
    LevelChunk* getChunk();

private:
    PlayerChunkMap* mMap;
    std::vector<boost::shared_ptr<ServerPlayer> > mPlayers;
    ChunkPos mPos;
    arrayWithLength<short> mBlockChanges;
    LevelChunk* mChunk;
    int mBlockChangesCount;
    int mMinX;
    int mMaxX;
    int mMinY;
    int mMaxY;
    int mMinZ;
    int mMaxZ;
    int mUpdateCounter;
    bool mPrioritiseBlockChanges;
    long long mLastUpdate;
    bool mHasSentToPlayers;
};

#endif
