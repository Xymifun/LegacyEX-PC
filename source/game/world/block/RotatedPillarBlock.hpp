#ifndef GAME_WORLD_BLOCK_ROTATEDPILLARBLOCK_HPP
#define GAME_WORLD_BLOCK_ROTATEDPILLARBLOCK_HPP

#include "game/core/Direction.hpp"
#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class Level;
class Material;
class MaterialColor;
class Rotation;
class TextureAtlasSprite;
class LivingEntity;

template <class T> class EnumProperty;

class RotatedPillarBlock : public Block
{
public:
    RotatedPillarBlock(Material* material);
    RotatedPillarBlock(Material* material, const MaterialColor* materialColor);
    virtual ~RotatedPillarBlock() {}

    static void blockStaticCtor();
    static bool isOfThisClass(int data);

    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual TextureAtlasSprite* getTypeTexture(const BlockState* state);
    virtual TextureAtlasSprite* getTopTexture(const BlockState* state);
    virtual Direction::Axis* getAxis(const BlockState* state) = 0;
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

protected:
    TextureAtlasSprite* mTopTexture;
};

extern EnumProperty<Direction::Axis*>* gRotatedPillarBlockAxisProperty;

#endif
