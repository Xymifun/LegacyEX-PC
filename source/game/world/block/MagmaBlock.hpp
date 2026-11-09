#ifndef GAME_WORLD_BLOCK_MAGMABLOCK_HPP
#define GAME_WORLD_BLOCK_MAGMABLOCK_HPP

#include "game/world/block/Block.hpp"

#include <boost/shared_ptr.hpp>

class BlockPos;
class BlockState;
class Entity;
class Level;
class LevelSource;
class MaterialColor;
class Random;

class MagmaBlock : public Block
{
public:
    MagmaBlock();

    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity);
    virtual int getLightColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, int lightValue);
    void addToTickQueue(Level* level, const BlockPos& pos, Random* random);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual int getTickDelay(Level* level);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isValidSpawn(const BlockState* state, boost::shared_ptr<Entity> entity);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
};

#endif
