#ifndef GAME_WORLD_BLOCK_WATERLILYBLOCK_HPP
#define GAME_WORLD_BLOCK_WATERLILYBLOCK_HPP

#include "game/core/Bush.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>

class AABB;
class BlockPos;
class BlockState;
class Entity;
class Level;
class LevelSource;

class WaterlilyBlock : public Bush
{
public:
    WaterlilyBlock();
    virtual ~WaterlilyBlock() {}

    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual int getColor() const;
    virtual int getColor(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
