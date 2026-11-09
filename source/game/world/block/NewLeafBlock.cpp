#include "game/world/block/NewLeafBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/GenericStats.hpp"
#include "game/core/Stats.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/util/Random.hpp"

EnumProperty<PlanksBlock::Variant*>* NewLeafBlock::VARIANT = NULL;

bool leaf_variant_predicate(PlanksBlock::Variant* value);

bool leaf_variant_predicate(PlanksBlock::Variant* value)
{
    return value->getData() >= 4;
}

void NewLeafBlock::blockStaticCtor()
{
    std::wstring name(L"variant");
    Predicates<PlanksBlock::Variant*>::FunctionPredicate* filter =
        new Predicates<PlanksBlock::Variant*>::FunctionPredicate(leaf_variant_predicate);
    EnumProperty<PlanksBlock::Variant*>* variant;
    {
        boost::unordered_set<PlanksBlock::Variant*> values;
        for (int i = 0; i < 6; ++i)
        {
            values.insert(PlanksBlock::Variant::BY_DATA[i]);
        }
        variant = EnumProperty<PlanksBlock::Variant*>::create(name, typeid(PlanksBlock::Variant), values, filter);
    }
    VARIANT = variant;
}

NewLeafBlock::NewLeafBlock() : LeafBlock()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(VARIANT, PlanksBlock::Variant::ACACIA)
                             ->setValue(CHECK_DECAY, true)
                             ->setValue(DECAYABLE, true));
}

void NewLeafBlock::dropExtraLoot(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus)
{
    if (state->getValue(VARIANT) == PlanksBlock::Variant::DARK_OAK)
    {
        if (level->mRandom->nextInt(lootBonus) == 0)
            Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::apple, 1, 0)));
    }
}

int NewLeafBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

not_null_ptr<ItemInstance> NewLeafBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState* state)
{
    return not_null_ptr<ItemInstance>(
        new ItemInstance(this, 1, state->getBlock()->convertBlockStateToLegacyData(state) & 3));
}

not_null_ptr<ItemInstance> NewLeafBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(
        new ItemInstance(Item::byBlock(this), 1, state->getValue(VARIANT)->getData() - 4));
}

const BlockState* NewLeafBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    return state->setValue(VARIANT, getWoodVariantForData(data))
        ->setValue(DECAYABLE, (data & 4) == 0)
        ->setValue(CHECK_DECAY, (data & 8) > 0);
}

int NewLeafBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData() - 4;
    bool decayable = state->getValue(DECAYABLE);
    if (!decayable)
        data |= 4;
    bool checkDecay = state->getValue(CHECK_DECAY);
    if (checkDecay)
        data |= 8;
    return data;
}

PlanksBlock::Variant* NewLeafBlock::getWoodVariantForData(int data)
{
    return PlanksBlock::Variant::byData((data & 3) + 4);
}

BlockStateDefinition* NewLeafBlock::createBlockStateDefinition()
{
    static const Property* properties[3] = {VARIANT, CHECK_DECAY, DECAYABLE};
    return new BlockStateDefinition(this, properties, 3);
}

void NewLeafBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                 const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                 not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears)
    {
        Player* miner = player.operator->();
        Stat* stat = gStats->get_blocksMined(Block::getId(this));
        miner->awardStat(stat, GenericStats::param_blocksMined(state, 1));
        Block::popResource(level, pos,
                           not_null_ptr<ItemInstance>(
                               new ItemInstance(Item::byBlock(this), 1, state->getValue(VARIANT)->getData() - 4)));
    }
    else
    {
        Block::playerDestroy(level, player, pos, state, blockEntity, tool);
    }
}

int NewLeafBlock::getWoodType(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}
