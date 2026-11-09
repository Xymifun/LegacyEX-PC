#ifndef GAME_NETWORK_SERVERLEVELLISTENER_HPP
#define GAME_NETWORK_SERVERLEVELLISTENER_HPP

#include <boost/shared_ptr.hpp>

#include "game/network/LevelListener.hpp"

class BlockPos;
class BlockState;
class Entity;
class Level;
class MinecraftServer;
class ServerLevel;
class SoundEvent;
class Player;

class ServerLevelListener : public LevelListener
{
public:
    ServerLevelListener(MinecraftServer* server, ServerLevel* level);
    virtual ~ServerLevelListener();

    virtual void blockChanged(Level* level, const BlockPos& pos, const BlockState* oldState, const BlockState* newState,
                              int flags, bool immediate);
    virtual void blockLightChanged(const BlockPos& pos);
    virtual void setBlocksDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level);
    virtual void allChanged();
    virtual void playStreamingMusic(const SoundEvent* sound, const BlockPos& pos, bool playing);
    virtual void playSoundToAllPlayers(const SoundEvent* sound, SoundSource::ESoundSource source, double x, double y,
                                       double z, float volume, float pitch, float range);
    virtual void playSound(boost::shared_ptr<Player> player, const SoundEvent* sound, SoundSource::ESoundSource source,
                           double x, double y, double z, float volume, float pitch, float range);
    virtual void addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, double x, double y, double z,
                             double xa, double ya, double za, arrayWithLength<int> params);
    virtual void addParticle(Level* level, ePARTICLE_TYPE type, bool overrideLimiter, bool force, double x, double y,
                             double z, double xa, double ya, double za, arrayWithLength<int> params);
    virtual void entityAdded(boost::shared_ptr<Entity> entity);
    virtual void entityRemoved(boost::shared_ptr<Entity> entity);
    virtual void playerRemoved(boost::shared_ptr<Entity> player);
    virtual void globalLevelEvent(int type, const BlockPos& pos, int data);
    virtual void levelEvent(boost::shared_ptr<Player> player, int type, const BlockPos& pos, int data);
    virtual void destroyBlockProgress(int breakerId, const BlockPos& pos, int progress);

private:
    MinecraftServer* mServer;
    ServerLevel* mLevel;
};

#endif
