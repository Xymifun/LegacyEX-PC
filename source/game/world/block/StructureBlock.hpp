#ifndef GAME_WORLD_BLOCK_STRUCTUREBLOCK_HPP
#define GAME_WORLD_BLOCK_STRUCTUREBLOCK_HPP

#include "game/world/block/Block.hpp"

class StructureBlock : public Block
{
public:
    StructureBlock();

    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool mayPick(const BlockState* state, bool includeLiquid);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
};

#endif
