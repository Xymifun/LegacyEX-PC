#include "game/world/block/DynamicLiquidBlock.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"

DynamicLiquidBlock::DynamicLiquidBlock(Material* material) : LiquidBlock(material), mTickQueue()
{
    LiquidBlock::DerivedInit();
    mAdjacentSources = 0;
    mProcessingTicks = false;
}

void DynamicLiquidBlock::setStatic(Level* level, const BlockPos& pos, bool extraLayer)
{
    unsigned short staticElement = LiquidBlock::getStaticLiquidElementId(LiquidBlock::getLiquidElement(level, pos));
    int blockId = extraLayer ? level->getExtraBlockId(pos.x, pos.y, pos.z) : level->getBlockId(pos.x, pos.y, pos.z);
    if (blockId != BlockId::FLOWING_WATER)
    {
        if (blockId != BlockId::FLOWING_LAVA)
            return;
        extraLayer = false;
    }
    LiquidBlock::setLiquidElement(level, pos, staticElement, extraLayer, 18);
}

int DynamicLiquidBlock::getHighest(Level* level, const BlockPos& pos, int currentHighest)
{
    int depth = getDepth(level, pos);
    if (depth >= 0)
    {
        if (depth < 1)
        {
            ++mAdjacentSources;
        }
        else if (depth >= 8)
        {
            depth = 0;
        }
        if (currentHighest < 0 || depth < currentHighest)
        {
            return depth;
        }
    }
    return currentHighest;
}

bool DynamicLiquidBlock::_isWaterBlocking(Level* level, const BlockPos& pos, const BlockPos& sourcePos,
                                          const Direction* direction)
{
    PIXBeginNamedEvent(0.0F, "isWaterBlocking");
    Block* block = level->getBlock(pos);
    PIXBeginNamedEvent(0.0F, "isWaterBlocking check 1");
    if (!block->isWaterBlocking())
    {
        PIXEndNamedEvent();
        PIXBeginNamedEvent(0.0F, "isWaterBlocking check 2");
        if (block->liquidCanFlowIntoFromDirection(level, pos, direction))
        {
            PIXEndNamedEvent();
            PIXBeginNamedEvent(0.0F, "isWaterBlocking check 3");
            Block* sourceBlock = level->getBlock(sourcePos);
            if (sourceBlock->liquidCanFlowIntoFromDirection(level, sourcePos, direction->getOpposite()))
            {
                PIXEndNamedEvent();
                PIXEndNamedEvent();
                return false;
            }
        }
    }
    PIXEndNamedEvent();
    PIXEndNamedEvent();
    return true;
}

bool DynamicLiquidBlock::_canSpreadTo(Level* level, const BlockPos& pos, const BlockPos& sourcePos,
                                      const Direction* direction)
{
    int size = level->mChunkCacheSize;
    int chunkX = (pos.x >> 4) + size / 2;
    int chunkZ = (pos.z >> 4) + size / 2;
    if (chunkX >= 0 && chunkX < size && chunkZ >= 0 && chunkZ < size)
    {
        Block* liquid = Block::byId(LiquidBlock::getLiquidElement(level, pos) >> 4);
        if (liquid->mMaterial != mMaterial && liquid->mMaterial != Material::LAVA)
            return !_isWaterBlocking(level, pos, sourcePos, direction);
    }
    return false;
}

void DynamicLiquidBlock::_spread(Level* level, const BlockPos& pos, int depth, bool extraLayer)
{
    unsigned short element = static_cast<unsigned short>(depth
                                                         | ((getCraftingMaterial() == Material::LAVA ? 10 : 8) << 4));
    LiquidBlock::setLiquidElement(level, pos, element, extraLayer, 3);
    level->addToTickNextTick(pos, this, getTickDelay(level));
}

void DynamicLiquidBlock::_trySpreadTo(Level* level, const BlockPos& pos, const BlockPos& sourcePos,
                                      const Direction* direction, int depth)
{
    PIXBeginNamedEvent(0.0F, "Try spread to");
    if (_canSpreadTo(level, pos, sourcePos, direction))
    {
        bool extraLayer = false;
        Block* block = level->getBlock(pos);
        if (block->mMaterial != Material::AIR)
        {
            extraLayer = true;
            if (mMaterial == Material::LAVA)
            {
                fizz(level, pos);
                _spread(level, pos, depth, false);
                PIXEndNamedEvent();
                return;
            }
            if (block->waterSpreadCausesSpawn())
            {
                extraLayer = false;
                level->levelEvent(LevelEvent::PARTICLES_DESTROY_BLOCK, pos,
                                  Block::getIdWithData(level->getBlockState(pos)));
                block->spawnResources(level, pos, level->getBlockState(pos), 0);
            }
        }
        _spread(level, pos, depth, extraLayer);
    }
    PIXEndNamedEvent();
}

