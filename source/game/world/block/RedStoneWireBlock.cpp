#include "game/world/block/RedStoneWireBlock.hpp"

#include <string>
#include <vector>

#include <boost/unordered_set.hpp>

#include "game/client/render/ParticleType.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DiodeBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const AABB* gRedStoneWireShapes[16] = {AABB::newPermanent(0.1875, 0.0, 0.1875, 0.8125, 0.0625, 0.8125),
                                              AABB::newPermanent(0.1875, 0.0, 0.1875, 0.8125, 0.0625, 1.0),
                                              AABB::newPermanent(0.0, 0.0, 0.1875, 0.8125, 0.0625, 0.8125),
                                              AABB::newPermanent(0.0, 0.0, 0.1875, 0.8125, 0.0625, 1.0),
                                              AABB::newPermanent(0.1875, 0.0, 0.0, 0.8125, 0.0625, 0.8125),
                                              AABB::newPermanent(0.1875, 0.0, 0.0, 0.8125, 0.0625, 1.0),
                                              AABB::newPermanent(0.0, 0.0, 0.0, 0.8125, 0.0625, 0.8125),
                                              AABB::newPermanent(0.0, 0.0, 0.0, 0.8125, 0.0625, 1.0),
                                              AABB::newPermanent(0.1875, 0.0, 0.1875, 1.0, 0.0625, 0.8125),
                                              AABB::newPermanent(0.1875, 0.0, 0.1875, 1.0, 0.0625, 1.0),
                                              AABB::newPermanent(0.0, 0.0, 0.1875, 1.0, 0.0625, 0.8125),
                                              AABB::newPermanent(0.0, 0.0, 0.1875, 1.0, 0.0625, 1.0),
                                              AABB::newPermanent(0.1875, 0.0, 0.0, 1.0, 0.0625, 0.8125),
                                              AABB::newPermanent(0.1875, 0.0, 0.0, 1.0, 0.0625, 1.0),
                                              AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.0625, 0.8125),
                                              AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.0625, 1.0)};

const std::wstring RedStoneWireBlock::TEXTURE_CROSS = L"_cross";
const std::wstring RedStoneWireBlock::TEXTURE_LINE = L"_line";
const std::wstring RedStoneWireBlock::TEXTURE_CROSS_OVERLAY = L"_cross_overlay";
const std::wstring RedStoneWireBlock::TEXTURE_LINE_OVERLAY = L"_line_overlay";

AbstractProperty<int>* RedStoneWireBlock::POWER = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"power", 0, 15));

RedStoneWireBlock::ConnectingSide* RedStoneWireBlock::UP = new RedStoneWireBlock::ConnectingSide(L"up");
RedStoneWireBlock::ConnectingSide* RedStoneWireBlock::SIDE = new RedStoneWireBlock::ConnectingSide(L"side");
RedStoneWireBlock::ConnectingSide* RedStoneWireBlock::NONE = new RedStoneWireBlock::ConnectingSide(L"none");

AbstractProperty<RedStoneWireBlock::ConnectingSide*>* RedStoneWireBlock::NORTH = NULL;
AbstractProperty<RedStoneWireBlock::ConnectingSide*>* RedStoneWireBlock::EAST = NULL;
AbstractProperty<RedStoneWireBlock::ConnectingSide*>* RedStoneWireBlock::SOUTH = NULL;
AbstractProperty<RedStoneWireBlock::ConnectingSide*>* RedStoneWireBlock::WEST = NULL;

static RedStoneWireBlock::ConnectingSide* gRedStoneWireConnectingSides[] = {
    RedStoneWireBlock::UP, RedStoneWireBlock::SIDE, RedStoneWireBlock::NONE};

RedStoneWireBlock::ConnectingSide::ConnectingSide(const std::wstring& name) : mName(name)
{
}

std::wstring RedStoneWireBlock::ConnectingSide::toString() const
{
    return getSerializedName();
}

std::wstring RedStoneWireBlock::ConnectingSide::getSerializedName() const
{
    return mName;
}

RedStoneWireBlock::RedStoneWireBlock() : Block(Material::DECORATION)
{
    Block::DerivedInit();
    mSignalSource = true;
    registerDefaultState(mStateDefinition->any()
                             ->setValue(NORTH, NONE)
                             ->setValue(EAST, NONE)
                             ->setValue(SOUTH, NONE)
                             ->setValue(WEST, NONE)
                             ->setValue(POWER, 0));
    setThickness(0.07f);
}

