#ifndef GAME_WORLD_BLOCK_BEETROOTBLOCK_HPP
#define GAME_WORLD_BLOCK_BEETROOTBLOCK_HPP

#include "game/world/block/CropBlock.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Item;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;

class BeetrootBlock : public CropBlock
{
public:
    static AbstractProperty<int>* AGE;

    BeetrootBlock();
    virtual AbstractProperty<int>* getAgeProperty();
    virtual int getMaxAge();
    virtual Item* getBaseSeedId();
    virtual Item* getBasePlantId();
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual int getBonemealAgeIncrease(Level* level);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

protected:
    TextureAtlasSprite* mStageIcons[4];
};

#endif
