#ifndef GAME_WORLD_BLOCK_BLOCKPREDICATE_HPP
#define GAME_WORLD_BLOCK_BLOCKPREDICATE_HPP

#include "game/world/block/Predicate.hpp"

class Block;
class BlockState;

class BlockPredicate : public Predicate<const BlockState*>
{
public:
    static BlockPredicate forBlock(Block* block);

    BlockPredicate(Block* block);
    virtual ~BlockPredicate();

    virtual bool apply(const BlockState* state) const;

private:
    Block* mBlock;
};

#endif
