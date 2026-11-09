#ifndef GAME_CORE_BUSH_HPP
#define GAME_CORE_BUSH_HPP

#include "game/world/block/Block.hpp"

class Bush : public Block
{
public:
    Bush();
    Bush(Material* material);
    Bush(Material* material, const MaterialColor* materialColor);

    virtual ~Bush() {}
    virtual bool isCubeShaped(const BlockState* state);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual int getRenderLayer();
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool waterSpreadCausesSpawn();
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void checkAlive(Level* level, const BlockPos& pos, const BlockState* state);

private:
    void _init();
};

#endif
