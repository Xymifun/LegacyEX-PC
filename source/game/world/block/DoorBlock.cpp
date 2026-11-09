#include "game/world/block/DoorBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"

namespace
{
const unsigned int kDoorTooltipMetalLocked = IDS_TOOLTIPS_PLACE;
const unsigned int kDoorTooltipUnavailable = 0xffffffff;
const unsigned int kDoorTooltipInteract = IDS_TOOLTIPS_USE;
}

#include <cstring>
#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/FlippedIcon.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

static DoorBlock::DoorHalf gLowerDoorHalf;
static DoorBlock::DoorHalf gUpperDoorHalf;
static DoorBlock::HingeSide gLeftHingeSide;
static DoorBlock::HingeSide gRightHingeSide;

static const AABB* gDoorSouthShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 0.1875);
static const AABB* gDoorNorthShape = AABB::newPermanent(0.0, 0.0, 0.8125, 1.0, 1.0, 1.0);
static const AABB* gDoorWestShape = AABB::newPermanent(0.8125, 0.0, 0.0, 1.0, 1.0, 1.0);
static const AABB* gDoorEastShape = AABB::newPermanent(0.0, 0.0, 0.0, 0.1875, 1.0, 1.0);

AbstractProperty<DoorBlock::DoorHalf*>* DoorBlock::HALF = NULL;
AbstractProperty<DoorBlock::HingeSide*>* DoorBlock::HINGE = NULL;
AbstractProperty<bool>* DoorBlock::OPEN = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"open"));
AbstractProperty<bool>* DoorBlock::POWERED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"powered"));
AbstractProperty<const Direction*>* DoorBlock::FACING = NULL;
DoorBlock::DoorHalf* DoorBlock::LOWER = &gLowerDoorHalf;
DoorBlock::DoorHalf* DoorBlock::UPPER = &gUpperDoorHalf;
DoorBlock::HingeSide* DoorBlock::LEFT = &gLeftHingeSide;
DoorBlock::HingeSide* DoorBlock::RIGHT = &gRightHingeSide;

std::wstring DoorBlock::DoorHalf::toString() const
{
    return getSerializedName();
}
std::wstring DoorBlock::DoorHalf::getSerializedName() const
{
    return this == DoorBlock::UPPER ? L"upper" : L"lower";
}
std::wstring DoorBlock::HingeSide::toString() const
{
    return getSerializedName();
}
std::wstring DoorBlock::HingeSide::getSerializedName() const
{
    return this == DoorBlock::LEFT ? L"left" : L"right";
}

DoorBlock::DoorBlock(Material* material) : Block(material)
{
    mIsDoor = true;
    mBlockType = EBLOCK_TYPE_DOOR;
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    state = state->setValue(FACING, Direction::NORTH);
    state = state->setValue(OPEN, false);
    state = state->setValue(HINGE, LEFT);
    state = state->setValue(POWERED, false);
    state = state->setValue(HALF, LOWER);
    registerDefaultState(state);
    memset(mUpperIcons, 0, sizeof(mUpperIcons));
    memset(mLowerIcons, 0, sizeof(mLowerIcons));
}

void DoorBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
    {
        std::wstring name;
        name = L"hinge";
        boost::unordered_set<HingeSide*> hinges;
        hinges.insert(LEFT);
        hinges.insert(RIGHT);
        HINGE = EnumProperty<HingeSide*>::create(name, typeid(HingeSide*), hinges, Predicates<HingeSide*>::alwaysTrue);
    }
    {
        std::wstring name;
        name = L"half";
        boost::unordered_set<DoorHalf*> halves;
        halves.insert(LOWER);
        halves.insert(UPPER);
        HALF = EnumProperty<DoorHalf*>::create(name, typeid(DoorHalf*), halves, Predicates<DoorHalf*>::alwaysTrue);
    }
}

const AABB* DoorBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    state = state->fillVirtualBlockStateProperties(levelSource, pos);
    const Direction* facing = state->getValue(FACING);
    bool closed = !state->getValue(OPEN);
    bool rightHinge = state->getValue(HINGE) == RIGHT;
    switch (facing->get3DDataValue())
    {
    case 5:
    default:
        return closed ? gDoorEastShape : (rightHinge ? gDoorNorthShape : gDoorSouthShape);
    case 2:
        return closed ? gDoorNorthShape : (rightHinge ? gDoorWestShape : gDoorEastShape);
    case 3:
        return closed ? gDoorSouthShape : (rightHinge ? gDoorEastShape : gDoorWestShape);
    case 4:
        return closed ? gDoorWestShape : (rightHinge ? gDoorSouthShape : gDoorNorthShape);
    }
}

