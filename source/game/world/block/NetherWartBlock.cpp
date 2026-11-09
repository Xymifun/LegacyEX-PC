#include "game/world/block/NetherWartBlock.hpp"

#include "game/core/IconRegister.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

#include "game/core/StringConversion.hpp"

#include <string>

static AbstractProperty<int>* AGE = static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"age", 0, 3));
static const AABB* SHAPE_BY_AGE[4] = {
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.375, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.625, 1.0), AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.875, 1.0)};

NetherWartBlock::NetherWartBlock() : Bush(Material::PLANT, MaterialColor::COLOR_RED)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue<int>(AGE, 0));
    setTicking(true);
}

const AABB* NetherWartBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    return SHAPE_BY_AGE[state->getValue(AGE)];
}

bool NetherWartBlock::mayPlaceOn(Level*, const BlockPos&, const BlockState* state)
{
    return state->getBlock() == Blocks::soul_sand;
}

bool NetherWartBlock::canSurvive(Level* level, const BlockPos& pos, const BlockState*)
{
    return mayPlaceOn(level, pos.below(), level->getBlockState(pos.below()));
}

void NetherWartBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    int age = state->getValue(AGE);
    if (age < 3 && random->nextInt(10) == 0)
    {
        state = state->setValue<int>(AGE, age + 1);
        level->setBlock(pos, state, Block::UPDATE_CLIENTS, false);
    }
    Bush::tick(level, pos, state, random);
}

void NetherWartBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float, int lootBonus)
{
    if (level->mIsClientSide)
        return;

    int count = 1;
    if (state->getValue(AGE) >= 3)
    {
        count = level->mRandom->nextInt(3) + 2;
        if (lootBonus > 0)
            count += level->mRandom->nextInt(lootBonus + 1);
    }

    for (int i = 0; i < count; ++i)
        Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::netherWart)));
}

Item* NetherWartBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
}

int NetherWartBlock::getResourceCount(Random*)
{
    return 0;
}

not_null_ptr<ItemInstance> NetherWartBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::netherWart));
}

const BlockState* NetherWartBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue<int>(AGE, data);
}

int NetherWartBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(AGE);
}

bool NetherWartBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* NetherWartBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {AGE};
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* NetherWartBlock::getTexture(const Direction*, const BlockState* state)
{
    if (state->getValue(AGE) >= 3)
        return mStageIcons[2];
    if (state->getValue(AGE) > 0)
        return mStageIcons[1];
    return mStageIcons[0];
}

void NetherWartBlock::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 3; ++i)
        mStageIcons[i] = iconRegister->registerIcon(getIconName() + L"_stage_" + _toString<int>(i));
}

EBlockRenderShape NetherWartBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_ROW;
}