static inline AbstractProperty<RedStoneWireBlock::ConnectingSide*>* createConnectingSideProperty(
    const std::wstring& name)
{
    const Predicate<RedStoneWireBlock::ConnectingSide*>* filter =
        Predicates<RedStoneWireBlock::ConnectingSide*>::alwaysTrue;
    boost::unordered_set<RedStoneWireBlock::ConnectingSide*> values;
    for (int i = 0; i < 3; ++i)
        values.insert(gRedStoneWireConnectingSides[i]);
    return EnumProperty<RedStoneWireBlock::ConnectingSide*>::create(name, typeid(RedStoneWireBlock::ConnectingSide*),
                                                                    values, filter);
}

void RedStoneWireBlock::blockStaticCtor()
{
    NORTH = createConnectingSideProperty(L"north");
    EAST = createConnectingSideProperty(L"east");
    SOUTH = createConnectingSideProperty(L"south");
    WEST = createConnectingSideProperty(L"west");
}

int RedStoneWireBlock::getAABBIndex(const BlockState* state)
{
    int index = 0;
    bool north = state->getValue(NORTH) != RedStoneWireBlock::NONE;
    bool east = state->getValue(EAST) != RedStoneWireBlock::NONE;
    bool south = state->getValue(SOUTH) != RedStoneWireBlock::NONE;
    bool west = state->getValue(WEST) != RedStoneWireBlock::NONE;

    if (north || (south && !north && !east && !west))
    {
        index |= 1 << Direction::NORTH->get2DDataValue();
    }
    if (east || (west && !north && !east && !south))
    {
        index |= 1 << Direction::EAST->get2DDataValue();
    }
    if (south || (north && !east && !south && !west))
    {
        index |= 1 << Direction::SOUTH->get2DDataValue();
    }
    if (west || (east && !north && !south && !west))
    {
        index |= 1 << Direction::WEST->get2DDataValue();
    }
    return index;
}

const AABB* RedStoneWireBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return gRedStoneWireShapes[getAABBIndex(state->fillVirtualBlockStateProperties(levelSource, pos))];
}

bool RedStoneWireBlock::shouldConnectTo(const BlockState* state, const Direction* direction)
{
    if (state->getBlock() == Blocks::redstone_wire)
    {
        return true;
    }
    if (static_cast<DiodeBlock*>(Blocks::unpowered_repeater)->isSameDiode(state))
    {
        const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
        return facing == direction || facing->getOpposite() == direction;
    }
    if (Blocks::observer == state->getBlock())
    {
        return direction == state->getValue(DirectionalBlock::FACING_PROPERTY);
    }
    return state->isSignalSource() && direction != NULL;
}

bool RedStoneWireBlock::shouldConnectTo(const BlockState* state)
{
    return shouldConnectTo(state, NULL);
}

RedStoneWireBlock::ConnectingSide* RedStoneWireBlock::getConnectingSide(LevelSource* levelSource, const BlockPos& pos,
                                                                        const Direction* direction)
{
    BlockPos neighbourPos = pos.relative(direction);
    const BlockState* neighbour = levelSource->getBlockState(pos.relative(direction));
    if (shouldConnectTo(levelSource->getBlockState(neighbourPos), direction)
        || (!neighbour->isSolidBlockingCubeAndNotSignalSource()
            && shouldConnectTo(levelSource->getBlockState(neighbourPos.below()))))
    {
        return RedStoneWireBlock::SIDE;
    }
    if (!levelSource->getBlockState(pos.above())->isSolidBlockingCubeAndNotSignalSource())
    {
        if (!levelSource->getBlockState(neighbourPos)->isTopSolidBlocking()
            && levelSource->getBlockState(neighbourPos)->getBlock() != Blocks::glowstone)
        {
            return RedStoneWireBlock::NONE;
        }
        if (shouldConnectTo(levelSource->getBlockState(neighbourPos.above())))
        {
            return neighbour->isSolidBlockingCube() ? RedStoneWireBlock::UP : RedStoneWireBlock::SIDE;
        }
    }
    return RedStoneWireBlock::NONE;
}

