#include "game/world/block/BaseRailBlock.hpp"

#include <typeinfo>

#include "game/core/Direction.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/Material.hpp"

static const AABB* gRailFlatAABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.125, 1.0);
static const AABB* gRailAscendingAABB = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.5, 1.0);

BaseRailBlock::RailShape* BaseRailBlock::RailShape::NORTH_SOUTH = new BaseRailBlock::RailShape(0, L"north_south");
BaseRailBlock::RailShape* BaseRailBlock::RailShape::EAST_WEST = new BaseRailBlock::RailShape(1, L"east_west");
BaseRailBlock::RailShape* BaseRailBlock::RailShape::ASCENDING_EAST = new BaseRailBlock::RailShape(2, L"ascending_east");
BaseRailBlock::RailShape* BaseRailBlock::RailShape::ASCENDING_WEST = new BaseRailBlock::RailShape(3, L"ascending_west");
BaseRailBlock::RailShape* BaseRailBlock::RailShape::ASCENDING_NORTH = new BaseRailBlock::RailShape(4,
                                                                                                   L"ascending_north");
BaseRailBlock::RailShape* BaseRailBlock::RailShape::ASCENDING_SOUTH = new BaseRailBlock::RailShape(5,
                                                                                                   L"ascending_south");
BaseRailBlock::RailShape* BaseRailBlock::RailShape::SOUTH_EAST = new BaseRailBlock::RailShape(6, L"south_east");
BaseRailBlock::RailShape* BaseRailBlock::RailShape::SOUTH_WEST = new BaseRailBlock::RailShape(7, L"south_west");
BaseRailBlock::RailShape* BaseRailBlock::RailShape::NORTH_WEST = new BaseRailBlock::RailShape(8, L"north_west");
BaseRailBlock::RailShape* BaseRailBlock::RailShape::NORTH_EAST = new BaseRailBlock::RailShape(9, L"north_east");

BaseRailBlock::RailShape* BaseRailBlock::RailShape::VALUES[10] = {
    BaseRailBlock::RailShape::NORTH_SOUTH,     BaseRailBlock::RailShape::EAST_WEST,
    BaseRailBlock::RailShape::ASCENDING_EAST,  BaseRailBlock::RailShape::ASCENDING_WEST,
    BaseRailBlock::RailShape::ASCENDING_NORTH, BaseRailBlock::RailShape::ASCENDING_SOUTH,
    BaseRailBlock::RailShape::SOUTH_EAST,      BaseRailBlock::RailShape::SOUTH_WEST,
    BaseRailBlock::RailShape::NORTH_WEST,      BaseRailBlock::RailShape::NORTH_EAST};

bool railfilter(BaseRailBlock::RailShape* shape)
{
    return shape != BaseRailBlock::RailShape::NORTH_EAST && shape != BaseRailBlock::RailShape::NORTH_WEST
           && shape != BaseRailBlock::RailShape::SOUTH_EAST && shape != BaseRailBlock::RailShape::SOUTH_WEST;
}

bool railpredicate(BaseRailBlock::RailShape* shape)
{
    return shape != BaseRailBlock::RailShape::NORTH_EAST && shape != BaseRailBlock::RailShape::NORTH_WEST
           && shape != BaseRailBlock::RailShape::SOUTH_EAST && shape != BaseRailBlock::RailShape::SOUTH_WEST;
}

BaseRailBlock::RailShape::RailShape(int data, const std::wstring& name)
{
    mData = data;
    mName = name;
}

bool BaseRailBlock::RailShape::isAscending()
{
    return this == RailShape::ASCENDING_NORTH || this == RailShape::ASCENDING_EAST || this == RailShape::ASCENDING_SOUTH
           || this == RailShape::ASCENDING_WEST;
}

int BaseRailBlock::RailShape::getData()
{
    return mData;
}

std::wstring BaseRailBlock::RailShape::toString() const
{
    return mName;
}

BaseRailBlock::RailShape* BaseRailBlock::RailShape::byData(int data)
{
    if (data < 0 || data > 9)
        data = 0;
    return VALUES[data];
}

std::wstring BaseRailBlock::RailShape::getSerializedName() const
{
    return mName;
}

BaseRailBlock::BaseRailBlock(bool isPowered) : Block(Material::DECORATION), mIsPowered(isPowered)
{
    mIsRail = true;
    mBlockType = EBLOCK_TYPE_RAIL;
    setThickness(0.07f);
}

