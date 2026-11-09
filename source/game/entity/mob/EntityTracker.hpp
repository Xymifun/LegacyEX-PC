#ifndef GAME_ENTITY_MOB_ENTITYTRACKER_HPP
#define GAME_ENTITY_MOB_ENTITYTRACKER_HPP

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/world/chunk/Chunk.hpp"

class LevelChunk;
class Packet;
class ServerGamePacketListenerImpl;
class ServerLevel;
class ServerPlayer;
class TrackedEntity;

class EntityTracker
{
public:
    EntityTracker(ServerLevel* level);

    static unsigned int truncate(double value);
    static void setPacketCoordinates(boost::shared_ptr<Entity> entity, double x, double y, double z);

    void addEntity(boost::shared_ptr<Entity> entity, int range, int updateInterval, bool sendVelocityUpdates);
    void addEntity(boost::shared_ptr<Entity> entity, int range, int updateInterval);
    void addEntity(boost::shared_ptr<Entity> entity);
    void removeEntity(boost::shared_ptr<Entity> entity);
    void removePlayer(boost::shared_ptr<Entity> entity);
    void tick(boost::shared_ptr<ServerPlayer> player);
    void updateVisibility(boost::shared_ptr<ServerPlayer> player);
    void broadcast(boost::shared_ptr<Entity> entity, boost::shared_ptr<Packet> packet);
    void broadcastAndSend(boost::shared_ptr<Entity> entity, boost::shared_ptr<Packet> packet);
    void clear(boost::shared_ptr<ServerPlayer> player);
    void playerLoadedChunk(boost::shared_ptr<ServerPlayer> player, LevelChunk* chunk);
    void setViewDistance(int distance);
    void updateMaxRange();
    boost::shared_ptr<TrackedEntity> getTracker(boost::shared_ptr<Entity> entity);

private:
    ServerLevel* mLevel;
    boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >
        mTrackedEntities;
    boost::unordered_map<int, boost::shared_ptr<TrackedEntity>, IntKeyHash2, IntKeyEq> mEntityMap;
    int mMaxRange;
};

#endif
