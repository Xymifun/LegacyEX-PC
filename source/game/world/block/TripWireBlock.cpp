#include "game/world/block/TripWireBlock.hpp"

#include <vector>
#include <typeinfo>
#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Item.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TripWireHookBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* TripWireBlock::POWERED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"powered"));
AbstractProperty<bool>* TripWireBlock::SUSPENDED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"suspended"));
AbstractProperty<bool>* TripWireBlock::ATTACHED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"attached"));
AbstractProperty<bool>* TripWireBlock::DISARMED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"disarmed"));
AbstractProperty<bool>* TripWireBlock::NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
AbstractProperty<bool>* TripWireBlock::EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
AbstractProperty<bool>* TripWireBlock::SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
AbstractProperty<bool>* TripWireBlock::WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));

static const AABB* TRIPWIRE_ATTACHED = AABB::newPermanent(0.0, 0.0625, 0.0, 1.0, 0.15625, 1.0);
static const AABB* TRIPWIRE_DETACHED = AABB::newPermanent(0.0, 0.0625, 0.0, 1.0, 0.09375, 1.0);

TripWireBlock::TripWireBlock() : Block(Material::DECORATION)
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(POWERED, false)
                             ->setValue(ATTACHED, false)
                             ->setValue(DISARMED, false)
                             ->setValue(NORTH, false)
                             ->setValue(EAST, false)
                             ->setValue(SOUTH, false)
                             ->setValue(WEST, false));
    setTicking(true);
}

const AABB* TripWireBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    if (!state->getValue(ATTACHED))
        return TRIPWIRE_DETACHED;
    return TRIPWIRE_ATTACHED;
}

bool TripWireBlock::shouldConnectTo(LevelSource* levelSource, const BlockPos& pos, const BlockState*,
                                    const Direction* direction)
{
    BlockPos neighbourPos = pos.relative(direction);
    const BlockState* neighbour = levelSource->getBlockState(neighbourPos);
    Block* block = neighbour->getBlock();
    if (block == Blocks::tripwire_hook)
    {
        const Direction* opposite = direction->getOpposite();
        return neighbour->getValue(static_cast<AbstractProperty<const Direction*>*>(TripWireHookBlock::FACING))
               == opposite;
    }
    return block == Blocks::tripwire;
}

const BlockState* TripWireBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                                 const BlockPos& pos)
{
    return state->setValue(NORTH, shouldConnectTo(levelSource, pos, state, Direction::NORTH))
        ->setValue(EAST, shouldConnectTo(levelSource, pos, state, Direction::EAST))
        ->setValue(SOUTH, shouldConnectTo(levelSource, pos, state, Direction::SOUTH))
        ->setValue(WEST, shouldConnectTo(levelSource, pos, state, Direction::WEST));
}

const AABB* TripWireBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}
bool TripWireBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool TripWireBlock::isCubeShaped(const BlockState*)
{
    return false;
}
int TripWireBlock::getRenderLayer()
{
    return 1;
}
Item* TripWireBlock::getResource(const BlockState*, Random*, int)
{
    return Items::string;
}
not_null_ptr<ItemInstance> TripWireBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::string));
}

void TripWireBlock::updateSource(Level* level, const BlockPos& pos, const BlockState* state)
{
    static const Direction* directions[2] = {Direction::SOUTH, Direction::WEST};
    for (int d = 0; d < 2; ++d)
    {
        const Direction* direction = directions[d];
        for (int i = 1; i < 42; ++i)
        {
            BlockPos testPos = pos.relative(direction, i);
            const BlockState* testState = level->getBlockState(testPos);
            if (testState->getBlock() == Blocks::tripwire_hook)
            {
                if (testState->getValue(TripWireHookBlock::FACING) == direction->getOpposite())
                {
                    static_cast<TripWireHookBlock*>(Blocks::tripwire_hook)
                        ->calculateState(level, testPos, testState, false, true, i, state);
                }
                break;
            }
            if (testState->getBlock() != Blocks::tripwire)
                break;
        }
    }
}

void TripWireBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    level->setBlock(pos, state, Block::UPDATE_ALL, false);
    updateSource(level, pos, state);
}

void TripWireBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    updateSource(level, pos, state->setValue(POWERED, true));
}

bool TripWireBlock::playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                      boost::shared_ptr<Player> player)
{
    if (!level->mIsClientSide)
    {
        if (!player->getMainHandItem()->isEmpty() && player->getMainHandItem()->getItem() == Items::shears)
        {
            level->setBlock(pos, state->setValue(DISARMED, true), Block::UPDATE_INVISIBLE, false);
        }
    }
    return false;
}

void TripWireBlock::checkPressed(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != this)
        return;

    bool wasPressed = state->getValue(POWERED);
    bool pressed = false;
    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(
        boost::shared_ptr<Entity>(), state->getShape(level, pos)->move(pos), NULL);
    if (!entities->empty())
    {
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
        {
            boost::shared_ptr<Entity> entity = *it;
            if (!entity->isIgnoringBlockTriggers())
            {
                pressed = true;
                break;
            }
        }
    }

    if (pressed != wasPressed)
    {
        state = state->setValue(POWERED, pressed);
        level->setBlock(pos, state, Block::UPDATE_ALL, false);
        updateSource(level, pos, state);
    }
    if (pressed)
        level->addToTickNextTick(pos, this, getTickDelay(level));
}

void TripWireBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                 const boost::shared_ptr<Entity>&)
{
    if (!level->mIsClientSide && !state->getValue(POWERED))
        checkPressed(level, pos);
}

void TripWireBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}
void TripWireBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && level->getBlockState(pos)->getValue(POWERED))
        checkPressed(level, pos);
}

const BlockState* TripWireBlock::getBlockState(int data)
{
    return defaultBlockState()
        ->setValue(POWERED, (data & 1) > 0)
        ->setValue(ATTACHED, (data & 4) > 0)
        ->setValue(DISARMED, (data & 8) > 0);
}

int TripWireBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = 0;
    if (state->getValue(POWERED))
        data |= 1;
    if (state->getValue(ATTACHED))
        data |= 4;
    if (state->getValue(DISARMED))
        data |= 8;
    return data;
}

const BlockState* TripWireBlock::rotate(const BlockState* state, Rotation* rotation)
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

const BlockState* TripWireBlock::mirror(const BlockState* state, Mirror* mirror)
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

BlockStateDefinition* TripWireBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {POWERED, SUSPENDED, ATTACHED, DISARMED, NORTH, EAST, WEST, SOUTH};
    return new BlockStateDefinition(this, properties, 8);
}

EBlockFaceShape TripWireBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}
EBlockRenderShape TripWireBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_TRIPWIRE;
}