bool BaseRailBlock::isRail(const Block* block)
{
    return block == Blocks::rail || block == Blocks::golden_rail || block == Blocks::detector_rail
           || block == Blocks::activator_rail;
}

bool BaseRailBlock::isRail(Level* level, const BlockPos& pos)
{
    return isRail(level->getBlock(pos));
}

bool BaseRailBlock::isRail(const BlockState* state)
{
    return isRail(state->getBlock());
}

const AABB* BaseRailBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL_AABB;
}

bool BaseRailBlock::isSolidRender(const BlockState*) const
{
    return false;
}

const AABB* BaseRailBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    if (state->getBlock() == this)
    {
        RailShape* shape = state->getValue(getShapeProperty());
        if (shape != NULL && shape->isAscending())
        {
            return gRailAscendingAABB;
        }
    }
    return gRailFlatAABB;
}

EBlockFaceShape BaseRailBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool BaseRailBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool BaseRailBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.below())->isTopSolidBlocking()
           || Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.below())->getBlock());
}

void BaseRailBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!level->mIsClientSide)
    {
        state = updateDir(level, pos, state, true);
        if (mIsPowered)
            state->neighborChanged(level, pos, this, pos);
    }
}

void BaseRailBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                    const BlockPos& fromPos)
{
    if (level->mIsClientSide)
    {
        return;
    }

    RailShape* shape = state->getValue(getShapeProperty());
    bool unsupported = false;
    if (!level->getBlockState(pos.below())->isTopSolidBlocking()
        && !Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.below())->getBlock()))
    {
        unsupported = true;
    }

    if ((shape == RailShape::ASCENDING_EAST && !level->getBlockState(pos.east())->isTopSolidBlocking()
         && !Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.east())->getBlock()))
        || (shape == RailShape::ASCENDING_WEST && !level->getBlockState(pos.west())->isTopSolidBlocking()
            && !Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.west())->getBlock()))
        || (shape == RailShape::ASCENDING_NORTH && !level->getBlockState(pos.north())->isTopSolidBlocking()
            && !Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.north())->getBlock()))
        || (shape == RailShape::ASCENDING_SOUTH && !level->getBlockState(pos.south())->isTopSolidBlocking()
            && !Block::isExceptionForTopSolidBlocking(level->getBlockState(pos.south())->getBlock()))
        || unsupported)
    {
        if (!level->isEmptyBlock(pos))
        {
            spawnResources(level, pos, state, 0);
            level->removeBlock(pos);
            return;
        }
    }
    updateState(state, level, pos, block);
}

void BaseRailBlock::updateState(const BlockState*, Level*, const BlockPos&, Block*)
{
}

const BlockState* BaseRailBlock::updateDir(Level* level, const BlockPos& pos, const BlockState* state, bool forceUpdate)
{
    if (!level->mIsClientSide)
    {
        state = Rail(level, pos, state).place(level->hasNeighborSignal(pos), forceUpdate)->getState();
    }
    return state;
}

EPistonPushReaction BaseRailBlock::getPistonPushReaction(const BlockState*)
{
    return PISTON_PUSH_NORMAL;
}

int BaseRailBlock::getRenderLayer()
{
    return 0;
}

void BaseRailBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onRemove(level, pos, state);
    RailShape* shape = state->getValue(getShapeProperty());
    if (shape->isAscending())
        level->updateNeighborsAt(pos.above(), this, false);
    if (mIsPowered)
    {
        level->updateNeighborsAt(pos, this, false);
        level->updateNeighborsAt(pos.below(), this, false);
    }
}

EBlockRenderShape BaseRailBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_RAIL;
}

BaseRailBlock::Rail::Rail(Level* level, const BlockPos& pos, const BlockState* state)
{
    mLevel = level;
    mPos = pos;
    mState = state;
    mBlock = state->getBlock();
    RailShape* shape = state->getValue(static_cast<BaseRailBlock*>(mBlock)->getShapeProperty());
    mPowered = static_cast<BaseRailBlock*>(mBlock)->mIsPowered;
    updateConnections(shape);
}

const BlockState* BaseRailBlock::Rail::getState()
{
    return mState;
}

