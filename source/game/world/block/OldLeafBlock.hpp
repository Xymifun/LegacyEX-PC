#ifndef GAME_WORLD_BLOCK_OLDLEAFBLOCK_HPP
#define GAME_WORLD_BLOCK_OLDLEAFBLOCK_HPP

#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/PlanksBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Level;
class LevelSource;
class Property;

class OldLeafBlock : public LeafBlock
{
public:
    static EnumProperty<PlanksBlock::Variant*>* VARIANT;
    OldLeafBlock();
    static void blockStaticCtor();
    virtual int getColor(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex);
    virtual void dropExtraLoot(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus);
    virtual int getLootDropChance(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual PlanksBlock::Variant* getWoodVariantForData(int data);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual int getWoodType(const BlockState* state);
};

bool oldleaf_filter(PlanksBlock::Variant* value);

#endif
