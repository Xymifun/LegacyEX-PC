#ifndef GAME_WORLD_BLOCK_COLOREDBLOCK_HPP
#define GAME_WORLD_BLOCK_COLOREDBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class DyeColor;
class Direction;
class IconRegister;
class LevelSource;
class Material;
class MaterialColor;
class TextureAtlasSprite;
template <typename T> class EnumProperty;

class ColoredBlock : public Block
{
public:
    ColoredBlock(Material* material);
    virtual ~ColoredBlock() {}
    static void blockStaticCtor();

    static EnumProperty<const DyeColor*>* COLOR;

    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual void registerIcons(IconRegister* iconRegister);

    TextureAtlasSprite* mIcons[16];
};

#endif
