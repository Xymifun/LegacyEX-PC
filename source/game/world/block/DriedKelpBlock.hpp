#ifndef GAME_WORLD_BLOCK_DRIEDKELPBLOCK_HPP
#define GAME_WORLD_BLOCK_DRIEDKELPBLOCK_HPP

#include "game/world/block/Block.hpp"

class DriedKelpBlock : public Block
{
public:
    DriedKelpBlock(Material* material);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);

private:
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mSideIcons[2];
};

#endif
