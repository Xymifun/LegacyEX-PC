#include "game/world/block/ButtonBlock.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

DirectionProperty* ButtonBlock::FACING = NULL;
AbstractProperty<bool>* ButtonBlock::POWERED = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"powered"));
const AABB* ButtonBlock::AABB_DOWN_OFF = AABB::newPermanent(0.3125, 0.875, 0.375, 0.6875, 1.0, 0.625);
const AABB* ButtonBlock::AABB_UP_OFF = AABB::newPermanent(0.3125, 0.0, 0.375, 0.6875, 0.125, 0.625);
const AABB* ButtonBlock::AABB_NORTH_OFF = AABB::newPermanent(0.3125, 0.375, 0.875, 0.6875, 0.625, 1.0);
const AABB* ButtonBlock::AABB_SOUTH_OFF = AABB::newPermanent(0.3125, 0.375, 0.0, 0.6875, 0.625, 0.125);
const AABB* ButtonBlock::AABB_WEST_OFF = AABB::newPermanent(0.875, 0.375, 0.3125, 1.0, 0.625, 0.6875);
const AABB* ButtonBlock::AABB_EAST_OFF = AABB::newPermanent(0.0, 0.375, 0.3125, 0.125, 0.625, 0.6875);
const AABB* ButtonBlock::AABB_DOWN_ON = AABB::newPermanent(0.3125, 0.9375, 0.375, 0.6875, 1.0, 0.625);
const AABB* ButtonBlock::AABB_UP_ON = AABB::newPermanent(0.3125, 0.0, 0.375, 0.6875, 0.0625, 0.625);
const AABB* ButtonBlock::AABB_NORTH_ON = AABB::newPermanent(0.3125, 0.375, 0.9375, 0.6875, 0.625, 1.0);
const AABB* ButtonBlock::AABB_SOUTH_ON = AABB::newPermanent(0.3125, 0.375, 0.0, 0.6875, 0.625, 0.0625);
const AABB* ButtonBlock::AABB_WEST_ON = AABB::newPermanent(0.9375, 0.375, 0.3125, 1.0, 0.625, 0.6875);
const AABB* ButtonBlock::AABB_EAST_ON = AABB::newPermanent(0.0, 0.375, 0.3125, 0.0625, 0.625, 0.6875);

ButtonBlock::ButtonBlock(bool wooden, Material* material) : Block(material)
{
    setTicking(true);
    mWooden = wooden;
}

void ButtonBlock::blockStaticCtor()
{
    FACING = DirectionalBlock::FACING;
}

void ButtonBlock::DerivedInit()
{
    Block::DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(FACING, Direction::NORTH)->setValue(POWERED, false));
}

const AABB* ButtonBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

int ButtonBlock::getTickDelay(Level*)
{
    return mWooden ? 30 : 20;
}

bool ButtonBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool ButtonBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool ButtonBlock::canAttachTo(Level* level, const BlockPos& pos, const Direction* direction)
{
    BlockPos supportPos = pos.relative(direction->getOpposite());
    const BlockState* supportState = level->getBlockState(supportPos);
    bool solidFace = supportState->getBlockFaceShape(level, supportPos, direction) == BLOCK_FACE_SOLID;
    Block* support = supportState->getBlock();
    if (direction == Direction::UP)
    {
        return support == Blocks::hopper || (!Block::isExceptionBlockForAttaching(support) && solidFace)
               || Block::isExceptionForPistonAttaching(support) || support->mIsFence;
    }
    return (!Block::isExceptionBlockForAttaching(support) && solidFace)
           || Block::isExceptionForPistonAttaching(support);
}

bool ButtonBlock::mayPlace(Level* level, const BlockPos& pos, const Direction* direction)
{
    return canAttachTo(level, pos, direction);
}

bool ButtonBlock::mayPlace(Level* level, const BlockPos& pos)
{
    for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
         ++it)
    {
        const Direction* direction = *it;
        if (canAttachTo(level, pos, direction))
            return true;
    }
    return false;
}

const BlockState* ButtonBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                    float, float, float, int, boost::shared_ptr<LivingEntity>)
{
    bool canAttach = canAttachTo(level, pos, direction);
    const BlockState* state = defaultBlockState();
    if (canAttach)
        return state->setValue(FACING, direction)->setValue(POWERED, false);
    return state->setValue(FACING, Direction::DOWN)->setValue(POWERED, false);
}

bool ButtonBlock::checkCanSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (mayPlace(level, pos))
        return true;
    spawnResources(level, pos, state, 0);
    level->removeBlock(pos);
    return false;
}

void ButtonBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (checkCanSurvive(level, pos, state) && !canAttachTo(level, pos, state->getValue(FACING)))
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
}

int ButtonBlock::getSpawnResourcesAuxValue(const BlockState*)
{
    return 4;
}

const AABB* ButtonBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    const Direction* direction = state->getValue(FACING);
    bool powered = state->getValue(POWERED);
    switch (direction->get3DDataValue())
    {
    case 0:
        return powered ? AABB_DOWN_ON : AABB_DOWN_OFF;
    case 1:
        return powered ? AABB_UP_ON : AABB_UP_OFF;
    case 3:
        return powered ? AABB_SOUTH_ON : AABB_SOUTH_OFF;
    case 4:
        return powered ? AABB_WEST_ON : AABB_WEST_OFF;
    case 5:
        return powered ? AABB_EAST_ON : AABB_EAST_OFF;
    default:
        return powered ? AABB_NORTH_ON : AABB_NORTH_OFF;
    }
}
bool ButtonBlock::TestUse()
{
    return true;
}

