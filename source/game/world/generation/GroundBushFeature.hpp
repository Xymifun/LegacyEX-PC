#ifndef GAME_WORLD_GENERATION_GROUNDBUSHFEATURE_HPP
#define GAME_WORLD_GENERATION_GROUNDBUSHFEATURE_HPP

#include "game/world/generation/TreeFeature.hpp"

class BlockState;

class GroundBushFeature : public TreeFeature
{
public:
    GroundBushFeature(const BlockState* trunkState, const BlockState* leafState);
    virtual bool place(Level* level, Random& random, const BlockPos& pos);

private:
    const BlockState* mLeafState;
    const BlockState* mTrunkState;
};

#endif
