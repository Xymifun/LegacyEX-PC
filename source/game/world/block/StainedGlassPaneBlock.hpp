#ifndef GAME_WORLD_BLOCK_STAINEDGLASSPANEBLOCK_HPP
#define GAME_WORLD_BLOCK_STAINEDGLASSPANEBLOCK_HPP

#include "game/world/block/ThinFenceBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class Mirror;
class Rotation;
class TextureAtlasSprite;

class DyeColor;
template <class T> class EnumProperty;

class StainedGlassPaneBlock : public ThinFenceBlock
{
public:
    static EnumProperty<const DyeColor*>* COLOR;

    StainedGlassPaneBlock();

    static void blockStaticCtor();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual TextureAtlasSprite* getIconTexture(const Direction* direction, const BlockState* state);
    virtual TextureAtlasSprite* getEdgeTexture(const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    TextureAtlasSprite* getEdgeTexture();
    virtual int getRenderLayer();
};

#endif