void ButtonBlock::updateNeighbours(Level* level, const BlockPos& pos, const Direction* direction)
{
    level->updateNeighborsAt(pos, this, false);
    level->updateNeighborsAt(pos.relative(direction->getOpposite()), this, false);
}

bool ButtonBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                      InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
    {
        playOnSound(player, level, pos);
        return false;
    }
    if (state->getValue(POWERED))
        return true;
    level->setBlock(pos, state->setValue(POWERED, true), Block::UPDATE_ALL, false);
    level->setBlocksDirty(pos, pos);
    playOnSound(player, level, pos);
    updateNeighbours(level, pos, state->getValue(FACING));
    level->addToTickNextTick(pos, this, getTickDelay(level));
    return true;
}

void ButtonBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (state->getValue(POWERED))
        updateNeighbours(level, pos, state->getValue(FACING));
}

int ButtonBlock::getSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction*)
{
    if (state->getValue(POWERED))
        return SIGNAL_MAX;
    return SIGNAL_NONE;
}

int ButtonBlock::getDirectSignal(const BlockState* state, LevelSource*, const BlockPos&, const Direction* direction)
{
    if (state->getValue(POWERED) && direction == state->getValue(FACING))
        return SIGNAL_MAX;
    return SIGNAL_NONE;
}

bool ButtonBlock::isSignalSource(const BlockState*)
{
    return true;
}
void ButtonBlock::randomTick(Level*, const BlockPos&, const BlockState*, Random*)
{
}

void ButtonBlock::checkPressed(const BlockState* state, Level* level, const BlockPos& pos)
{
    const AABB* shape = state->getShape(level, pos);
    std::vector<boost::shared_ptr<Entity> >* arrows = level->getEntitiesOfClass(typeid(Arrow), shape->move(pos));
    std::vector<boost::shared_ptr<Entity> >* tridents = level->getEntitiesOfClass(typeid(ThrownTrident),
                                                                                  shape->move(pos));
    bool pressed = !arrows->empty() || !tridents->empty();
    bool powered = state->getValue(POWERED);
    delete arrows;
    delete tridents;

    if (pressed && !powered)
    {
        level->setBlockAndUpdate(pos, state->setValue(POWERED, true));
        updateNeighbours(level, pos, state->getValue(FACING));
        level->setBlocksDirty(pos, pos);
        playOnSound(boost::shared_ptr<Player>(), level, pos);
    }
    if (!pressed && powered)
    {
        level->setBlockAndUpdate(pos, state->setValue(POWERED, false));
        updateNeighbours(level, pos, state->getValue(FACING));
        level->setBlocksDirty(pos, pos);
        playOffSound(level, pos);
    }
    if (pressed)
        level->addToTickNextTick(pos, this, getTickDelay(level));
}

void ButtonBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (!level->mIsClientSide && state->getValue(POWERED))
    {
        if (mWooden)
            checkPressed(state, level, pos);
        else
        {
            level->setBlockAndUpdate(pos, state->setValue(POWERED, false));
            updateNeighbours(level, pos, state->getValue(FACING));
            playOffSound(level, pos);
            level->setBlocksDirty(pos, pos);
        }
    }
}

void ButtonBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                               const boost::shared_ptr<Entity>&)
{
    if (!level->mIsClientSide && mWooden && !state->getValue(POWERED))
        checkPressed(state, level, pos);
}

const BlockState* ButtonBlock::getBlockState(int data)
{
    const Direction* direction;
    switch (data & 7)
    {
    case 0:
        direction = Direction::DOWN;
        break;
    case 1:
        direction = Direction::EAST;
        break;
    case 2:
        direction = Direction::WEST;
        break;
    case 3:
        direction = Direction::SOUTH;
        break;
    case 4:
        direction = Direction::NORTH;
        break;
    default:
        direction = Direction::UP;
        break;
    }
    return defaultBlockState()->setValue(FACING, direction)->setValue(POWERED, (data & 8) > 0);
}

int ButtonBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data;
    switch (state->getValue(FACING)->get3DDataValue())
    {
    case 0:
        data = 0;
        break;
    case 2:
        data = 4;
        break;
    case 3:
        data = 3;
        break;
    case 4:
        data = 2;
        break;
    case 5:
        data = 1;
        break;
    default:
        data = 5;
        break;
    }
    if (state->getValue(POWERED))
        data |= 8;
    return data;
}

unsigned int ButtonBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
        return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* ButtonBlock::rotate(const BlockState* state, Rotation* rotation)
{
    return state->setValue(FACING, rotation->rotate(state->getTypedBoxed(FACING)->getValue()));
}

const BlockState* ButtonBlock::mirror(const BlockState* state, Mirror* mirror)
{
    DirectionProperty* facing = FACING;
    const BlockState* result = rotate(state, mirror->getRotation(state->getValue(facing)));
    return result;
}

BlockStateDefinition* ButtonBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {FACING, POWERED};
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape ButtonBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool ButtonBlock::shouldBlockTick(Level*, const BlockPos&, const BlockState* state)
{
    return state->getValue(POWERED);
}
