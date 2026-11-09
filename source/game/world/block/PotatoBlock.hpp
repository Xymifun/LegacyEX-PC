#ifndef GAME_WORLD_BLOCK_POTATOBLOCK_HPP
#define GAME_WORLD_BLOCK_POTATOBLOCK_HPP

#include "game/world/block/CropBlock.hpp"

class AABB;
class BlockPos;
class BlockState;
class Direction;
class IconRegister;
class Item;
class Level;
class LevelSource;
class TextureAtlasSprite;

class PotatoBlock : public CropBlock
{
public:
    PotatoBlock();

    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual Item* getBaseSeedId();
    virtual Item* getBasePlantId();

protected:
    TextureAtlasSprite* mStageIcons[4];
};

#endif
