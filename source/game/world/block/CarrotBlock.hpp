#ifndef GAME_WORLD_BLOCK_CARROTBLOCK_HPP
#define GAME_WORLD_BLOCK_CARROTBLOCK_HPP

#include "game/world/block/CropBlock.hpp"

class AABB;
class BlockPos;
class BlockState;
class Direction;
class IconRegister;
class Item;
class LevelSource;
class TextureAtlasSprite;

class CarrotBlock : public CropBlock
{
public:
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual Item* getBaseSeedId();
    virtual Item* getBasePlantId();

protected:
    TextureAtlasSprite* mStageIcons[4];
};

#endif
