#ifndef GAME_WORLD_BLOCK_FALLINGBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_FALLINGBLOCKENTITY_HPP

#include "game/entity/mob/Entity.hpp"
#include "game/world/block/BlockPos.hpp"

class BlockState;
class CompoundTag;
class DataFixerUpper;
class Level;

class FallingBlockEntity : public Entity
{
public:
    FallingBlockEntity(Level* level, double x, double y, double z, const BlockState* blockState);
    FallingBlockEntity(Level* level);
    virtual ~FallingBlockEntity();

    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual void tick();
    virtual bool makeStepSound();
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual bool isPickable();
    virtual bool isAttackable();
    virtual bool displayFireAnimation();
    virtual bool onlyOpCanSetNbt();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);

    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* dataFixerUpper);
    static Entity* create(Level* level);

    Level* getLevel();
    void setHurtsEntities(bool hurtsEntities);
    const BlockState* getBlock();

    BlockPos getStartPos();

    static const EntityDataAccessor<BlockPos>* DATA_START_POS;

private:
    void _init();
    void setStartPos(const BlockPos& pos);

    const BlockState* mBlockState;
    int mTime;
    bool mDropItem;
    bool mCancelDrop;
    bool mHurtsEntities;
    int mFallHurtMax;
    float mFallHurtAmount;
    CompoundTag* mBlockEntityData;
};

#endif
