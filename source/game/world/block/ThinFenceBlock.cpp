#include "game/world/block/ThinFenceBlock.hpp"
#include "game/item/Items.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/LeafBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* THIN_FENCE_NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
AbstractProperty<bool>* THIN_FENCE_EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
AbstractProperty<bool>* THIN_FENCE_SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
AbstractProperty<bool>* THIN_FENCE_WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));

static const double THIN_FENCE_POST_MIN = 0.375;
static const double THIN_FENCE_POST_MAX = 0.625;

static const AABB* thinFenceShapes[16] = {
    AABB::newPermanent(THIN_FENCE_POST_MIN, 0.0, THIN_FENCE_POST_MIN, THIN_FENCE_POST_MAX, 1.0, THIN_FENCE_POST_MAX),
    AABB::newPermanent(THIN_FENCE_POST_MIN, 0.0, THIN_FENCE_POST_MIN, THIN_FENCE_POST_MAX, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, THIN_FENCE_POST_MIN, THIN_FENCE_POST_MAX, 1.0, THIN_FENCE_POST_MAX),
    AABB::newPermanent(0.0, 0.0, THIN_FENCE_POST_MIN, THIN_FENCE_POST_MAX, 1.0, 1.0),
    AABB::newPermanent(THIN_FENCE_POST_MIN, 0.0, 0.0, THIN_FENCE_POST_MAX, 1.0, THIN_FENCE_POST_MAX),
    AABB::newPermanent(THIN_FENCE_POST_MIN, 0.0, 0.0, THIN_FENCE_POST_MAX, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, THIN_FENCE_POST_MAX, 1.0, THIN_FENCE_POST_MAX),
    AABB::newPermanent(0.0, 0.0, 0.0, THIN_FENCE_POST_MAX, 1.0, 1.0),
    AABB::newPermanent(THIN_FENCE_POST_MIN, 0.0, THIN_FENCE_POST_MIN, 1.0, 1.0, THIN_FENCE_POST_MAX),
    AABB::newPermanent(THIN_FENCE_POST_MIN, 0.0, THIN_FENCE_POST_MIN, 1.0, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, THIN_FENCE_POST_MIN, 1.0, 1.0, THIN_FENCE_POST_MAX),
    AABB::newPermanent(0.0, 0.0, THIN_FENCE_POST_MIN, 1.0, 1.0, 1.0),
    AABB::newPermanent(THIN_FENCE_POST_MIN, 0.0, 0.0, 1.0, 1.0, THIN_FENCE_POST_MAX),
    AABB::newPermanent(THIN_FENCE_POST_MIN, 0.0, 0.0, 1.0, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, THIN_FENCE_POST_MAX),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)};

ThinFenceBlock::ThinFenceBlock(const std::wstring& name, const std::wstring& edgeTexture, Material* material,
                               bool extraBlock)
    : Block(material)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(THIN_FENCE_NORTH, false)
                             ->setValue(THIN_FENCE_EAST, false)
                             ->setValue(THIN_FENCE_SOUTH, false)
                             ->setValue(THIN_FENCE_WEST, false));
    mEdgeTextureId = 0;
    mEdgeTexture = edgeTexture;
    mDropsOnBreak = extraBlock;
    mName = name;
}

int ThinFenceBlock::indexFor(const Direction* direction)
{
    return 1 << (direction->get2DDataValue() & 0x3f);
}

void ThinFenceBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos,
                                       const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                       boost::shared_ptr<Entity> entity, bool isActualState)
{
    if (!isActualState)
    {
        state = fillVirtualBlockStateProperties(state, level, pos);
    }
    addCollisionAABB(pos, entityBox, collidingBoxes, thinFenceShapes[0]);
    if (state->getValue(THIN_FENCE_NORTH))
    {
        addCollisionAABB(pos, entityBox, collidingBoxes, thinFenceShapes[indexFor(Direction::NORTH)]);
    }
    if (state->getValue(THIN_FENCE_SOUTH))
    {
        addCollisionAABB(pos, entityBox, collidingBoxes, thinFenceShapes[indexFor(Direction::SOUTH)]);
    }
    if (state->getValue(THIN_FENCE_EAST))
    {
        addCollisionAABB(pos, entityBox, collidingBoxes, thinFenceShapes[indexFor(Direction::EAST)]);
    }
    if (state->getValue(THIN_FENCE_WEST))
    {
        addCollisionAABB(pos, entityBox, collidingBoxes, thinFenceShapes[indexFor(Direction::WEST)]);
    }
}

int ThinFenceBlock::getAABBIndex(const BlockState* state)
{
    int index = 0;
    if (state->getValue(THIN_FENCE_NORTH))
        index |= indexFor(Direction::NORTH);
    if (state->getValue(THIN_FENCE_EAST))
        index |= indexFor(Direction::EAST);
    if (state->getValue(THIN_FENCE_SOUTH))
        index |= indexFor(Direction::SOUTH);
    if (state->getValue(THIN_FENCE_WEST))
        index |= indexFor(Direction::WEST);
    return index;
}

const AABB* ThinFenceBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* actual = fillVirtualBlockStateProperties(state, levelSource, pos);
    return thinFenceShapes[getAABBIndex(actual)];
}

