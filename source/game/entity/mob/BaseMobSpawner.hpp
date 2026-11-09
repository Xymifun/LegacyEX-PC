#ifndef GAME_ENTITY_MOB_BASEMOBSPAWNER_HPP
#define GAME_ENTITY_MOB_BASEMOBSPAWNER_HPP

#include <boost/shared_ptr.hpp>
#include <vector>

#include "game/core/SpawnData.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class Level;

class BaseMobSpawner
{
public:
    BaseMobSpawner();
    virtual ~BaseMobSpawner();
    virtual ResourceLocation getEntityId();
    virtual void setEntityId(const ResourceLocation& entityId);
    virtual bool isNearPlayer();
    virtual void tick();
    virtual void delay();
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual boost::shared_ptr<Entity> getOrCreateDisplayEntity(Level* level);
    virtual int onEventTriggered(int event);
    virtual void setNextSpawnData(SpawnData* data);
    virtual void broadcastEvent(int event) = 0;
    virtual Level* getLevel() = 0;
    virtual BlockPos getPos() = 0;
    bool hasMob();
    double getSpin();
    double getoSpin();

protected:
    int mDelay;
    std::vector<SpawnData*> mSpawnPotentials;
    SpawnData* mNextSpawnData;
    double mSpin;
    double moSpin;
    int mMinSpawnDelay;
    int mMaxSpawnDelay;
    int mSpawnCount;
    boost::shared_ptr<Entity> mDisplayEntity;
    int mMaxNearbyEntities;
    int mRequiredPlayerRange;
    int mSpawnRange;
};

#endif
