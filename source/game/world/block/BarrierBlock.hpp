#ifndef GAME_WORLD_BLOCK_BARRIERBLOCK_HPP
#define GAME_WORLD_BLOCK_BARRIERBLOCK_HPP

#include "game/world/block/Block.hpp"

class BarrierBlock : public Block
{
public:
    BarrierBlock();

    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual float getShadeBrightness(const BlockState* state);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
};

#endif