void BaseRailBlock::Rail::updateConnections(RailShape* shape)
{
    switch (shape->getData())
    {
    case 0:
        mConnections.push_back(mPos.north());
        mConnections.push_back(mPos.south());
        break;
    case 1:
        mConnections.push_back(mPos.west());
        mConnections.push_back(mPos.east());
        break;
    case 2:
        mConnections.push_back(mPos.west());
        mConnections.push_back(mPos.east().above());
        break;
    case 3:
        mConnections.push_back(mPos.west().above());
        mConnections.push_back(mPos.east());
        break;
    case 4:
        mConnections.push_back(mPos.north().above());
        mConnections.push_back(mPos.south());
        break;
    case 5:
        mConnections.push_back(mPos.north());
        mConnections.push_back(mPos.south().above());
        break;
    case 6:
        mConnections.push_back(mPos.east());
        mConnections.push_back(mPos.south());
        break;
    case 7:
        mConnections.push_back(mPos.west());
        mConnections.push_back(mPos.south());
        break;
    case 8:
        mConnections.push_back(mPos.west());
        mConnections.push_back(mPos.north());
        break;
    case 9:
        mConnections.push_back(mPos.east());
        mConnections.push_back(mPos.north());
        break;
    }
}

BaseRailBlock::Rail* BaseRailBlock::Rail::getRail(const BlockPos& pos)
{
    BlockPos railPos = pos;
    const BlockState* state = mLevel->getBlockState(railPos);
    if (!BaseRailBlock::isRail(state))
    {
        railPos = pos.above();
        state = mLevel->getBlockState(railPos);
        if (!BaseRailBlock::isRail(state))
        {
            railPos = pos.below();
            state = mLevel->getBlockState(railPos);
            if (!BaseRailBlock::isRail(state))
                return NULL;
        }
    }
    return new Rail(mLevel, railPos, state);
}

bool BaseRailBlock::Rail::hasConnection(const BlockPos& pos)
{
    std::vector<BlockPos>::iterator end;
    std::vector<BlockPos>::iterator it;
    for (end = mConnections.end(), it = mConnections.begin(); it != end; ++it)
        if (it->x == pos.x && it->z == pos.z)
            return true;
    return false;
}

bool BaseRailBlock::Rail::connectsTo(Rail* rail)
{
    return hasConnection(rail->mPos);
}

bool BaseRailBlock::Rail::canConnectTo(Rail* rail)
{
    return connectsTo(rail) || mConnections.size() != 2;
}

void BaseRailBlock::Rail::removeSoftConnections()
{
    for (unsigned int i = 0; i < mConnections.size(); ++i)
    {
        Rail* rail = getRail(mConnections[i]);
        if (rail == NULL || !rail->connectsTo(this))
        {
            mConnections.erase(mConnections.begin() + i);
            --i;
        }
        else
        {
            mConnections[i] = rail->mPos;
        }
        delete rail;
    }
}

bool BaseRailBlock::Rail::hasNeighborRail(const BlockPos& pos)
{
    Rail* rail = getRail(pos);
    if (rail == NULL)
        return false;
    rail->removeSoftConnections();
    bool result = rail->canConnectTo(this);
    delete rail;
    return result;
}

void BaseRailBlock::Rail::connectTo(Rail* rail)
{
    mConnections.push_back(rail->mPos);

    BlockPos northPos = mPos.north();
    BlockPos southPos = mPos.south();
    BlockPos westPos = mPos.west();
    BlockPos eastPos = mPos.east();
    bool north = hasConnection(northPos);
    bool south = hasConnection(southPos);
    bool west = hasConnection(westPos);
    bool east = hasConnection(eastPos);

    RailShape* shape = NULL;
    if (north || south)
        shape = RailShape::NORTH_SOUTH;
    if (west || east)
        shape = RailShape::EAST_WEST;
    if (!mPowered)
    {
        if (south && east && !north && !west)
            shape = RailShape::SOUTH_EAST;
        if (south && west && !north && !east)
            shape = RailShape::SOUTH_WEST;
        if (north && west && !south && !east)
            shape = RailShape::NORTH_WEST;
        if (north && east && !south && !west)
            shape = RailShape::NORTH_EAST;
    }

    if (shape == RailShape::NORTH_SOUTH)
    {
        if (BaseRailBlock::isRail(mLevel, northPos.above()))
            shape = RailShape::ASCENDING_NORTH;
        if (BaseRailBlock::isRail(mLevel, southPos.above()))
            shape = RailShape::ASCENDING_SOUTH;
    }
    if (shape == RailShape::EAST_WEST)
    {
        if (BaseRailBlock::isRail(mLevel, eastPos.above()))
            shape = RailShape::ASCENDING_EAST;
        if (BaseRailBlock::isRail(mLevel, westPos.above()))
            shape = RailShape::ASCENDING_WEST;
    }
    if (shape == NULL)
        shape = RailShape::NORTH_SOUTH;

    mState = mState->setValue(static_cast<BaseRailBlock*>(mBlock)->getShapeProperty(), shape);
    mLevel->setBlock(mPos, mState, Block::UPDATE_ALL, false);
}

