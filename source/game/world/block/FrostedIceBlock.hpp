#ifndef GAME_WORLD_BLOCK_FROSTEDICEBLOCK_HPP
#define GAME_WORLD_BLOCK_FROSTEDICEBLOCK_HPP

#include "game/world/block/IceBlock.hpp"

class Block;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class Random;
class TextureAtlasSprite;

class FrostedIceBlock : public IceBlock
{
public:
    FrostedIceBlock();

    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    int countNeighbours(Level* level, const BlockPos& pos);
    void slightlyMelt(Level* level, const BlockPos& pos, const BlockState* state, Random* random, bool meltNeighbors);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isSilkTouchable();

private:
    TextureAtlasSprite* mIcon1;
    TextureAtlasSprite* mIcon2;
    TextureAtlasSprite* mIcon3;
};

#endif
