#ifndef GAME_NETWORK_LEVELLISTENER_HPP
#define GAME_NETWORK_LEVELLISTENER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundSource.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class BlockPos;
class BlockState;
class Entity;
class Level;
class Player;
class SoundEvent;

class LevelListener
{
public:
    virtual ~LevelListener() {}

    virtual void blockChanged(Level* level, const BlockPos& pos, const BlockState* oldState, const BlockState* newState,
                              int flags, bool immediate) = 0;
    virtual void blockLightChanged(const BlockPos& pos) = 0;
    virtual void setBlocksDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level) = 0;
    virtual void allChanged() = 0;
    virtual void playStreamingMusic(const SoundEvent* sound, const BlockPos& pos, bool playing) = 0;
    virtual void playSoundToAllPlayers(const SoundEvent* sound, SoundSource::ESoundSource source, double x, double y,
                                       double z, float volume, float pitch, float range);
    virtual void playSound(boost::shared_ptr<Player> player, const SoundEvent* sound, SoundSource::ESoundSource source,
                           double x, double y, double z, float volume, float pitch, float range) = 0;
    virtual void addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z,
                             double xa, double ya, double za, arrayWithLength<int> params) = 0;
    virtual void addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, bool force, double x, double y,
                             double z, double xa, double ya, double za, arrayWithLength<int> params) = 0;
    virtual void entityAdded(boost::shared_ptr<Entity> entity) = 0;
    virtual void entityRemoved(boost::shared_ptr<Entity> entity) = 0;
    virtual void playerRemoved(boost::shared_ptr<Entity> player) = 0;
    virtual void globalLevelEvent(int type, const BlockPos& pos, int data) = 0;
    virtual void levelEvent(boost::shared_ptr<Player> player, int type, const BlockPos& pos, int data) = 0;
    virtual void destroyBlockProgress(int breakerId, const BlockPos& pos, int progress) = 0;
    virtual void invalidateChunksOutsideArea(int minChunkX, int minChunkZ, int maxChunkX, int maxChunkZ);
};

#endif
