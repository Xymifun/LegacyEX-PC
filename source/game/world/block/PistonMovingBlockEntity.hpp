#ifndef GAME_WORLD_BLOCK_PISTONMOVINGBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_PISTONMOVINGBLOCKENTITY_HPP
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/world/block/BlockEntity.hpp"
class AABB;
class BlockState;
class CompoundTag;
class DataFixerUpper;
class Direction;
class Entity;
class Level;
class LevelSource;
class PistonMovingBlockEntity : public BlockEntity
{
public:
    PistonMovingBlockEntity(const BlockState*, const Direction*, bool, bool, CompoundTag*);
    PistonMovingBlockEntity();
    virtual ~PistonMovingBlockEntity();
    static BlockEntity* create();
    void stop();
    void finalTick();
    const Direction* getDirection();
    bool isExtending();
    static void NOCLIP_set(const Direction*);
    static const Direction* NOCLIP_get();
    const BlockState* getBlockState();
    virtual CompoundTag* getUpdateTag();
    virtual int getData();
    bool isSourcePiston();
    float getProgress(float);
    float getExtendedProgress(float);
    float getXOff(float);
    float getYOff(float);
    float getZOff(float);
    const BlockState* getCollisionRelatedBlockState();
    AABB* getAABB(LevelSource*, const BlockPos&, float);
    AABB* getAABB(LevelSource*, const BlockPos&);
    AABB* moveByPositionAndProgress(AABB*);
    AABB* getMinMaxPiecesAABB(std::vector<AABB*>*);
    AABB* getMovementArea(AABB*, const Direction*, double);
    static double getDeltaX(AABB*, const Direction*, AABB*);
    static double getDeltaY(AABB*, const Direction*, AABB*);
    static double getDeltaZ(AABB*, const Direction*, AABB*);
    double getMovement(AABB*, const Direction*, AABB*);
    void fixEntityWithinPistonBase(boost::shared_ptr<Entity>, const Direction*, double);
    void moveCollidedEntities(float);
    virtual void tick();
    static void addDataWalkers(DataFixerUpper*);
    virtual void load(CompoundTag*);
    virtual CompoundTag* save(CompoundTag*);
    void addCollissionAABBs(Level*, const BlockPos&, const AABB*, std::vector<AABB*>*, boost::shared_ptr<Entity>);
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();

private:
    const BlockState* mMovedState;
    const Direction* mDirection;
    bool mExtending;
    bool mSourcePiston;
    CompoundTag* mBlockEntityTag;
    float mProgress;
    float mLastProgress;
};
#endif
