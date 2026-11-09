#ifndef GAME_WORLD_BLOCK_MOBSPAWNERBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_MOBSPAWNERBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/BaseMobSpawner.hpp"
#include "game/world/block/BlockEntity.hpp"

class CompoundTag;
class DataFixerUpper;
class Level;
class Packet;
class SpawnData;

class MobSpawnerBlockEntity : public BlockEntity
{
public:
    class BlockEntityMobSpawner : public BaseMobSpawner
    {
    public:
        explicit BlockEntityMobSpawner(MobSpawnerBlockEntity* blockEntity);
        virtual void setNextSpawnData(SpawnData* data);
        virtual void broadcastEvent(int event);
        virtual Level* getLevel();
        virtual BlockPos getPos();

    private:
        MobSpawnerBlockEntity* mBlockEntity;
    };

    MobSpawnerBlockEntity();
    virtual ~MobSpawnerBlockEntity();
    virtual unsigned long long GetType();
    static BlockEntity* create();
    BlockEntityMobSpawner* getSpawner();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual void tick();
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    virtual bool triggerEvent(int id, int param);
    virtual bool onlyOpCanSetNbt();
    void setEntityId(const std::wstring& entityId);
    virtual boost::shared_ptr<BlockEntity> clone();

private:
    BlockEntityMobSpawner* mSpawner;
};

#endif
