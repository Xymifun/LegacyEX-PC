#ifndef GAME_WORLD_BLOCK_FALLINGBLOCK_HPP
#define GAME_WORLD_BLOCK_FALLINGBLOCK_HPP

#include "game/world/block/Block.hpp"

#include <boost/shared_ptr.hpp>

class FallingBlockEntity;

class FallingBlock : public Block
{
public:
    static bool fallInstantly;

    FallingBlock();
    FallingBlock(Material* material);
    virtual ~FallingBlock() {}

    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual int getTickDelay(Level* level);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void falling(boost::shared_ptr<FallingBlockEntity> entity);
    virtual void onLand(Level* level, const BlockPos& pos, const BlockState* state, const BlockState* replacedState);
    virtual void onBroken(Level* level, const BlockPos& pos);
    virtual unsigned int getDustColor(const BlockState* state);

    static bool isFree(const BlockState* state);
    void checkSlide(Level* level, const BlockPos& pos);
};

#endif
