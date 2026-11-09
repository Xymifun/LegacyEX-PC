#ifndef GAME_CORE_PORTALFORCER_HPP
#define GAME_CORE_PORTALFORCER_HPP

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"

class Entity;
class ServerLevel;

class PortalForcer
{
public:
    class PortalPosition : public BlockPos
    {
    public:
        PortalPosition(const BlockPos& pos, long long lastUpdate);

        long long lastUpdate;
    };

    PortalForcer(ServerLevel* level);
    ~PortalForcer();

    bool HasCachedPortal(const ChunkPos& chunkPos, BlockPos& portalPos);
    void uncacheLeastUsedPortal();
    bool findPortal(boost::shared_ptr<Entity> entity, float rotation, int chunkX, int chunkZ);
    bool createPortal(boost::shared_ptr<Entity> entity);
    void force(boost::shared_ptr<Entity> entity, float rotation, int chunkX, int chunkZ);
    bool isBlocked(const BlockPos& pos);
    void tick(long long gameTime);

private:
    ServerLevel* level;
    Random* random;
    boost::unordered_map<ChunkPos, PortalPosition*, ChunkPosKeyHash, ChunkPosKeyEq> portalCache;
    std::vector<ChunkPos> portalPositions;
};

#endif
