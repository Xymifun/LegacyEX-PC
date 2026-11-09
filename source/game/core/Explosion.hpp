#ifndef GAME_CORE_EXPLOSION_HPP
#define GAME_CORE_EXPLOSION_HPP

#include <functional>
#include <utility>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/ui/AbstractContainerMenu.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"

class Entity;
class Level;
class LivingEntity;
class Player;
class Vec3;


class Explosion
{
public:
    typedef boost::unordered_set<BlockPos, boost::hash<const BlockPos>, std::equal_to<const BlockPos>,
                                 std::allocator<BlockPos> >
        ToBlowSet;
    typedef boost::unordered_map<boost::shared_ptr<Player>, Vec3*, _PlayerKeyHash, _PlayerKeyEq,
                                 std::allocator<std::pair<const boost::shared_ptr<Player>, Vec3*> > >
        HitPlayerMap;

    Explosion(Level* level, boost::shared_ptr<Entity> source, double x, double y, double z, float radius,
              float damageRadius);
    Explosion(Level* level, boost::shared_ptr<Entity> source, double x, double y, double z, float radius,
              std::vector<BlockPos>* toBlow, float damageRadius);
    Explosion(Level* level, boost::shared_ptr<Entity> source, double x, double y, double z, float radius, bool fire,
              bool breaksBlocks, bool playersOnly, std::vector<BlockPos>* toBlow, float damageRadius);
    Explosion(Level* level, boost::shared_ptr<Entity> source, double x, double y, double z, float radius, bool fire,
              bool breaksBlocks, bool playersOnly, bool noBlockDrops, bool silent, float damageRadius,
              bool breaksBlocksUnderwater);
    ~Explosion();

    void explode();
    void finalizeExplosion(bool spawnParticles, std::vector<BlockPos>* toBlow);

    Vec3* getHitPlayerKnockback(boost::shared_ptr<Player> player);
    boost::shared_ptr<LivingEntity> getSourceMob();
    boost::shared_ptr<Entity> getDirectSource();

    void clearToBlow();
    ToBlowSet* getToBlow();
    HitPlayerMap* getHitPlayers();

private:
    void _init(Level* level, boost::shared_ptr<Entity> source, double x, double y, double z, float radius, bool fire,
               bool breaksBlocks, bool playersOnly, bool noBlockDrops, bool silent, float damageRadius,
               bool breaksBlocksUnderwater);

    bool mFire;
    bool mBreaksBlocks;
    bool mPlayersOnly;
    bool mNoBlockDrops;
    bool mSilent;
    bool mBreaksBlocksUnderwater;
    Random mRandom;
    Level* mLevel;
    double mX;
    double mY;
    double mZ;
    boost::shared_ptr<Entity> mSource;
    float mRadius;
    float mDamageRadius;
    ToBlowSet mToBlow;
    HitPlayerMap mHitPlayers;
};

#endif
