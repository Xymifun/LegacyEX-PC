#ifndef GAME_WORLD_BLOCK_RAILBLOCK_HPP
#define GAME_WORLD_BLOCK_RAILBLOCK_HPP

#include "game/world/block/BaseRailBlock.hpp"

template <typename T>
class EnumProperty;

class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class Mirror;
class Rotation;
class Property;
class TextureAtlasSprite;

class RailBlock : public BaseRailBlock
{
public:
    RailBlock();
    static void blockStaticCtor();
    virtual void updateState(const BlockState* state, Level* level, const BlockPos& pos, Block* block);
    virtual const AbstractProperty<BaseRailBlock::RailShape*>* getShapeProperty();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

    static EnumProperty<BaseRailBlock::RailShape*>* SHAPE;

private:
    TextureAtlasSprite* mTurnedTexture;
};

#endif