std::wstring DoorBlock::getName()
{
    return L10N::GetString(getDescriptionId(-1));
}
bool DoorBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool DoorBlock::isOpenFromData(int data)
{
    return (data >> 2) & 1;
}
bool DoorBlock::isUpperFromData(int data)
{
    return (data >> 3) & 1;
}
int DoorBlock::getLowerData(int data)
{
    return data & 7;
}

int DoorBlock::getCompositeData(LevelSource* levelSource, const BlockPos& pos)
{
    const BlockState* state = levelSource->getBlockState(pos);
    int data = state->getBlock()->convertBlockStateToLegacyData(state);
    bool upper = isUpperFromData(data);
    const BlockState* lowerState = levelSource->getBlockState(pos.below());
    int belowData = lowerState->getBlock()->convertBlockStateToLegacyData(lowerState);
    int lowerData = upper ? belowData : data;
    const BlockState* upperState = levelSource->getBlockState(pos.above());
    int aboveData = upperState->getBlock()->convertBlockStateToLegacyData(upperState);
    int upperData = upper ? data : aboveData;
    return getLowerData(lowerData) | (upper ? 8 : 0) | ((upperData & 1) ? 16 : 0) | ((upperData & 2) ? 32 : 0);
}

bool DoorBlock::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    return isOpenFromData(getCompositeData(levelSource, pos));
}
bool DoorBlock::isCubeShaped(const BlockState*)
{
    return false;
}
int DoorBlock::getCloseSound()
{
    return mMaterial == Material::METAL ? LevelEvent::SOUND_CLOSE_IRON_DOOR : LevelEvent::SOUND_CLOSE_WOODEN_DOOR;
}

int DoorBlock::getOpenSound()
{
    return mMaterial == Material::METAL ? LevelEvent::SOUND_OPEN_IRON_DOOR : LevelEvent::SOUND_OPEN_WOODEN_DOOR;
}
const MaterialColor* DoorBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (state->getBlock() == Blocks::iron_door)
        return MaterialColor::METAL;
    if (state->getBlock() == Blocks::wooden_door)
        return PlanksBlock::Variant::OAK->getMaterialColor();
    if (state->getBlock() == Blocks::spruce_door)
        return PlanksBlock::Variant::SPRUCE->getMaterialColor();
    if (state->getBlock() == Blocks::birch_door)
        return PlanksBlock::Variant::BIRCH->getMaterialColor();
    if (state->getBlock() == Blocks::jungle_door)
        return PlanksBlock::Variant::JUNGLE->getMaterialColor();
    if (state->getBlock() == Blocks::acacia_door)
        return PlanksBlock::Variant::ACACIA->getMaterialColor();
    if (state->getBlock() == Blocks::dark_oak_door)
        return PlanksBlock::Variant::DARK_OAK->getMaterialColor();
    return Block::getMapColor(state, levelSource, pos);
}
bool DoorBlock::TestUse()
{
    return this != Blocks::iron_door;
}

bool DoorBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                    InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (mMaterial == Material::METAL)
        return false;
    if (clientSide)
    {
        level->levelEvent(player, state->getValue(OPEN) ? getOpenSound() : getCloseSound(), pos, 0);
        return false;
    }
    BlockPos lowerPos = state->getValue(HALF) == LOWER ? pos : pos.below();
    if (!pos.equals(lowerPos))
        state = level->getBlockState(lowerPos);
    if (state->getBlock() != this)
        return false;
    const BlockState* changedState = state->cycle(OPEN);
    level->setBlock(lowerPos, changedState, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
    level->setBlocksDirty(lowerPos, pos);
    BlockPos upperPos = lowerPos.above();
    level->updateObservingBlocksAt(upperPos, level->getBlock(upperPos));
    level->levelEvent(player, changedState->getValue(OPEN) ? getOpenSound() : getCloseSound(), pos, 0);
    return true;
}

void DoorBlock::setOpen(Level* level, const BlockPos& pos, bool open)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != this)
        return;
    BlockPos lowerPos = state->getValue(HALF) == LOWER ? pos : pos.below();
    const BlockState* lowerState = state;
    if (!(pos == lowerPos))
        lowerState = level->getBlockState(lowerPos);
    if (lowerState->getBlock() != this)
        return;
    if (lowerState->getValue(OPEN) == open)
        return;
    level->setBlock(lowerPos, lowerState->setValue(OPEN, open), Block::UPDATE_CLIENTS | Block::UPDATE_IMMEDIATE, false);
    BlockPos upperPos = lowerPos.above();
    level->updateObservingBlocksAt(upperPos, level->getBlock(upperPos));
    level->setBlocksDirty(lowerPos, pos);
    level->levelEvent(boost::shared_ptr<Player>(), open ? getOpenSound() : getCloseSound(), pos, 0);
}