const BlockState* RedStoneWireBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                                     const BlockPos& pos)
{
    state = state->setValue(WEST, getConnectingSide(levelSource, pos, Direction::WEST));
    state = state->setValue(EAST, getConnectingSide(levelSource, pos, Direction::EAST));
    state = state->setValue(NORTH, getConnectingSide(levelSource, pos, Direction::NORTH));
    return state->setValue(SOUTH, getConnectingSide(levelSource, pos, Direction::SOUTH));
}

const AABB* RedStoneWireBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL_AABB;
}

bool RedStoneWireBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool RedStoneWireBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool RedStoneWireBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.below())->isTopSolidBlocking()
           || level->getBlockState(pos.below())->getBlock() == Blocks::glowstone
           || Block::isExceptionForPistonAttaching(level->getBlockState(pos.below())->getBlock());
}

bool RedStoneWireBlock::waterSpreadCausesSpawn()
{
    return true;
}

bool RedStoneWireBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

int RedStoneWireBlock::checkTarget(Level* level, const BlockPos& pos, int strength)
{
    const BlockState* state = level->getBlockState(pos);
    int data;
    if (state->getBlock() != this || (data = level->getBlockData(pos.x, pos.y, pos.z)) <= strength)
    {
        data = strength;
    }
    return data;
}

const BlockState* RedStoneWireBlock::updatePowerStrength(Level* level, const BlockPos& pos, const BlockPos& sourcePos,
                                                         const BlockState* state)
{
    const BlockState* placedState = state;
    int previousPower = state->getValue(POWER);
    int power = checkTarget(level, sourcePos, 0);

    mSignalSource = false;
    int neighbourSignal = level->getBestNeighborSignal(pos);
    mSignalSource = true;

    if (neighbourSignal > 0 && neighbourSignal > power - 1)
    {
        power = neighbourSignal;
    }

    int wirePower = 0;
    arrayWithLength<const Direction*> horizontal = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < horizontal.length; ++i)
    {
        BlockPos neighbourPos = pos.relative(horizontal.data[i]);
        bool offAxis = neighbourPos.x != sourcePos.x || neighbourPos.z != sourcePos.z;
        if (offAxis)
        {
            wirePower = checkTarget(level, neighbourPos, wirePower);
        }
        if (level->getBlockState(neighbourPos)->isSolidBlockingCubeAndNotSignalSource()
            && !level->getBlockState(pos.above())->isSolidBlockingCubeAndNotSignalSource())
        {
            if (offAxis && pos.y >= sourcePos.y)
            {
                wirePower = checkTarget(level, neighbourPos.above(), wirePower);
            }
        }
        else if (!level->getBlockState(neighbourPos)->isSolidBlockingCubeAndNotSignalSource() && offAxis
                 && pos.y <= sourcePos.y)
        {
            wirePower = checkTarget(level, neighbourPos.below(), wirePower);
        }
    }

    if (wirePower > power)
    {
        power = wirePower - 1;
    }
    else if (power > 0)
    {
        power = power - 1;
    }
    else
    {
        power = 0;
    }

    if (neighbourSignal > power - 1)
    {
        power = neighbourSignal;
    }

    if (previousPower != power)
    {
        state = state->setValue(POWER, power);
        if (level->getBlockState(pos) == placedState)
        {
            int flags = gApp.GetGameHostOption(eGameHostOption_MiniGame) ? 0x24 : 2;
            level->setData(pos.x, pos.y, pos.z, power, flags, false);
        }
        mUpdatedPositions.insert(pos);
        for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin();
             it != Direction::VALUES.end(); ++it)
        {
            mUpdatedPositions.insert(pos.relative(*it));
        }
    }

    return state;
}

const BlockState* RedStoneWireBlock::updatePowerStrength(Level* level, const BlockPos& pos, const BlockState* state)
{
    state = updatePowerStrength(level, pos, pos, state);
    std::vector<BlockPos> positions(mUpdatedPositions.begin(), mUpdatedPositions.end());
    mUpdatedPositions.clear();
    for (std::vector<BlockPos>::const_iterator it = positions.begin(), end = positions.end(); it != end; ++it)
    {
        level->updateNeighborsAt(*it, this, false);
    }
    return state;
}

void RedStoneWireBlock::checkCornerChangeAt(Level* level, const BlockPos& pos)
{
    if (level->getBlockState(pos)->getBlock() != this)
    {
        return;
    }
    level->updateNeighborsAt(pos, this, false);
    for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
         ++it)
    {
        level->updateNeighborsAt(pos.relative(*it), this, false);
    }
}

void RedStoneWireBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->mIsClientSide)
    {
        return;
    }

    updatePowerStrength(level, pos, state);

    arrayWithLength<const Direction*> vertical = Direction::Plane::VERTICAL->iterator();
    for (unsigned int i = 0; i < vertical.length; ++i)
    {
        level->updateNeighborsAt(pos.relative(vertical.data[i]), this, false);
    }

    arrayWithLength<const Direction*> horizontal = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < horizontal.length; ++i)
    {
        checkCornerChangeAt(level, pos.relative(horizontal.data[i]));
    }

    for (unsigned int i = 0; i < horizontal.length; ++i)
    {
        BlockPos neighbourPos = pos.relative(horizontal.data[i]);
        if (level->getBlockState(neighbourPos)->isSolidBlockingCubeAndNotSignalSource())
        {
            checkCornerChangeAt(level, neighbourPos.above());
        }
        else
        {
            checkCornerChangeAt(level, neighbourPos.below());
        }
    }
}

void RedStoneWireBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
    if (level->mIsClientSide)
    {
        return;
    }

    for (std::vector<const Direction*>::const_iterator it = Direction::VALUES.begin(); it != Direction::VALUES.end();
         ++it)
    {
        level->updateNeighborsAt(pos.relative(*it), this, false);
    }

    updatePowerStrength(level, pos, state);

    arrayWithLength<const Direction*> horizontal = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < horizontal.length; ++i)
    {
        checkCornerChangeAt(level, pos.relative(horizontal.data[i]));
    }

    for (unsigned int i = 0; i < horizontal.length; ++i)
    {
        BlockPos neighbourPos = pos.relative(horizontal.data[i]);
        if (level->getBlockState(neighbourPos)->isSolidBlockingCubeAndNotSignalSource())
        {
            checkCornerChangeAt(level, neighbourPos.above());
        }
        else
        {
            checkCornerChangeAt(level, neighbourPos.below());
        }
    }
}

void RedStoneWireBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*,
                                        const BlockPos&)
{
    if (level->mIsClientSide)
    {
        return;
    }
    if (mayPlace(level, pos))
    {
        updatePowerStrength(level, pos, state);
        return;
    }
    spawnResources(level, pos, state, 0);
    level->removeBlock(pos);
}

Item* RedStoneWireBlock::getResource(const BlockState*, Random*, int)
{
    return Items::redstone;
}

int RedStoneWireBlock::getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                       const Direction* direction)
{
    if (!mSignalSource)
    {
        return 0;
    }
    return state->getSignal(levelSource, pos, direction);
}

bool RedStoneWireBlock::shouldConnectTo(LevelSource* levelSource, const BlockPos& pos)
{
    return shouldConnectTo(levelSource->getBlockState(pos));
}

bool RedStoneWireBlock::isPowerSourceAt(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    BlockPos neighbourPos = pos.relative(direction);
    const BlockState* neighbour = levelSource->getBlockState(neighbourPos);
    bool solid = neighbour->isSolidBlockingCubeAndNotSignalSource();
    bool aboveSolid = levelSource->getBlockState(pos.above())->isSolidBlockingCubeAndNotSignalSource();

    if (!aboveSolid && solid && shouldConnectTo(levelSource, neighbourPos.above()))
    {
        return true;
    }
    if (shouldConnectTo(neighbour, direction))
    {
        return true;
    }
    if (neighbour->getBlock() == Blocks::powered_repeater
        && neighbour->getValue(HorizontalDirectionalBlock::FACING_PROPERTY) == direction)
    {
        return true;
    }
    if (!solid && shouldConnectTo(levelSource, neighbourPos.below()))
    {
        return true;
    }
    return false;
}

int RedStoneWireBlock::getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                 const Direction* direction)
{
    if (!mSignalSource)
    {
        return 0;
    }
    int power = state->getValue(POWER);
    if (power == 0)
    {
        return 0;
    }
    if (direction == Direction::UP)
    {
        return power;
    }

    boost::unordered_set<const Direction*> connections;
    arrayWithLength<const Direction*> horizontal = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < horizontal.length; ++i)
    {
        const Direction* side = horizontal.data[i];
        if (isPowerSourceAt(levelSource, pos, side))
        {
            connections.insert(side);
        }
    }

    if (direction->getAxis()->isHorizontal() && connections.empty())
    {
        return power;
    }
    if (connections.find(direction) != connections.end()
        && connections.find(direction->getCounterClockWise()) == connections.end()
        && connections.find(direction->getClockWise()) == connections.end())
    {
        return power;
    }
    return 0;
}

