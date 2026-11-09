#ifndef GAME_WORLD_BLOCK_MYCELIUMBLOCK_HPP
#define GAME_WORLD_BLOCK_MYCELIUMBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;

template <class T> class AbstractProperty;

class MyceliumBlock : public Block
{
public:
    static AbstractProperty<bool>* SNOWY;

    MyceliumBlock();
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual TextureAtlasSprite* getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction,
                                           const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mSnowIcon;
};

#endif
