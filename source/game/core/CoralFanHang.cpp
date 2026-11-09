#include "game/core/CoralFanHang.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/CoralFan.hpp"
#include "game/core/Direction.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<int>* CoralFanHang::VARIANT = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"variantHang", 0, 1));
AbstractProperty<int>* CoralFanHang::DIRECTION = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"directionHang", 0, 3));
AbstractProperty<bool>* CoralFanHang::DEAD = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"dead_coral"));

namespace
{
const AABB* CORAL_FAN_HANG_SHAPE = AABB::newPermanent(0.19999998807907104, 0.25, 0.19999998807907104, 0.8f, 0.85f,
                                                      0.8f);
}

CoralFanHang::CoralFanHang() : CoralFan()
{
    ClearDerivedInit();
    DerivedInit();

    registerDefaultState(defaultBlockState()
                             ->setValue(CoralFanHang::VARIANT, 0)
                             ->setValue(CoralFanHang::DIRECTION, 2)
                             ->setValue(CoralFanHang::DEAD, false));
}

Item* CoralFanHang::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return NULL;
}

const AABB* CoralFanHang::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return CORAL_FAN_HANG_SHAPE;
}

const Direction* CoralFanHang::_getNeighborFromDirection(int direction) const
{
    if (direction == 0)
    {
        return Direction::WEST;
    }

    if (direction == 1)
    {
        return Direction::EAST;
    }

    if (direction == 2)
    {
        return Direction::NORTH;
    }

    return Direction::SOUTH;
}

void CoralFanHang::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                   const BlockPos& fromPos)
{
    if (level->mIsClientSide)
    {
        return;
    }

    const Direction* facing = _getNeighborFromDirection(level->getBlockState(pos)->getValue(CoralFanHang::DIRECTION));
    BlockPos support = pos.relative(facing->getOpposite());

    if (!level->getBlockState(support)->canProvideSupport(facing, BLOCK_SUPPORT_CENTER))
    {
        level->addToTickNextTick(pos, this, 1);
        return;
    }

    int delay = level->mRandom->nextInt(40, 200);
    level->addToTickNextTick(pos, this, delay);
}

TextureAtlasSprite* CoralFanHang::getTexture(const Direction* direction, const BlockState* state)
{
    int variant = state->getValue(CoralFanHang::VARIANT);

    if (this == Blocks::coral_fan_hang2)
    {
        variant += 2;
    }

    if (this == Blocks::coral_fan_hang3)
    {
        variant = 4;
    }

    if (state->getValue(CoralFanHang::DEAD))
    {
        return mDeadIcons[variant];
    }

    return mIcons[variant];
}

int CoralFanHang::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(CoralFanHang::VARIANT) << 1;
    data |= state->getValue(CoralFanHang::DIRECTION) << 2;
    return data | state->getValue(CoralFanHang::DEAD);
}

const BlockState* CoralFanHang::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(CoralFanHang::VARIANT, (data >> 1) & 1)
        ->setValue(CoralFanHang::DIRECTION, (data >> 2) & 3)
        ->setValue(CoralFanHang::DEAD, (data & 1) > 0);
}

not_null_ptr<ItemInstance> CoralFanHang::getSilkTouchItemInstance(const BlockState* state)
{
    Block* fanBlock = state->getValue(CoralFanHang::DEAD) ? Blocks::coral_fan_dead : Blocks::coral_fan;
    const BlockState* fanState = fanBlock->defaultBlockState();
    fanState = fanState->setValue(CoralFan::DIRECTION, 0);

    if (state->getBlock() == Blocks::coral_fan_hang)
    {
        fanState = fanState->setValue(CoralFan::VARIANT, state->getValue(CoralFanHang::VARIANT));
    }
    else if (state->getBlock() == Blocks::coral_fan_hang2)
    {
        if (state->getValue(CoralFanHang::VARIANT) == 0)
        {
            fanState = fanState->setValue(CoralFan::VARIANT, 2);
        }
        else
        {
            fanState = fanState->setValue(CoralFan::VARIANT, 3);
        }
    }
    else
    {
        fanState = fanState->setValue(CoralFan::VARIANT, 4);
    }

    return not_null_ptr<ItemInstance>(
        new ItemInstance(fanState->getBlock(), 1, Blocks::coral_fan->convertBlockStateToLegacyData(fanState)));
}

not_null_ptr<ItemInstance> CoralFanHang::getCloneItemInstance(Level* level, const BlockPos& pos,
                                                              const BlockState* state)
{
    Block* fanBlock = state->getValue(CoralFanHang::DEAD) ? Blocks::coral_fan_dead : Blocks::coral_fan;
    const BlockState* fanState = fanBlock->defaultBlockState();
    fanState = fanState->setValue(CoralFan::DIRECTION, 0);

    if (state->getBlock() == Blocks::coral_fan_hang)
    {
        fanState = fanState->setValue(CoralFan::VARIANT, state->getValue(CoralFanHang::VARIANT));
    }
    else if (state->getBlock() == Blocks::coral_fan_hang2)
    {
        if (state->getValue(CoralFanHang::VARIANT) == 0)
        {
            fanState = fanState->setValue(CoralFan::VARIANT, 2);
        }
        else
        {
            fanState = fanState->setValue(CoralFan::VARIANT, 3);
        }
    }
    else
    {
        fanState = fanState->setValue(CoralFan::VARIANT, 4);
    }

    return not_null_ptr<ItemInstance>(
        new ItemInstance(fanState->getBlock(), 1, Blocks::coral_fan->convertBlockStateToLegacyData(fanState)));
}

BlockStateDefinition* CoralFanHang::createBlockStateDefinition()
{
    static const Property* PROPERTIES[3] = {CoralFanHang::VARIANT, CoralFanHang::DIRECTION, CoralFanHang::DEAD};

    return new BlockStateDefinition(this, PROPERTIES, 3);
}

void CoralFanHang::checkAlive(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* facing = _getNeighborFromDirection(state->getValue(CoralFanHang::DIRECTION));
    BlockPos support = pos.relative(facing->getOpposite());

    if (!level->isUnderWater(pos, Block::getId(this), 0))
    {
        state = state->setValue(CoralFanHang::DEAD, true);
        level->setBlock(pos, state, Block::UPDATE_ALL, false);
    }

    if (!level->isSolidBlockingBlock(support) && !level->getBlock(support)->isSlabBlock())
    {
        level->setBlockAndData(pos.x, pos.y, pos.z, BlockId::AIR, 0, Block::UPDATE_ALL);
    }
}

bool CoralFanHang::mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state)
{
    const BlockState* support = level->getBlockState(pos);

    if (support->canProvideSupport(Direction::NORTH, BLOCK_SUPPORT_CENTER))
    {
        return true;
    }

    if (HalfSlabBlock::isHalfSlab(support))
    {
        if (support->getValue(HalfSlabBlock::TYPE) == HalfSlabBlock::TOP)
        {
            return true;
        }
    }

    return false;
}

void CoralFanHang::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!state->getValue(CoralFanHang::DEAD) && level->getLiquidBlock(pos)->mMaterial == mMaterial)
    {
        level->setExtraBlock(pos, Blocks::flowing_water->defaultBlockState(), Block::UPDATE_ALL, false);
    }
}
