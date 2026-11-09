#ifndef GAME_ENTITY_MOB_TRACKEDENTITY_HPP
#define GAME_ENTITY_MOB_TRACKEDENTITY_HPP

#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/ui/AbstractContainerMenu.hpp"

class EntityTracker;
class Packet;
class Player;
class ServerPlayer;

class TrackedEntity
{
public:
    TrackedEntity(boost::shared_ptr<Entity> entity, int range, int maxRange, int updateInterval, bool trackDelta);

    bool canBySeenBy(boost::shared_ptr<ServerPlayer> player);
    int isVisibleTo(EntityTracker* tracker, const boost::shared_ptr<ServerPlayer>& player, bool force,
                    bool skipRangeCheck);
    boost::shared_ptr<Packet> getAddEntityPacket();
    bool ShouldSendEntityDataOnContruction(boost::shared_ptr<Entity> entity);
    void updatePlayer(EntityTracker* tracker, boost::shared_ptr<ServerPlayer> player);
    void updatePlayers(EntityTracker* tracker, std::vector<boost::shared_ptr<Player> >* players);
    void broadcast(boost::shared_ptr<Packet> packet);
    void broadcastAndSend(boost::shared_ptr<Packet> packet);
    void sendDirtyEntityData();
    void resetPlayerVisibility();
    void tick(EntityTracker* tracker, std::vector<boost::shared_ptr<Player> >* players);
    void broadcastRemoved();
    void removePlayer(EntityTracker* tracker, boost::shared_ptr<ServerPlayer> player);
    void clear(boost::shared_ptr<ServerPlayer> player);
    boost::shared_ptr<Entity> getEntity();
    void setMaxRange(int maxRange);

    boost::shared_ptr<Entity> mEntity;
    int mRange;
    int mMaxRange;
    int mUpdateInterval;
    int mXp;
    int mYp;
    int mZp;
    int mYRotp;
    int mXRotp;
    int mYHeadRotp;
    double mXap;
    double mYap;
    double mZap;
    int mTickCount;
    double mXpu;
    double mYpu;
    double mZpu;
    bool mUpdatedPlayerVisibility;
    bool mTrackDelta;
    int mTeleportDelay;
    std::vector<boost::weak_ptr<Entity> > mLastPassengers;
    bool mWasRiding;
    bool mWasOnGround;
    bool mPlayerEntitiesUpdated;
    boost::unordered_set<boost::shared_ptr<ServerPlayer>, _PlayerKeyHash, _PlayerKeyEq> mSeenBy;
};

#endif
