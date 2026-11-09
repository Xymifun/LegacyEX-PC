#ifndef GAME_WORLD_GENERATION_MULTIPLAYERLEVEL_HPP
#define GAME_WORLD_GENERATION_MULTIPLAYERLEVEL_HPP

#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "game/client/audio/SoundSource.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"

class BlockState;
class ClientChunkCache;
class ClientPacketListener;
class ClientScoreboard;
class ClientboundBlockEntityDataPacket;
class CompoundTag;
class Difficulty;
class Entity;
class LevelSettings;
class LevelStorage;
class Minecraft;
class Minecart;
class Packet;
class Player;
class Scoreboard;
class SoundEvent;
class SoundInstance;

class MultiPlayerLevel : public Level
{
public:
    class ResetInfo
    {
    public:
        ResetInfo(const BlockPos& pos, const BlockState* state);

        BlockPos pos;
        int delay;
        const BlockState* state;
    };

    MultiPlayerLevel(boost::shared_ptr<LevelStorage> levelStorage, ClientPacketListener* connection,
                     LevelSettings* settings, int dimension, const Difficulty* difficulty, int seaLevel);
    MultiPlayerLevel(ClientPacketListener* connection, LevelSettings* settings, int dimension,
                     const Difficulty* difficulty, int seaLevel);
    virtual ~MultiPlayerLevel();

    void removeClientPacketListener(ClientPacketListener* connection, bool sendDisconnect);
    ClientScoreboard* GetClientScoreboard();
    void animateTick(int x, int y, int z);
    void animateTickDoWork();
    void removeAllPendingEntityRemovals();
    void playLocalSound(const BlockPos& pos, const SoundEvent* sound, SoundSource::ESoundSource source, float volume,
                        float pitch, bool distanceDelay, float range);
    void MultiPlayerLevelInit(ClientPacketListener* connection, LevelSettings* settings, const Difficulty* difficulty,
                              int seaLevel);
    virtual void tick();
    void clearResetRegion(int minX, int minY, int minZ, int maxX, int maxY, int maxZ);
    virtual ChunkSource* createChunkSource();
    ChunkSource* getChunkSource();
    virtual bool hasChunk(int x, int z, bool allowEmpty);
    virtual bool hasChunksAt(int minX, int minY, int minZ, int maxX, int maxY, int maxZ, bool allowEmpty);
    virtual LevelChunk* getChunkIfHasChunk(int x, int z, bool allowEmpty);
    virtual void validateSpawn();
    virtual void tickBlocks();
    void ReloadChunk(int x, int z, const std::vector<LevelChunk::BlockChange>* changes, bool replace);
    void setChunkVisible(int x, int z, bool visible, bool force);
    virtual bool addEntity(boost::shared_ptr<Entity> entity);
    virtual void removeEntity(boost::shared_ptr<Entity> entity);
    virtual void entityAdded(boost::shared_ptr<Entity> entity);
    virtual void entityRemoved(boost::shared_ptr<Entity> entity);
    void putEntity(int id, boost::shared_ptr<Entity> entity);
    virtual boost::shared_ptr<Entity> getEntity(int id);
    boost::shared_ptr<Entity> removeEntity(int id);
    virtual void removeEntities(std::vector<boost::shared_ptr<Entity> >* entities);
    virtual bool setBlock(const BlockPos& pos, const BlockState* state, int flags, bool forceUpdate);
    bool doSetBlockAndData(const BlockPos& pos, const BlockState* state, const BlockState* extraState);
    virtual void disconnect(bool save);
    virtual SoundInstance* makeSoundUpdater(boost::shared_ptr<Minecart> minecart);
    virtual void tickWeather(bool blockEvents);
    virtual int getChunkPollRange();
    virtual void tickClientSideBlocks(int x, int z, LevelChunk* chunk);
    virtual void playSound(boost::shared_ptr<Player> player, double x, double y, double z, const SoundEvent* sound,
                           SoundSource::ESoundSource source, float volume, float pitch, float range);
    virtual void playLocalSound(double x, double y, double z, const SoundEvent* sound, SoundSource::ESoundSource source,
                                float volume, float pitch, bool distanceDelay, float range);
    virtual void createFireworks(double x, double y, double z, double xa, double ya, double za, CompoundTag* fireworks);
    virtual void sendPacketToServer(boost::shared_ptr<Packet> packet);
    virtual void setDayTime(long long time);
    void dataReceivedForChunk(int x, int z);
    void removeUnusedBlockEntitiesInRegion(int minX, int minY, int minZ, int maxX, int maxY, int maxZ);
    virtual Scoreboard* GetScoreboard();
    void DropChunksOutsideArea(int minX, int minZ, int maxX, int maxZ, bool force);
    void StopReloadChunk(int x, int z);
    bool MustQueueBlockEntityDataPackets();
    void QueueBlockEntityDataPacket(const boost::shared_ptr<ClientboundBlockEntityDataPacket>& packet);
    void BlockUntilSafeToChangeSaveData(bool block);
    void tickAllConnections();

private:
    friend class Minecraft;
    friend class ClientPacketListener;

    typedef boost::unordered_map<int, boost::shared_ptr<Entity>, IntKeyHash2, IntKeyEq> EntityMap;
    typedef boost::unordered_set<boost::shared_ptr<Entity> > EntitySet;

    std::vector<ResetInfo> mResetInfos;
    bool mAnimateTickReady;
    int mAnimateTickX;
    int mAnimateTickY;
    int mChunkTickX;
    int mChunkTickZ;
    std::vector<ClientPacketListener*> mConnections;
    ClientChunkCache* mClientChunkCache;
    Minecraft* mMinecraft;
    ClientScoreboard* mClientScoreboard;
    EntityMap mEntitiesById;
    EntitySet mTrackedEntities;
    EntitySet mPendingEntityAdditions;
    boost::unordered_set<int> mAnimatedSections;
    int mAmbientSoundTime;
};

#endif
