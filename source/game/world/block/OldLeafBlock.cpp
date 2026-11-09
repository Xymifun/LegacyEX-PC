#include "game/world/block/OldLeafBlock.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/FoliageColor.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

bool oldleaf_filter(PlanksBlock::Variant* value)
{
    return value->getData() < 4;
}

EnumProperty<PlanksBlock::Variant*>* OldLeafBlock::VARIANT = NULL;

void OldLeafBlock::blockStaticCtor()
{
    std::wstring name(L"variant");
    Predicates<PlanksBlock::Variant*>::FunctionPredicate* filter =
        new Predicates<PlanksBlock::Variant*>::FunctionPredicate(oldleaf_filter);
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

OldLeafBlock::OldLeafBlock() : LeafBlock()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(VARIANT, PlanksBlock::Variant::OAK)
                             ->setValue(CHECK_DECAY, true)
                             ->setValue(DECAYABLE, true));
}

int OldLeafBlock::getColor(const BlockState* state)
{
    if (state->getBlock() == this)
    {
        PlanksBlock::Variant* variant = state->getValue(VARIANT);
        if (variant == PlanksBlock::Variant::SPRUCE)
            return FoliageColor::getEvergreenColor();
        if (variant == PlanksBlock::Variant::BIRCH)
            return FoliageColor::getBirchColor();
    }
    return LeafBlock::getColor(state);
}

int OldLeafBlock::getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex)
{
    const BlockState* state = levelSource->getBlockState(pos);
    if (state->getBlock() == this)
    {
        PlanksBlock::Variant* variant = state->getValue(VARIANT);
        if (variant == PlanksBlock::Variant::SPRUCE)
            return FoliageColor::getEvergreenColor();
        if (variant == PlanksBlock::Variant::BIRCH)
            return FoliageColor::getBirchColor();
    }
    return LeafBlock::getColor(levelSource, pos, tintIndex);
}

void OldLeafBlock::dropExtraLoot(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus)
{
    if (state->getValue(VARIANT) != PlanksBlock::Variant::OAK)
        return;
    if (level->mRandom->nextInt(lootBonus) != 0)
        return;
    Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::apple, 1, 0)));
}

int OldLeafBlock::getLootDropChance(const BlockState* state)
{
    if (state->getValue(VARIANT) == PlanksBlock::Variant::JUNGLE)
        return 40;
    return LeafBlock::getLootDropChance(state);
}

not_null_ptr<ItemInstance> OldLeafBlock::getSilkTouchItemInstance(const BlockState* state)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1, state->getValue(VARIANT)->getData()));
}

const BlockState* OldLeafBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(VARIANT, getWoodVariantForData(data))
        ->setValue(DECAYABLE, (data & 4) == 0)
        ->setValue(CHECK_DECAY, (data & 8) > 0);
}

int OldLeafBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(VARIANT)->getData();
    if (!state->getValue(DECAYABLE))
        data |= 4;
    if (state->getValue(CHECK_DECAY))
        data |= 8;
    return data;
}

PlanksBlock::Variant* OldLeafBlock::getWoodVariantForData(int data)
{
    return PlanksBlock::Variant::byData(data & 3);
}

BlockStateDefinition* OldLeafBlock::createBlockStateDefinition()
{
    static const Property* properties[3] = {VARIANT, CHECK_DECAY, DECAYABLE};
    return new BlockStateDefinition(this, properties, 3);
}

int OldLeafBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

void OldLeafBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                 const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                                 not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears)
    {
        Stat* stat = gStats->get_blocksMined(Block::getId(this));
        player->awardStat(stat, GenericStats::param_blocksMined(state, 1));
        Block::popResource(
            level, pos,
            not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1, state->getValue(VARIANT)->getData())));
    }
    else
    {
        Block::playerDestroy(level, player, pos, state, blockEntity, tool);
    }
}

int OldLeafBlock::getWoodType(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}
