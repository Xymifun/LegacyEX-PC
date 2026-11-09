#ifndef GAME_WORLD_BLOCK_OLDLOGBLOCK_HPP
#define GAME_WORLD_BLOCK_OLDLOGBLOCK_HPP

#include "game/world/block/LogBlock.hpp"

#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/PlanksBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class LevelSource;
class MaterialColor;
class Property;

class OldLogBlock : public LogBlock
{
public:
    static EnumProperty<PlanksBlock::Variant*>* VARIANT;

    OldLogBlock();

    static void blockStaticCtor();

    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual int getWoodType(const BlockState* state);
};


bool oldlog_filter(PlanksBlock::Variant* value);

#endif