int DynamicLiquidBlock::_getSlopeDistance(Level* level, const BlockPos& pos, int distance, int directionIndex)
{
    PIXBeginNamedEvent(0.0F, "getSlopeDistance");
    int best = 1000;
    MutableBlockPos next(pos);
    for (int index = 0; index < 4; ++index)
    {
        if (index == 0 && directionIndex == 1)
            continue;
        if (index == 1 && directionIndex == 0)
            continue;
        if (index == 2 && directionIndex == 3)
            continue;
        if (index == 3 && directionIndex == 2)
            continue;

        next = MutableBlockPos(pos);
        const Direction* fromDirection = Direction::NORTH;
        if (index == 0)
        {
            fromDirection = Direction::EAST;
            next.addX(-1);
        }
        else if (index == 1)
        {
            fromDirection = Direction::WEST;
            next.addX(1);
        }
        else if (index == 2)
        {
            fromDirection = Direction::SOUTH;
            next.addZ(-1);
        }
        else if (index == 3)
        {
            next.addZ(1);
        }
        if (_isWaterBlocking(level, next, pos, fromDirection))
            continue;

        const BlockState* state = level->getBlockState(next);
        if (state->getBlock()->mMaterial == mMaterial && state->getValue(LEVEL) == 0)
            continue;

        if (!_isWaterBlocking(level, next.below(), pos, Direction::UP))
        {
            PIXEndNamedEvent();
            return distance;
        }

        int limit = mMaterial == Material::LAVA ? 2 : 4;
        if (distance < limit)
        {
            int slope = _getSlopeDistance(level, next, distance + 1, index);
            if (slope < best)
                best = slope;
        }
    }
    PIXEndNamedEvent();
    return best;
}

void DynamicLiquidBlock::_getSpread(Level* level, const BlockPos& pos, bool (&spread)[4])
{
    PIXBeginNamedEvent(0.0F, "Get Spread");
    int distances[4];
    MutableBlockPos next(pos);
    for (unsigned int index = 0; index < 4; ++index)
    {
        distances[index] = 1000;
        next = MutableBlockPos(pos);
        const Direction* fromDirection = Direction::NORTH;
        if (index == 0)
        {
            fromDirection = Direction::EAST;
            next.addX(-1);
        }
        else if (index == 1)
        {
            fromDirection = Direction::WEST;
            next.addX(1);
        }
        else if (index == 2)
        {
            fromDirection = Direction::SOUTH;
            next.addZ(-1);
        }
        else
        {
            next.addZ(1);
        }
        if (_isWaterBlocking(level, next, pos, fromDirection))
            continue;

        int element = level->getElement(next.x, next.y, next.z);
        Block* block = level->getBlock(next);
        if (block->mMaterial != mMaterial || (element & 0xf) != 0)
        {
            int slopeDistance = 0;
            if (_isWaterBlocking(level, next.below(), pos, Direction::UP))
                slopeDistance = _getSlopeDistance(level, next, 1, index);
            distances[index] = slopeDistance;
        }
    }

    int minimum = distances[0];
    if (distances[1] <= minimum)
        minimum = distances[1];
    if (distances[2] <= minimum)
        minimum = distances[2];
    if (distances[3] <= minimum)
        minimum = distances[3];
    spread[0] = distances[0] == minimum;
    spread[1] = distances[1] == minimum;
    spread[2] = distances[2] == minimum;
    spread[3] = distances[3] == minimum;
    PIXEndNamedEvent();
}

