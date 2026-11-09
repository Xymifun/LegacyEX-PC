#ifndef GAME_WORLD_BLOCK_SLIMEBLOCK_HPP
#define GAME_WORLD_BLOCK_SLIMEBLOCK_HPP

#include "game/world/block/HalfTransparentBlock.hpp"

class BlockState;
class BlockPos;
class Entity;
class Level;

class SlimeBlock : public HalfTransparentBlock
{
public:
    SlimeBlock();
    virtual int getRenderLayer();
    virtual void fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance);
    virtual void updateEntityAfterFallOn(Level* level, boost::shared_ptr<Entity> entity);
    virtual void stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
