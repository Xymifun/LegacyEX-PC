#include "game/world/block/FenceBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/LeadItem.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FenceGateBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* FenceBlock::NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
AbstractProperty<bool>* FenceBlock::EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
AbstractProperty<bool>* FenceBlock::SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
AbstractProperty<bool>* FenceBlock::WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));

const AABB* FenceBlock::BOUNDING_BOXES[16] = {
    AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 1.0, 0.625), AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.375, 0.625, 1.0, 0.625),   AABB::newPermanent(0.0, 0.0, 0.375, 0.625, 1.0, 1.0),
    AABB::newPermanent(0.375, 0.0, 0.0, 0.625, 1.0, 0.625),   AABB::newPermanent(0.375, 0.0, 0.0, 0.625, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 0.625, 1.0, 0.625),     AABB::newPermanent(0.0, 0.0, 0.0, 0.625, 1.0, 1.0),
    AABB::newPermanent(0.375, 0.0, 0.375, 1.0, 1.0, 0.625),   AABB::newPermanent(0.375, 0.0, 0.375, 1.0, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.375, 1.0, 1.0, 0.625),     AABB::newPermanent(0.0, 0.0, 0.375, 1.0, 1.0, 1.0),
    AABB::newPermanent(0.375, 0.0, 0.0, 1.0, 1.0, 0.625),     AABB::newPermanent(0.375, 0.0, 0.0, 1.0, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.625),       AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0),
};
const AABB* FenceBlock::PILLAR_AABB = AABB::newPermanent(0.375, 0.0, 0.375, 0.625, 1.5, 0.625);
const AABB* FenceBlock::SOUTH_AABB = AABB::newPermanent(0.375, 0.0, 0.625, 0.625, 1.5, 1.0);
const AABB* FenceBlock::WEST_AABB = AABB::newPermanent(0.0, 0.0, 0.375, 0.375, 1.5, 0.625);
const AABB* FenceBlock::NORTH_AABB = AABB::newPermanent(0.375, 0.0, 0.0, 0.625, 1.5, 0.375);
const AABB* FenceBlock::EAST_AABB = AABB::newPermanent(0.625, 0.0, 0.375, 1.0, 1.5, 0.625);

FenceBlock::FenceBlock(const std::wstring& name, Material* material, const MaterialColor* materialColor)
    : Block(material, materialColor),
      mTextureName(name)
{
    _init();
}

void FenceBlock::_init()
{
    mIsFence = true;
    mBlockType = EBLOCK_TYPE_FENCE;
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(NORTH, false)
                             ->setValue(EAST, false)
                             ->setValue(SOUTH, false)
                             ->setValue(WEST, false));
}

unsigned int FenceBlock::getAABBIndex(const BlockState* state)
{
    unsigned int index = 0;
    if (state->getValue(NORTH))
        index |= 1u << Direction::NORTH->get2DDataValue();
    if (state->getValue(EAST))
        index |= 1u << Direction::EAST->get2DDataValue();
    if (state->getValue(SOUTH))
        index |= 1u << Direction::SOUTH->get2DDataValue();
    if (state->getValue(WEST))
        index |= 1u << Direction::WEST->get2DDataValue();
    return index;
}

const AABB* FenceBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    state = fillVirtualBlockStateProperties(state, levelSource, pos);
    return BOUNDING_BOXES[getAABBIndex(state)];
}

void FenceBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState)
{
    addCollisionAABB(pos, entityBox, collidingBoxes, PILLAR_AABB);
    if (connectsTo(level, pos.north(), Direction::SOUTH))
        addCollisionAABB(pos, entityBox, collidingBoxes, NORTH_AABB);
    if (connectsTo(level, pos.east(), Direction::WEST))
        addCollisionAABB(pos, entityBox, collidingBoxes, EAST_AABB);
    if (connectsTo(level, pos.south(), Direction::NORTH))
        addCollisionAABB(pos, entityBox, collidingBoxes, SOUTH_AABB);
    if (connectsTo(level, pos.west(), Direction::EAST))
        addCollisionAABB(pos, entityBox, collidingBoxes, WEST_AABB);
}

bool FenceBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool FenceBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool FenceBlock::isPathfindable(LevelSource*, const BlockPos&)
{
    return false;
}
bool FenceBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*)
{
    return true;
}
int FenceBlock::convertBlockStateToLegacyData(const BlockState*)
{
    return 0;
}
bool FenceBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