void DynamicLiquidBlock::maintick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    bool extraLayer = level->getBlockState(pos)->getMaterial() != mMaterial;
    int depth = getDepth(level, pos);
    int dropOff = 1;
    if (mMaterial == Material::LAVA)
    {
        if (!level->mDimension->isUltraWarm())
            dropOff = 2;
        trySpreadFire(level, pos, random);
    }
    int tickDelay = getTickDelay(level);

    bool placeStatic = false;
    bool lowDepth = depth < 1;
    if (!lowDepth)
    {
        PIXBeginNamedEvent(0.0F, "Tick depth != 0");
        int highest = -100;
        mAdjacentSources = 0;
        arrayWithLength<const Direction*> directions = Direction::Plane::HORIZONTAL->iterator();
        for (unsigned int i = 0; i < directions.length; ++i)
            highest = getHighest(level, pos.relative(directions.data[i]), highest);

        int newDepth = highest + dropOff;
        if (newDepth >= 8 || highest < 0)
            newDepth = -1;

        int aboveDepth = getDepth(level, pos.above());
        if (aboveDepth >= 0)
        {
            if (aboveDepth >= 8)
                newDepth = aboveDepth;
            else
                newDepth = aboveDepth + 8;
        }

        if (mAdjacentSources == 1 && getDepth(level, pos.above()) == 0)
            ++mAdjacentSources;

        if (mAdjacentSources >= 2 && mMaterial == Material::WATER)
        {
            const BlockState* belowState = level->getBlockState(pos.below());
            if (belowState->getMaterial()->isSolid())
            {
                newDepth = 0;
            }
            else if (belowState->getBlock()->mMaterial == mMaterial && belowState->getValue(LEVEL) == 0)
            {
                newDepth = 0;
            }
        }

        if (mMaterial == Material::LAVA && depth < 8 && newDepth < 8 && newDepth > depth && random->nextInt(4) != 0)
            tickDelay *= 4;

        if (newDepth != depth)
        {
            depth = newDepth;
            if (newDepth < 0)
            {
                if (extraLayer)
                    level->setExtraBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
                else
                    level->removeBlock(pos);
            }
            else
            {
                state = state->setValue(LEVEL, depth);
                if (mMaterial == Material::LAVA)
                    extraLayer = false;
                if (extraLayer)
                    level->setExtraBlock(pos, state, Block::UPDATE_CLIENTS, false);
                else
                    level->setBlock(pos, state, Block::UPDATE_CLIENTS, false);
                level->addToTickNextTick(pos, this, tickDelay);
                level->updateNeighborsAt(pos, this, false);
            }
            PIXEndNamedEvent();
        }
        else
        {
            placeStatic = true;
            PIXEndNamedEvent();
        }
    }
    if (lowDepth || placeStatic)
    {
        PIXBeginNamedEvent(0.0F, "Tick setStatic");
        setStatic(level, pos, extraLayer);
        PIXEndNamedEvent();
    }

    PIXBeginNamedEvent(0.0F, "Check for flow and spread");
    if (_canSpreadTo(level, pos.below(), pos, Direction::UP))
    {
        PIXBeginNamedEvent(0.0F, "Can flow");
        if (mMaterial == Material::LAVA
            && (!Minecraft::InMiniGame(eMiniGameId_Any, true)
                || Minecraft::GetInstance()->GetMiniGame()->canLavaConvertWater()))
        {
            Material* belowMaterial = level->getBlockState(pos.below())->getMaterial();
            if (belowMaterial == Material::WATER || belowMaterial == Material::BUBBLE_COLUMN)
            {
                level->setBlockAndUpdate(pos.below(), Blocks::stone->defaultBlockState());
                fizz(level, pos.below());
                PIXEndNamedEvent();
                PIXEndNamedEvent();
                return;
            }
        }
        if (depth >= 8)
            _trySpreadTo(level, pos.below(), pos, Direction::UP, depth);
        else
            _trySpreadTo(level, pos.below(), pos, Direction::UP, depth + 8);
        PIXEndNamedEvent();
    }
    else if (depth >= 0 && (depth == 0 || _isWaterBlocking(level, pos.below(), pos, Direction::UP)))
    {
        PIXBeginNamedEvent(0.0F, "Cannot flow");
        bool spread[4];
        _getSpread(level, pos, spread);
        int spreadDepth = depth + dropOff;
        if (depth >= 8)
            spreadDepth = 1;
        if (spreadDepth < 8)
        {
            if (spread[0])
                _trySpreadTo(level, pos.west(), pos, Direction::EAST, spreadDepth);
            if (spread[1])
                _trySpreadTo(level, pos.east(), pos, Direction::WEST, spreadDepth);
            if (spread[2])
                _trySpreadTo(level, pos.north(), pos, Direction::SOUTH, spreadDepth);
            if (spread[3])
                _trySpreadTo(level, pos.south(), pos, Direction::NORTH, spreadDepth);
        }
        PIXEndNamedEvent();
    }
    PIXEndNamedEvent();
}

void DynamicLiquidBlock::iterativetick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    _LiquidTickData first(level, pos, state, random);
    mTickQueue.push_back(first);
    for (int count = 100; !mTickQueue.empty() && count > 0; --count)
    {
        const _LiquidTickData data = mTickQueue.front();
        mTickQueue.pop_front();
        maintick(data.level, data.pos, data.state, data.random);
    }
    mTickQueue.clear();
}

void DynamicLiquidBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!mProcessingTicks)
    {
        if (Level::getInstaTick())
        {
            mProcessingTicks = true;
            iterativetick(level, pos, state, random);
            mProcessingTicks = false;
            return;
        }
        if (!mProcessingTicks)
        {
            maintick(level, pos, state, random);
            return;
        }
    }
    if (Level::getInstaTick())
    {
        _LiquidTickData data(level, pos, state, random);
        mTickQueue.push_back(data);
    }
    else
    {
        maintick(level, pos, state, random);
    }
}

void DynamicLiquidBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!updateLiquid(level, pos, state))
    {
        level->addToTickNextTick(pos, this, getTickDelay(level));
    }
}

bool DynamicLiquidBlock::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return true;
}
