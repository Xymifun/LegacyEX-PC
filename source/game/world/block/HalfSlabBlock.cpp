#include "game/world/block/Predicates.hpp"
#include "game/world/block/HalfSlabBlock.hpp"
#include "game/util/AABB.hpp"

#include <typeinfo>

#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Blocks.hpp"


static const AABB* FULL_BLOCK_SHAPE = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
static const AABB* BOTTOM_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0);
static const AABB* TOP_AABB = AABB::newPermanent(0.0, 0.5, 0.0, 1.0, 1.0, 1.0);

AbstractProperty<HalfSlabBlock::Half*>* HalfSlabBlock::TYPE = NULL;
HalfSlabBlock::Half* HalfSlabBlock::TOP = new HalfSlabBlock::Half(L"top");
HalfSlabBlock::Half* HalfSlabBlock::BOTTOM = new HalfSlabBlock::Half(L"bottom");
HalfSlabBlock::Half* HalfSlabBlock::HALVES[HalfSlabBlock::Half::HALF_COUNT] = {HalfSlabBlock::TOP,
                                                                               HalfSlabBlock::BOTTOM};

HalfSlabBlock::Half::Half(const std::wstring& name) : mName(name)
{
}

std::wstring HalfSlabBlock::Half::toString() const
{
    return mName;
}

std::wstring HalfSlabBlock::Half::getSerializedName() const
{
    return mName;
}

void HalfSlabBlock::blockStaticCtor()
{
    std::wstring name = L"half";
    Predicate<Half*>* filter = Predicates<Half*>::alwaysTrue;

    AbstractProperty<Half*>* type;
    {
        boost::unordered_set<Half*> values;
        for (int i = 0; i < Half::HALF_COUNT; ++i)
        {
            values.insert(HALVES[i]);
        }

        type = EnumProperty<Half*>::create(name, typeid(Half), values, filter);
    }
    TYPE = type;
}

HalfSlabBlock::HalfSlabBlock(Material* material) : Block(material)
{
    _init();
}

HalfSlabBlock::HalfSlabBlock(Material* material, const MaterialColor* materialColor) : Block(material, materialColor)
{
    _init();
}

void HalfSlabBlock::_init()
{
    mBlockType = EBLOCK_TYPE_SLAB;

    setLightBlock(255);
}

void HalfSlabBlock::DerivedInit()
{
    Block::DerivedInit();

    mFullBlock = isFullSize();

    setLightBlock(255);
}

bool HalfSlabBlock::isSilkTouchable()
{
    return false;
}

const AABB* HalfSlabBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (isFullSize())
    {
        return FULL_BLOCK_SHAPE;
    }

    return state->getValue(TYPE) == TOP ? TOP_AABB : BOTTOM_AABB;
}

bool HalfSlabBlock::isTopSolidBlocking(const BlockState* state)
{
    return static_cast<HalfSlabBlock*>(state->getBlock())->isFullSize() || state->getValue(TYPE) == TOP;
}

EBlockFaceShape HalfSlabBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                                 const Direction* direction)
{
    HalfSlabBlock* block = (HalfSlabBlock*)state->getBlock();

    if (block->isFullSize())
    {
        return BLOCK_FACE_SOLID;
    }

    if (direction == Direction::UP && state->getValue(TYPE) == TOP)
    {
        return BLOCK_FACE_SOLID;
    }

    if (direction == Direction::DOWN && state->getValue(TYPE) == BOTTOM)
    {
        return BLOCK_FACE_SOLID;
    }

    return BLOCK_FACE_UNDEFINED;
}

bool HalfSlabBlock::isSolidRender(const BlockState* state) const
{
    return isFullSize();
}

