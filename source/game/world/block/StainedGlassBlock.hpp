#ifndef GAME_WORLD_BLOCK_STAINEDGLASSBLOCK_HPP
#define GAME_WORLD_BLOCK_STAINEDGLASSBLOCK_HPP

#include "game/world/block/HalfTransparentBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Material;
class MaterialColor;
class Random;
class TextureAtlasSprite;

class DyeColor;
template <class T> class EnumProperty;

class StainedGlassBlock : public HalfTransparentBlock
{
public:
    static EnumProperty<const DyeColor*>* COLOR;

    StainedGlassBlock(Material* material);

    static void blockStaticCtor();

    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual int getRenderLayer();
    virtual int getResourceCount(Random* random);
    virtual bool isSilkTouchable();
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isTransparent(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
};

#endif
