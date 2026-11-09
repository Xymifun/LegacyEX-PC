#include "game/world/block/HopperBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/util/AABB.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/Direction.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Containers.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HopperBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

const std::wstring HopperBlock::TEXTURE_OUTSIDE = L"hopper_outside";
const std::wstring HopperBlock::TEXTURE_INSIDE = L"hopper_inside";
DirectionProperty* HopperBlock::FACING = NULL;
Property* HopperBlock::ENABLED = BooleanProperty::create(L"enabled");
const AABB* HopperBlock::BASE_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.625, 1.0);
const AABB* HopperBlock::SOUTH_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.125);
const AABB* HopperBlock::NORTH_AABB = AABB::newPermanent(0.0, 0.0, 0.875, 1.0, 1.0, 1.0);
const AABB* HopperBlock::WEST_AABB = AABB::newPermanent(0.875, 0.0, 0.0, 1.0, 1.0, 1.0);
const AABB* HopperBlock::EAST_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 0.125, 1.0, 1.0);

bool notUpFunction(const Direction* direction)
{
    return direction != Direction::UP;
}

HopperBlock::HopperBlock() : BaseEntityBlock(Material::METAL, MaterialColor::STONE)
{
    mBlockType = EBLOCK_TYPE_HOPPER;
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(FACING, Direction::DOWN)
                             ->setValue(static_cast<const AbstractProperty<bool>*>(ENABLED), true));
}

boost::shared_ptr<BlockEntity> HopperBlock::newBlockEntity(Level* level, int data)
{
    boost::shared_ptr<BlockEntity> blockEntity = boost::shared_ptr<HopperBlockEntity>(new HopperBlockEntity());
    blockEntity->setLevel(level);
    return blockEntity;
}

void HopperBlock::blockStaticCtor()
{
    FACING = DirectionProperty::create(L"facing", new Predicates<const Direction*>::FunctionPredicate(notUpFunction));
}

const AABB* HopperBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return FULL_BLOCK_SHAPE;
}

void HopperBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                    std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                    bool isActualState)
{
    addCollisionAABB(pos, entityBox, collidingBoxes, BASE_AABB);
    addCollisionAABB(pos, entityBox, collidingBoxes, EAST_AABB);
    addCollisionAABB(pos, entityBox, collidingBoxes, WEST_AABB);
    addCollisionAABB(pos, entityBox, collidingBoxes, SOUTH_AABB);
    addCollisionAABB(pos, entityBox, collidingBoxes, NORTH_AABB);
}

const BlockState* HopperBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                    float hitX, float hitY, float hitZ, int auxValue,
                                                    boost::shared_ptr<LivingEntity> placer)
{
    const Direction* attached = direction->getOpposite();
    if (attached == Direction::UP)
    {
        attached = Direction::DOWN;
    }
    return defaultBlockState()
        ->setValue(FACING, attached)
        ->setValue(static_cast<const AbstractProperty<bool>*>(ENABLED), true);
}

void HopperBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                              boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    Block::setPlacedBy(level, pos, state, placer, itemInstance);
    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        bool isHopper = eTYPE_DERIVED_FROM(BlockEntityType::HOPPER, blockEntity->GetType());
        if (isHopper)
        {
            boost::shared_ptr<HopperBlockEntity> hopper = boost::dynamic_pointer_cast<HopperBlockEntity>(blockEntity);
            hopper->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                  itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
        }
    }
}

bool HopperBlock::isTopSolidBlocking(const BlockState* state)
{
    return true;
}

void HopperBlock::checkPoweredState(Level* level, const BlockPos& pos, const BlockState* state)
{
    bool powered = !level->hasNeighborSignal(pos);
    bool enabled = state->getValue(static_cast<const AbstractProperty<bool>*>(ENABLED));
    if (powered != enabled)
    {
        level->setBlock(pos, state->setValue(static_cast<const AbstractProperty<bool>*>(ENABLED), powered),
                        Block::UPDATE_INVISIBLE, false);
    }
}

void HopperBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    checkPoweredState(level, pos, state);
}

bool HopperBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                      InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                      float hitZ, bool clientSide)
{
    if (!level->mIsClientSide)
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        bool isHopper = eTYPE_DERIVED_FROM(BlockEntityType::HOPPER, blockEntity->GetType());
        if (isHopper)
        {
            player->openContainer(boost::dynamic_pointer_cast<HopperBlockEntity>(blockEntity));
        }
    }
    return true;
}

void HopperBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                  const BlockPos& fromPos)
{
    checkPoweredState(level, pos, state);
}

void HopperBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity != NULL)
    {
        bool isHopper = eTYPE_DERIVED_FROM(BlockEntityType::HOPPER, blockEntity->GetType());
        if (isHopper)
        {
            Containers::dropContents(level, pos, boost::dynamic_pointer_cast<HopperBlockEntity>(blockEntity));
            level->updateNeighbourForOutputSignal(pos, this);
        }
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

bool HopperBlock::isCubeShaped(const BlockState* state)
{
    return false;
}
bool HopperBlock::isSolidRender(const BlockState* state) const
{
    return false;
}
bool HopperBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                   const Direction* direction)
{
    return true;
}

const Direction* HopperBlock::getAttachedFace(int data)
{
    return Direction::from3DDataValue(data & 7);
}

bool HopperBlock::isTurnedOn(int data)
{
    return (data & 8) == 0;
}

bool HopperBlock::hasAnalogOutputSignal(const BlockState* state)
{
    return true;
}

int HopperBlock::getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos)
{
    return AbstractContainerMenu::getRedstoneSignalFromBlockEntity(level->getBlockEntity(pos));
}

int HopperBlock::getRenderLayer()
{
    return 0;
}

const BlockState* HopperBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(FACING, getAttachedFace(data))
        ->setValue(static_cast<const AbstractProperty<bool>*>(ENABLED), isTurnedOn(data));
}

int HopperBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(static_cast<const AbstractProperty<const Direction*>*>(FACING))->get3DDataValue();
    if (!state->getValue(static_cast<const AbstractProperty<bool>*>(ENABLED)))
    {
        data |= 8;
    }
    return data;
}

unsigned int HopperBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
    {
        return IDS_TOOLTIPS_OPEN;
    }
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* HopperBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* HopperBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* HopperBlock::createBlockStateDefinition()
{
    static const Property* properties[2] = {FACING, ENABLED};
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape HopperBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                               const Direction* direction)
{
    return direction == Direction::UP ? BLOCK_FACE_BOWL : BLOCK_FACE_UNDEFINED;
}

TextureAtlasSprite* HopperBlock::getTexture(const Direction* direction, const BlockState* state)
{
    return direction == Direction::UP ? mTopTexture : mSideTexture;
}

void HopperBlock::registerIcons(IconRegister* iconRegister)
{
    mSideTexture = iconRegister->registerIcon(TEXTURE_OUTSIDE);
    mTopTexture = iconRegister->registerIcon(L"hopper_top");
    mBottomTexture = iconRegister->registerIcon(TEXTURE_INSIDE);
}

TextureAtlasSprite* HopperBlock::getTexture(const std::wstring& name)
{
    if (name == TEXTURE_OUTSIDE)
        return static_cast<HopperBlock*>(Blocks::hopper)->mSideTexture;
    if (name == TEXTURE_INSIDE)
        return static_cast<HopperBlock*>(Blocks::hopper)->mBottomTexture;
    return NULL;
}

std::wstring HopperBlock::getTileItemIconName()
{
    return L"hopper";
}

EBlockRenderShape HopperBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_HOPPER;
}

bool HopperBlock::canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType)
{
    return direction == Direction::UP;
}
