#ifndef GAME_WORLD_BLOCK_CACTUSBLOCK_HPP
#define GAME_WORLD_BLOCK_CACTUSBLOCK_HPP

#include "game/world/block/Block.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;

class CactusBlock : public Block
{
public:
    CactusBlock();
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const AABB* getOutlineAABB(const BlockState* state, Level* level, const BlockPos& pos);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mBottomIcon;
};

#endif
