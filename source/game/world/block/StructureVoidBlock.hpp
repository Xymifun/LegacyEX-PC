#ifndef GAME_WORLD_BLOCK_STRUCTUREVOIDBLOCK_HPP
#define GAME_WORLD_BLOCK_STRUCTUREVOIDBLOCK_HPP

#include "game/world/block/Block.hpp"

class StructureVoidBlock : public Block
{
public:
    StructureVoidBlock();

    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool mayPick(const BlockState* state, bool includeLiquid);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
};

#endif