const BlockState* HalfSlabBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                      float hitX, float hitY, float hitZ, int auxValue,
                                                      boost::shared_ptr<LivingEntity> placer)
{
    const BlockState* state =
        Block::getStateForPlacement(level, pos, direction, hitX, hitY, hitZ, auxValue, placer)->setValue(TYPE, BOTTOM);

    if (!isFullSize() && (direction == Direction::DOWN || (direction != Direction::UP && hitY > 0.5)))
    {
        return state->setValue(TYPE, TOP);
    }

    return state;
}

int HalfSlabBlock::getResourceCount(Random* random)
{
    return isFullSize() ? 2 : 1;
}

bool HalfSlabBlock::isCubeShaped(const BlockState* state)
{
    return isFullSize();
}

bool HalfSlabBlock::isHalfSlab(Block* block)
{
    return block == Blocks::stone_slab || block == Blocks::wooden_slab || block == Blocks::stone_slab2
           || block == Blocks::purpur_slab || block == Blocks::prismarine_slab;
}

bool HalfSlabBlock::isHalfSlab(const BlockState* state)
{
    return isHalfSlab(state->getBlock());
}

bool HalfSlabBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                     const Direction* direction)
{
    if (isFullSize())
    {
        return Block::shouldRenderFace(state, levelSource, pos, direction);
    }

    if (direction != Direction::UP && direction != Direction::DOWN
        && !Block::shouldRenderFace(state, levelSource, pos, direction))
    {
        return false;
    }

    const BlockState* neighbourState = levelSource->getBlockState(pos.relative(direction));

    bool topSlab = isHalfSlab(state) && state->getValue(TYPE) == TOP;

    if (isHalfSlab(neighbourState) && neighbourState->getValue(TYPE) == TOP)
    {
        if (direction == Direction::DOWN)
        {
            return true;
        }

        if (direction == Direction::UP && Block::shouldRenderFace(state, levelSource, pos, direction))
        {
            return true;
        }

        return !(isHalfSlab(state) && topSlab);
    }

    if (direction == Direction::UP
        || (direction == Direction::DOWN && Block::shouldRenderFace(state, levelSource, pos, direction)))
    {
        return true;
    }

    return !(isHalfSlab(neighbourState) && !topSlab);
}

bool HalfSlabBlock::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* state = levelSource->getBlockState(pos);

    if (!isFullSize() && state->getValue(TYPE) == BOTTOM)
    {
        return true;
    }

    return Block::isPathfindable(levelSource, pos);
}

bool HalfSlabBlock::isSlabBlock()
{
    return true;
}

bool HalfSlabBlock::isFullSlab(Block* block)
{
    return block == Blocks::purpur_double_slab || block == Blocks::double_wooden_slab
           || block == Blocks::double_stone_slab2 || block == Blocks::double_stone_slab
           || block == Blocks::prismarine_double_slab;
}

Block* HalfSlabBlock::GetDoubleSlabBlockFromSlab(Block* block)
{
    if (block == Blocks::stone_slab)
    {
        return Blocks::double_stone_slab;
    }

    if (block == Blocks::wooden_slab)
    {
        return Blocks::double_wooden_slab;
    }

    if (block == Blocks::stone_slab2)
    {
        return Blocks::double_stone_slab2;
    }

    if (block == Blocks::purpur_slab)
    {
        return Blocks::purpur_double_slab;
    }

    if (block == Blocks::prismarine_slab)
    {
        return Blocks::prismarine_double_slab;
    }

    return NULL;
}

bool HalfSlabBlock::liquidCanFlowIntoFromDirection(LevelSource* levelSource, const BlockPos& pos,
                                                   const Direction* direction) const
{
    if (direction == Direction::UP || direction == Direction::DOWN)
    {
        if (isHalfSlab(levelSource->getBlockState(pos)))
        {
            const Direction* halfFace = (levelSource->getBlockData(pos.x, pos.y, pos.z) & 8) != 0 ? Direction::UP
                                                                                                  : Direction::DOWN;

            if (direction == halfFace)
            {
                return false;
            }
        }
    }

    return true;
}

bool HalfSlabBlock::isBottomSlab(int data)
{
    return (data & 8) == 0;
}