bool RedStoneWireBlock::isSignalSource(const BlockState*)
{
    return mSignalSource;
}

int RedStoneWireBlock::getColorForData(int data)
{
    float value = (float)data / 15.0f;
    float red = value * 0.6f + 0.4f;
    if (data == 0)
    {
        red = 0.3f;
    }
    float green = value * value * 0.7f - 0.5f;
    float blue = value * value * 0.6f - 0.7f;
    if (green < 0.0f)
    {
        green = 0.0f;
    }
    if (blue < 0.0f)
    {
        blue = 0.0f;
    }
    int r = Mth::clamp((int)(red * 255.0f), 0, 255);
    int g = Mth::clamp((int)(green * 255.0f), 0, 255);
    int b = Mth::clamp((int)(blue * 255.0f), 0, 255);
    return r << 16 | 0xff000000 | g << 8 | b;
}

void RedStoneWireBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    int power = state->getValue(POWER);
    if (power != 0)
    {
        double x = (double)pos.x + 0.5 + ((double)random->nextFloat() - 0.5) * 0.2;
        double y = (double)((float)pos.y + 0.0625f);
        double z = (double)pos.z + ((double)random->nextFloat() - 0.5) * 0.2 + 0.5;

        float value = (float)power / 15.0f;
        float green = value * value * 0.7f - 0.5f;
        float blue = value * value * 0.6f - 0.7f;
        if (green < 0.0f)
        {
            green = 0.0f;
        }
        if (blue < 0.0f)
        {
            blue = 0.0f;
        }

        level->addParticle(PARTICLE_RED_DUST, x, y, z, (double)(value * 0.6f + 0.4f), (double)green, (double)blue,
                           arrayWithLength<int>());
    }
}

not_null_ptr<ItemInstance> RedStoneWireBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::redstone));
}

int RedStoneWireBlock::getRenderLayer()
{
    return 0;
}

const BlockState* RedStoneWireBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(POWER, data);
}

int RedStoneWireBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(POWER);
}

const BlockState* RedStoneWireBlock::rotate(const BlockState* state, Rotation* rotation)
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

const BlockState* RedStoneWireBlock::mirror(const BlockState* state, Mirror* mirror)
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

BlockStateDefinition* RedStoneWireBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {NORTH, EAST, SOUTH, WEST, POWER};
    return new BlockStateDefinition(this, properties, 5);
}

EBlockFaceShape RedStoneWireBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

void RedStoneWireBlock::registerIcons(IconRegister* iconRegister)
{
    mCrossIcon = iconRegister->registerIcon(getIconName() + TEXTURE_CROSS);
    mLineIcon = iconRegister->registerIcon(getIconName() + TEXTURE_LINE);
    mCrossOverlayIcon = iconRegister->registerIcon(getIconName() + TEXTURE_CROSS_OVERLAY);
    mLineOverlayIcon = iconRegister->registerIcon(getIconName() + TEXTURE_LINE_OVERLAY);
    mIcon = mCrossIcon;
}

TextureAtlasSprite* RedStoneWireBlock::getTexture(const std::wstring& name)
{
    if (name == TEXTURE_CROSS)
    {
        return static_cast<RedStoneWireBlock*>(Blocks::redstone_wire)->mCrossIcon;
    }
    if (name == TEXTURE_LINE)
    {
        return static_cast<RedStoneWireBlock*>(Blocks::redstone_wire)->mLineIcon;
    }
    if (name == TEXTURE_CROSS_OVERLAY)
    {
        return static_cast<RedStoneWireBlock*>(Blocks::redstone_wire)->mCrossOverlayIcon;
    }
    if (name == TEXTURE_LINE_OVERLAY)
    {
        return static_cast<RedStoneWireBlock*>(Blocks::redstone_wire)->mLineOverlayIcon;
    }
    return NULL;
}

EBlockRenderShape RedStoneWireBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_DUST;
}