BaseRailBlock::Rail* BaseRailBlock::Rail::place(bool powered, bool forceUpdate)
{
    BlockPos northPos = mPos.north();
    BlockPos southPos = mPos.south();
    BlockPos westPos = mPos.west();
    BlockPos eastPos = mPos.east();
    bool north = hasNeighborRail(northPos);
    bool south = hasNeighborRail(southPos);
    bool west = hasNeighborRail(westPos);
    bool east = hasNeighborRail(eastPos);

    RailShape* shape = NULL;
    if ((north || south) && !west && !east)
        shape = RailShape::NORTH_SOUTH;
    if ((west || east) && !north && !south)
        shape = RailShape::EAST_WEST;
    if (!mPowered)
    {
        if (south && east && !north && !west)
            shape = RailShape::SOUTH_EAST;
        if (south && west && !north && !east)
            shape = RailShape::SOUTH_WEST;
        if (north && west && !south && !east)
            shape = RailShape::NORTH_WEST;
        if (north && east && !south && !west)
            shape = RailShape::NORTH_EAST;
    }

    if (shape == NULL)
    {
        if (north || south)
            shape = RailShape::NORTH_SOUTH;
        if (west || east)
            shape = RailShape::EAST_WEST;
        if (!mPowered)
        {
            if (powered)
            {
                if (south && east)
                    shape = RailShape::SOUTH_EAST;
                if (west && south)
                    shape = RailShape::SOUTH_WEST;
                if (east && north)
                    shape = RailShape::NORTH_EAST;
                if (north && west)
                    shape = RailShape::NORTH_WEST;
            }
            else
            {
                if (north && west)
                    shape = RailShape::NORTH_WEST;
                if (east && north)
                    shape = RailShape::NORTH_EAST;
                if (west && south)
                    shape = RailShape::SOUTH_WEST;
                if (south && east)
                    shape = RailShape::SOUTH_EAST;
            }
        }
    }

    if (shape == RailShape::NORTH_SOUTH)
    {
        if (BaseRailBlock::isRail(mLevel, northPos.above()))
            shape = RailShape::ASCENDING_NORTH;
        if (BaseRailBlock::isRail(mLevel, southPos.above()))
            shape = RailShape::ASCENDING_SOUTH;
    }
    if (shape == RailShape::EAST_WEST)
    {
        if (BaseRailBlock::isRail(mLevel, eastPos.above()))
            shape = RailShape::ASCENDING_EAST;
        if (BaseRailBlock::isRail(mLevel, westPos.above()))
            shape = RailShape::ASCENDING_WEST;
    }
    if (shape == NULL)
        shape = RailShape::NORTH_SOUTH;

    updateConnections(shape);
    mState = mState->setValue(static_cast<BaseRailBlock*>(mBlock)->getShapeProperty(), shape);

    if (forceUpdate || mLevel->getBlockState(mPos) != mState)
    {
        mLevel->setBlock(mPos, mState, Block::UPDATE_ALL, false);
        for (unsigned int i = 0; i < mConnections.size(); ++i)
        {
            Rail* rail = getRail(mConnections.at(i));
            if (rail != NULL)
            {
                rail->removeSoftConnections();
                if (rail->canConnectTo(this))
                    rail->connectTo(this);
                delete rail;
            }
        }
    }
    return this;
}

std::vector<BlockPos>* BaseRailBlock::Rail::getConnections()
{
    return &mConnections;
}

bool BaseRailBlock::Rail::hasRail(const BlockPos& pos)
{
    return BaseRailBlock::isRail(mLevel, pos) || BaseRailBlock::isRail(mLevel, pos.above())
           || BaseRailBlock::isRail(mLevel, pos.below());
}

int BaseRailBlock::Rail::countPotentialConnections()
{
    int count = 0;
    arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
    for (unsigned int i = 0; i < directions.length; ++i)
    {
        if (hasRail(mPos.relative(directions.data[i])))
            ++count;
    }
    return count;
}