bool ThinFenceBlock::isExceptionBlockForAttachingPlusPiston(Block* block)
{
    return (block != NULL && (dynamic_cast<ShulkerBoxBlock*>(block) != NULL || dynamic_cast<LeafBlock*>(block) != NULL))
           || block == Blocks::beacon || block == Blocks::cauldron || block == Blocks::glowstone || block == Blocks::ice
           || block == Blocks::sea_lantern || block == Blocks::piston || block == Blocks::sticky_piston
           || block == Blocks::piston_head || block == Blocks::melon_block || block == Blocks::pumpkin
           || block == Blocks::carved_pumpkin || block == Blocks::lit_pumpkin || block == Blocks::barrier
           || block == Blocks::turtle_egg;
}

bool ThinFenceBlock::attachsTo(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                               const Direction* direction)
{
    Block* block = state->getBlock();
    int faceShape = state->getBlockFaceShape(levelSource, pos, direction);
    return !isExceptionBlockForAttachingPlusPiston(block) && (faceShape == 0 || faceShape == 3);
}

const BlockState* ThinFenceBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                                  const BlockPos& pos)
{
    return state
        ->setValue(THIN_FENCE_NORTH,
                   attachsTo(levelSource, levelSource->getBlockState(pos.north()), pos.north(), Direction::SOUTH))
        ->setValue(THIN_FENCE_SOUTH,
                   attachsTo(levelSource, levelSource->getBlockState(pos.south()), pos.south(), Direction::NORTH))
        ->setValue(THIN_FENCE_WEST,
                   attachsTo(levelSource, levelSource->getBlockState(pos.west()), pos.west(), Direction::EAST))
        ->setValue(THIN_FENCE_EAST,
                   attachsTo(levelSource, levelSource->getBlockState(pos.east()), pos.east(), Direction::WEST));
}

Item* ThinFenceBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    if (!mDropsOnBreak)
    {
        return Items::air;
    }
    return Block::getResource(state, random, lootBonus);
}

bool ThinFenceBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool ThinFenceBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool ThinFenceBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                      const Direction* direction)
{
    if (levelSource->getBlockState(pos)->getBlock() != this)
        return Block::shouldRenderFace(state, levelSource, pos, direction);
    return false;
}

bool ThinFenceBlock::canContainLiquid(const BlockState*) const
{
    return true;
}
bool ThinFenceBlock::isSilkTouchable()
{
    return true;
}
int ThinFenceBlock::getRenderLayer()
{
    return 0;
}
int ThinFenceBlock::convertBlockStateToLegacyData(const BlockState*)
{
    return 0;
}

const BlockState* ThinFenceBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        return state->setValue(THIN_FENCE_NORTH, state->getValue(THIN_FENCE_SOUTH))
            ->setValue(THIN_FENCE_EAST, state->getValue(THIN_FENCE_WEST))
            ->setValue(THIN_FENCE_SOUTH, state->getValue(THIN_FENCE_NORTH))
            ->setValue(THIN_FENCE_WEST, state->getValue(THIN_FENCE_EAST));
    case Rotation::eTYPE_CounterClockwise90:
        return state->setValue(THIN_FENCE_NORTH, state->getValue(THIN_FENCE_EAST))
            ->setValue(THIN_FENCE_EAST, state->getValue(THIN_FENCE_SOUTH))
            ->setValue(THIN_FENCE_SOUTH, state->getValue(THIN_FENCE_WEST))
            ->setValue(THIN_FENCE_WEST, state->getValue(THIN_FENCE_NORTH));
    case Rotation::eTYPE_Clockwise90:
        return state->setValue(THIN_FENCE_NORTH, state->getValue(THIN_FENCE_WEST))
            ->setValue(THIN_FENCE_EAST, state->getValue(THIN_FENCE_NORTH))
            ->setValue(THIN_FENCE_SOUTH, state->getValue(THIN_FENCE_EAST))
            ->setValue(THIN_FENCE_WEST, state->getValue(THIN_FENCE_SOUTH));
    default:
        return state;
    }
}

const BlockState* ThinFenceBlock::mirror(const BlockState* state, Mirror* mirror)
{
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        return state->setValue(THIN_FENCE_NORTH, state->getValue(THIN_FENCE_SOUTH))
            ->setValue(THIN_FENCE_SOUTH, state->getValue(THIN_FENCE_NORTH));
    case Mirror::eTYPE_FrontBack:
        return state->setValue(THIN_FENCE_EAST, state->getValue(THIN_FENCE_WEST))
            ->setValue(THIN_FENCE_WEST, state->getValue(THIN_FENCE_EAST));
    default:
        return Block::mirror(state, mirror);
    }
}

BlockStateDefinition* ThinFenceBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {THIN_FENCE_NORTH, THIN_FENCE_EAST, THIN_FENCE_WEST, THIN_FENCE_SOUTH};
    return new BlockStateDefinition(this, properties, 4);
}

EBlockFaceShape ThinFenceBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                                  const Direction* direction)
{
    return direction == Direction::UP || direction == Direction::DOWN ? BLOCK_FACE_CENTER_SMALL
                                                                      : BLOCK_FACE_MIDDLE_POLE_THIN;
}

void ThinFenceBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mName);
    mEdgeTextureId = iconRegister->registerIcon(mEdgeTexture);
}

TextureAtlasSprite* ThinFenceBlock::getEdgeTexture()
{
    return mEdgeTextureId;
}

EBlockRenderShape ThinFenceBlock::getRenderShape(const BlockState* state)
{
    return mMaterial == Material::GLASS ? BLOCK_RENDER_GLASS_PANE : BLOCK_RENDER_THIN_FENCE;
}