bool DoorBlock::CanSurvive(const BlockState* state)
{
    return state->isTopSolidBlocking() || Block::isExceptionForTopSolidBlocking(state->getBlock());
}
void DoorBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                const BlockPos& fromPos)
{
    if (state->getValue(HALF) == UPPER)
    {
        BlockPos lowerPos = pos.below();
        const BlockState* lowerState = level->getBlockState(lowerPos);
        if (lowerState->getBlock() != this)
        {
            level->removeBlock(pos);
            return;
        }
        if (block != this)
            lowerState->neighborChanged(level, lowerPos, block, fromPos);
        return;
    }
    BlockPos upperPos = pos.above();
    const BlockState* upperState = level->getBlockState(upperPos);
    if (upperState->getBlock() != this)
    {
        level->removeBlock(pos);
#ifdef MINECRAFT_PC
        return;
#endif
    }
    if (!CanSurvive(level->getBlockState(pos.below())))
    {
        level->removeBlock(pos);
        if (upperState->getBlock() == this)
            level->removeBlock(upperPos);
        if (!level->mIsClientSide)
            spawnResources(level, pos, state, 0);
        return;
    }
    bool powered = level->hasNeighborSignal(pos) || level->hasNeighborSignal(upperPos);
    if (block == this || (!powered && !block->defaultBlockState()->isSignalSource()))
        return;
    if (powered != upperState->getValue(POWERED))
    {
        level->setBlock(upperPos, upperState->setValue(POWERED, powered), Block::UPDATE_ALL, false);
        if (powered != state->getValue(OPEN))
        {
            level->setBlock(pos, state->setValue(OPEN, powered), Block::UPDATE_ALL, false);
            level->setBlocksDirty(pos, pos);
            level->levelEvent(boost::shared_ptr<Player>(), powered ? getOpenSound() : getCloseSound(), pos, 0);
        }
    }
}
Item* DoorBlock::getItem()
{
    if (this == Blocks::iron_door)
        return Items::ironDoor;
    if (this == Blocks::spruce_door)
        return Items::spruceDoor;
    if (this == Blocks::birch_door)
        return Items::birchDoor;
    if (this == Blocks::jungle_door)
        return Items::jungleDoor;
    if (this == Blocks::acacia_door)
        return Items::acaciaDoor;
    if (this == Blocks::dark_oak_door)
        return Items::darkOakDoor;
    return Items::woodenDoor;
}
Item* DoorBlock::getResource(const BlockState*, Random*, int)
{
    return getItem();
}
bool DoorBlock::mayPlace(Level* level, const BlockPos& pos)
{
    if (pos.y >= 255)
        return false;
    return Block::mayPlace(level, pos) && Block::mayPlace(level, pos.above())
           && CanSurvive(level->getBlockState(pos.below()));
}
EPistonPushReaction DoorBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_DESTROY;
}
not_null_ptr<ItemInstance> DoorBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(getItem()));
}
bool DoorBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                  boost::shared_ptr<Player> player)
{
    BlockPos lowerPos = pos.below();
    BlockPos upperPos = pos.above();
    if (state->getValue(HALF) == UPPER && level->getBlockState(lowerPos)->getBlock() == this)
    {
        if (player->mAbilities.mInstabuild)
            level->removeBlock(pos);
        level->removeBlock(lowerPos);
    }
    if (state->getValue(HALF) == LOWER && level->getBlockState(upperPos)->getBlock() == this)
    {
        if (player->mAbilities.mInstabuild)
            level->removeBlock(pos);
        level->removeBlock(upperPos);
    }
    return true;
}
int DoorBlock::getRenderLayer()
{
    return 0;
}
const BlockState* DoorBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                             const BlockPos& pos)
{
    if (state->getValue(HALF) == LOWER)
    {
        const BlockState* upperState = levelSource->getBlockState(pos.above());
        if (upperState->getBlock() == this)
        {
            state =
                state->setValue(HINGE, upperState->getValue(HINGE))->setValue(POWERED, upperState->getValue(POWERED));
        }
    }
    else
    {
        const BlockState* lowerState = levelSource->getBlockState(pos.below());
        if (lowerState->getBlock() == this)
        {
            state = state->setValue(FACING, lowerState->getValue(FACING))->setValue(OPEN, lowerState->getValue(OPEN));
        }
    }
    return state;
}
const BlockState* DoorBlock::getBlockState(int data)
{
    int upper = data & 8;
    const BlockState* state = defaultBlockState();
    if (upper != 0)
    {
        state = state->setValue(HALF, UPPER);
        return state->setValue(HINGE, (data & 1) > 0 ? RIGHT : LEFT)->setValue(POWERED, (data & 2) > 0);
    }
    state = state->setValue(HALF, LOWER);
    state = state->setValue(FACING, Direction::from2DDataValue(data & 3)->getCounterClockWise());
    return state->setValue(OPEN, (data & 4) > 0);
}
int DoorBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = 0;
    if (state->getValue(HALF) == UPPER)
    {
        data = 8;
        if (state->getValue(HINGE) == RIGHT)
            data |= 1;
        if (state->getValue(POWERED))
            data |= 2;
    }
    else
    {
        data = state->getValue(FACING)->getClockWise()->get2DDataValue();
        if (state->getValue(OPEN))
            data |= 4;
    }
    return data;
}
unsigned int DoorBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    unsigned int tooltip = Block::GetInteractTooltip(tooltipData);
    if (mMaterial == Material::METAL)
    {
        if (!tooltipData.m_bUseBlock && !tooltipData.m_bUseItemOnBlock)
            return tooltip;
        if (tooltip == kDoorTooltipMetalLocked || tooltip == kDoorTooltipUnavailable)
            return kDoorTooltipUnavailable;
    }
    if (!tooltipData.m_bUseBlock && !tooltipData.m_bUseItemOnBlock)
        return tooltip;
    return kDoorTooltipInteract;
}
const BlockState* DoorBlock::rotate(const BlockState* state, Rotation* rotation)
{
    if (state->getValue(HALF) == LOWER)
    {
        state = state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
    }
    return state;
}
const BlockState* DoorBlock::mirror(const BlockState* state, Mirror* mirror)
{
    if (mirror == Mirror::NONE)
        return state;
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}
bool DoorBlock::isOpen(LevelSource* levelSource, const BlockPos& pos)
{
    return isOpenFromData(getCompositeData(levelSource, pos));
}
const Direction* DoorBlock::getFacingFromData(int data)
{
    return Direction::from2DDataValue(data & 3)->getCounterClockWise();
}
const Direction* DoorBlock::getFacing(LevelSource* levelSource, const BlockPos& pos)
{
    return getFacingFromData(getCompositeData(levelSource, pos));
}
BlockStateDefinition* DoorBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {HALF, FACING, OPEN, HINGE, POWERED};
    return new BlockStateDefinition(this, properties, 5);
}
EBlockFaceShape DoorBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
bool DoorBlock::isDoor(Block* block)
{
    return block == Blocks::wooden_door || block == Blocks::spruce_door || block == Blocks::jungle_door
           || block == Blocks::birch_door || block == Blocks::acacia_door || block == Blocks::dark_oak_door;
}
TextureAtlasSprite* DoorBlock::getTexture(const Direction*, const BlockState*)
{
    return mLowerIcons[0];
}
TextureAtlasSprite* DoorBlock::getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction,
                                          const BlockState* state)
{
    if (direction == Direction::UP || direction == Direction::DOWN)
        return mLowerIcons[0];
    state = fillVirtualBlockStateProperties(state, levelSource, pos);
    const Direction* facing = state->getValue(FACING)->getClockWise();
    bool open = state->getValue(OPEN);
    int flip = 0;
    bool upper = state->getValue(HALF) == UPPER;
    if (open)
    {
        if ((facing == Direction::SOUTH && direction == Direction::NORTH)
            || (facing == Direction::WEST && direction == Direction::EAST)
            || (facing == Direction::NORTH && direction == Direction::SOUTH)
            || (facing == Direction::EAST && direction == Direction::WEST))
        {
            flip ^= 1;
        }
    }
    else
    {
        if ((facing == Direction::SOUTH && direction == Direction::EAST)
            || (facing == Direction::WEST && direction == Direction::SOUTH)
            || (facing == Direction::NORTH && direction == Direction::WEST)
            || (facing == Direction::EAST && direction == Direction::NORTH))
        {
            flip = 1;
        }
        if (state->getValue(HINGE) == RIGHT)
            flip ^= 1;
    }
    if (upper)
        return mUpperIcons[flip];
    return mLowerIcons[flip];
}
void DoorBlock::registerIcons(IconRegister* iconRegister)
{
    mUpperIcons[0] = iconRegister->registerIcon(getIconName() + L"_upper");
    mLowerIcons[0] = iconRegister->registerIcon(getIconName() + L"_lower");
    delete mUpperIcons[1];
    mUpperIcons[1] = new FlippedIcon(mUpperIcons[0], true, false);
    delete mLowerIcons[1];
    mLowerIcons[1] = new FlippedIcon(mLowerIcons[0], true, false);
}
EBlockRenderShape DoorBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_DOOR;
}
