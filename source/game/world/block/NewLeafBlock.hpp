#ifndef GAME_WORLD_BLOCK_NEWLEAFBLOCK_HPP
#define GAME_WORLD_BLOCK_NEWLEAFBLOCK_HPP

#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/PlanksBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class ItemInstance;
class Level;
class MaterialColor;
class Property;
template <typename T> class AbstractProperty;

class NewLeafBlock : public LeafBlock
{
public:
    static EnumProperty<PlanksBlock::Variant*>* VARIANT;

    NewLeafBlock();

    static void blockStaticCtor();

    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual void dropExtraLoot(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus);
    virtual PlanksBlock::Variant* getWoodVariantForData(int data);
    virtual int getWoodType(const BlockState* state);
};

#endif