bool FenceBlock::isExceptionBlockForAttachingPlusPiston(Block* block)
{
    return Block::isExceptionBlockForAttachingPlusPiston(block) || block == Blocks::barrier
           || block == Blocks::melon_block || block == Blocks::pumpkin || block == Blocks::carved_pumpkin
           || block == Blocks::lit_pumpkin;
}

bool FenceBlock::connectsTo(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = levelSource->getBlockState(pos);
    EBlockFaceShape faceShape = state->getBlockFaceShape(levelSource, pos, direction);
    Block* block = state->getBlock();
    bool fenceFace = faceShape == BLOCK_FACE_MIDDLE_POLE
                     && (state->getMaterial() == mMaterial
                         || (block != NULL && dynamic_cast<FenceGateBlock*>(block) != NULL));
    return (!isExceptionBlockForAttachingPlusPiston(block) && faceShape == BLOCK_FACE_SOLID) || fenceFace;
}

bool FenceBlock::canProvideSupport(const BlockState*, const Direction* direction, EBlockSupportType supportType)
{
    return direction == Direction::UP && (supportType == BLOCK_SUPPORT_CENTER || supportType == BLOCK_SUPPORT_ANY);
}

bool FenceBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand, const Direction*, float, float, float, bool)
{
    if (level->mIsClientSide)
    {
        return LeadItem::bindPlayerMobsTest(player, level, pos);
    }
    else
    {
        return LeadItem::bindPlayerMobs(player, level, pos);
    }
}

const BlockState* FenceBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos)
{
    return state->setValue(NORTH, connectsTo(levelSource, pos.north(), Direction::SOUTH))
        ->setValue(EAST, connectsTo(levelSource, pos.east(), Direction::WEST))
        ->setValue(SOUTH, connectsTo(levelSource, pos.south(), Direction::NORTH))
        ->setValue(WEST, connectsTo(levelSource, pos.west(), Direction::EAST));
}

const BlockState* FenceBlock::rotate(const BlockState* state, Rotation* rotation)
{
    switch (rotation->getType())
    {
    case Rotation::eTYPE_Clockwise180:
        return state->setValue(NORTH, state->getValue(SOUTH))
            ->setValue(EAST, state->getValue(WEST))
            ->setValue(SOUTH, state->getValue(NORTH))
            ->setValue(WEST, state->getValue(EAST));
    case Rotation::eTYPE_CounterClockwise90:
        return state->setValue(NORTH, state->getValue(EAST))
            ->setValue(EAST, state->getValue(SOUTH))
            ->setValue(SOUTH, state->getValue(WEST))
            ->setValue(WEST, state->getValue(NORTH));
    case Rotation::eTYPE_Clockwise90:
        return state->setValue(NORTH, state->getValue(WEST))
            ->setValue(EAST, state->getValue(NORTH))
            ->setValue(SOUTH, state->getValue(EAST))
            ->setValue(WEST, state->getValue(SOUTH));
    default:
        return state;
    }
}

const BlockState* FenceBlock::mirror(const BlockState* state, Mirror* mirror)
{
    switch (mirror->getType())
    {
    case Mirror::eTYPE_LeftRight:
        return state->setValue(NORTH, state->getValue(SOUTH))->setValue(SOUTH, state->getValue(NORTH));
    case Mirror::eTYPE_FrontBack:
        return state->setValue(EAST, state->getValue(WEST))->setValue(WEST, state->getValue(EAST));
    default:
        return Block::mirror(state, mirror);
    }
}

BlockStateDefinition* FenceBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {NORTH, EAST, WEST, SOUTH};
    return new BlockStateDefinition(this, properties, 4);
}

EBlockFaceShape FenceBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                              const Direction* direction)
{
    return direction == Direction::UP || direction == Direction::DOWN ? BLOCK_FACE_CENTER : BLOCK_FACE_MIDDLE_POLE;
}

void FenceBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(mTextureName);
}

EBlockRenderShape FenceBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_FENCE;
}

unsigned int FenceBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if (LeadItem::bindPlayerMobsTest(tooltipData.m_pPlayer, tooltipData.m_pLevel, tooltipData.m_blockPos))
        return IDS_TOOLTIPS_ATTACH;
    return Block::GetInteractTooltip(tooltipData);
}
