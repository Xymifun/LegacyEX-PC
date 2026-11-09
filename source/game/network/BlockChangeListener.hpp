#ifndef GAME_NETWORK_BLOCKCHANGELISTENER_HPP
#define GAME_NETWORK_BLOCKCHANGELISTENER_HPP

#include <vector>
#include <boost/unordered_map.hpp>

#include "game/network/IChunkHistory.hpp"
#include "game/network/LevelListener.hpp"

class BlockPos;
class BlockState;
class Level;
class ServerLevel;
class SoundEvent;
class Entity;
class Player;

class BlockChangeListener : public LevelListener, public IChunkHistory
{
public:
    class Chunk
    {
    public:
        Chunk(int x, int z);
        std::vector<short>& GetChanges();
        int GetChangeCount();
        bool HasBlockChange(short blockKey);
        void AddBlockChange(const BlockPos& pos);
        int GetX() const;
        int GetZ() const;

    private:
        int mChangeCount;
        std::vector<short> mChanges;
        int mX;
        int mZ;
    };

    BlockChangeListener(ServerLevel* level);
    virtual ~BlockChangeListener();
    static long long GetChunkKey(int x, int z);
    bool HasChunk(long long key) const;
    Chunk& GetChunk(long long key) const;
    Chunk& GetChunk(long long key);
    virtual std::vector<short>* GetChangesForChunk(int x, int z) const;
    virtual bool ShouldLoadOnClient(int x, int z) const;
    Chunk& GetChunk(int x, int z, bool create);
    static int GetBlockKey(const BlockPos& pos);
    virtual void blockChanged(Level* level, const BlockPos& pos, const BlockState* oldState, const BlockState* newState,
                              int flags, bool immediate);
    virtual void blockLightChanged(const BlockPos& pos);
    virtual void setBlocksDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, Level* level);
    virtual void allChanged();
    virtual void playStreamingMusic(const SoundEvent* sound, const BlockPos& pos, bool playing);
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
    virtual void invalidateChunksOutsideArea(int minChunkX, int minChunkZ, int maxChunkX, int maxChunkZ);

private:
    ServerLevel* mLevel;
    boost::unordered_map<long long, Chunk>* mChunks;
};

#endif
