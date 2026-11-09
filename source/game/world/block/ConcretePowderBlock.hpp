#ifndef GAME_WORLD_BLOCK_CONCRETEPOWDERBLOCK_HPP
#define GAME_WORLD_BLOCK_CONCRETEPOWDERBLOCK_HPP

#include "game/world/block/FallingBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class DyeColor;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class TextureAtlasSprite;
template <typename T> class EnumProperty;

class ConcretePowderBlock : public FallingBlock
{
public:
    ConcretePowderBlock();
    static void blockStaticCtor();

    static EnumProperty<const DyeColor*>* COLOR;

    virtual void onLand(Level* level, const BlockPos& pos, const BlockState* state, const BlockState* replacedState);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    unsigned int tryTouchWater(Level* level, const BlockPos& pos, const BlockState* state);
    TextureAtlasSprite* mIcons[16];
};

#endif
