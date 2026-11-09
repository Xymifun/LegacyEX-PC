#ifndef GAME_WORLD_BLOCK_POWEREDRAILBLOCK_HPP
#define GAME_WORLD_BLOCK_POWEREDRAILBLOCK_HPP

#include "game/world/block/BaseRailBlock.hpp"

template <typename T>
class EnumProperty;

class BlockState;
class BooleanProperty;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class Mirror;
class Rotation;
class TextureAtlasSprite;

class PoweredRailBlock : public BaseRailBlock
{
public:
    PoweredRailBlock();
    static void blockStaticCtor();
    virtual bool findPoweredRailSignal(Level* level, const BlockPos& pos, const BlockState* state, bool forward,
                                       int distance);
    virtual bool isSameRailWithPower(Level* level, const BlockPos& pos, bool forward, int distance,
                                     BaseRailBlock::RailShape* shape);
    virtual void updateState(const BlockState* state, Level* level, const BlockPos& pos, Block* block);
    virtual const AbstractProperty<BaseRailBlock::RailShape*>* getShapeProperty();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

    static EnumProperty<BaseRailBlock::RailShape*>* SHAPE;
    static BooleanProperty* POWERED;

private:
    TextureAtlasSprite* mPoweredTexture;
};

#endif
