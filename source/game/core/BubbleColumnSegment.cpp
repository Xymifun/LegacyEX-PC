#include "game/core/BubbleColumnSegment.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* BubbleColumnSegment::DRAG = static_cast<AbstractProperty<bool>*>(
    BooleanProperty::create(L"drag"));

static const int BLOCK_ID_AIR = 0;
static const int BLOCK_ID_WATER = 9;
static const int BLOCK_ID_SOUL_SAND = 88;
static const int BLOCK_ID_MAGMA = 213;
static const int BLOCK_ID_BUBBLE_COLUMN_SEGMENT = 272;

BubbleColumnSegment::BubbleColumnSegment(Material* material) : Block(material)
{
    DerivedInit();
    registerDefaultState(defaultBlockState()->setValue(DRAG, true));
}

bool BubbleColumnSegment::isCubeShaped(const BlockState* state)
{
    return false;
}

const AABB* BubbleColumnSegment::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL;
}

bool BubbleColumnSegment::isSolidRender(const BlockState* state) const
{
    return false;
}

void BubbleColumnSegment::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                       const boost::shared_ptr<Entity>& entity)
{
    const BlockState* aboveState = level->getBlockState(entity->mTickCache, pos.above());
    if (aboveState->getBlock() == Blocks::water || aboveState->getBlock() == Blocks::air)
    {
        bool drag = state->getValue(DRAG);
        entity->onAboveBubbleCol(drag);

        if (!level->mIsClientSide && level->mRandom->nextInt(4) == 0)
        {
            ServerLevel* serverLevel = static_cast<ServerLevel*>(level);
            for (int i = 0; i < 2; ++i)
            {
                serverLevel->sendParticles(PARTICLE_SPLASH, (double)((float)pos.x + level->mRandom->nextFloat()),
                                           (double)(pos.y + 1), (double)((float)pos.z + level->mRandom->nextFloat()), 1,
                                           0.0, 0.0, 0.0, 1.0, arrayWithLength<int>());
                serverLevel->sendParticles(PARTICLE_BUBBLE, (double)((float)pos.x + level->mRandom->nextFloat()),
                                           (double)(pos.y + 1), (double)((float)pos.z + level->mRandom->nextFloat()), 1,
                                           0.0, 0.01, 0.0, 0.2, arrayWithLength<int>());
            }
        }
    }
    else
    {
        entity->onInsideBubbleColumn(state->getValue(DRAG));
    }
}

void BubbleColumnSegment::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    level->addToTickNextTick(pos, this, getTickDelay(level));
}

bool BubbleColumnSegment::getFlowDownward(const BlockState* state)
{
    if (state->getBlock() == Blocks::soul_sand)
    {
        return false;
    }

    if (state->getBlock() == Blocks::bubble_column_segment)
    {
        return state->getValue(DRAG);
    }

    return true;
}

bool BubbleColumnSegment::isValidBubbleColumnLocation(Level* level, const BlockPos& pos)
{
    BlockPos above = pos.above();
    int id = level->getBlockId(pos.x, pos.y, pos.z);

    if (id == BLOCK_ID_WATER)
    {
        if (level->getBlockId(above.x, above.y, above.z) == BLOCK_ID_AIR)
        {
            const BlockState* state = level->getBlockState(pos);
            if (state->getMaterial() == Material::WATER && state->getValue(LiquidBlock::LEVEL) != 0)
            {
                return false;
            }
        }
    }
    else if (id != BLOCK_ID_BUBBLE_COLUMN_SEGMENT)
    {
        return true;
    }

    int x = pos.x;
    int y = pos.y;
    int z = pos.z;
    if (level->getBlockId(x - 1, y, z) == BLOCK_ID_AIR || level->getBlockId(x + 1, y, z) == BLOCK_ID_AIR
        || level->getBlockId(x, y, z - 1) == BLOCK_ID_AIR || level->getBlockId(x, y, z + 1) == BLOCK_ID_AIR)
    {
        return false;
    }

    return true;
}

bool BubbleColumnSegment::addBubbleColumnSegment(Level* level, const BlockPos& pos)
{
    BlockPos above = pos.above();
    if (level->getBlockId(above.x, above.y, above.z) != BLOCK_ID_BUBBLE_COLUMN_SEGMENT
        && level->isEmptyWaterBlock(above) && isValidBubbleColumnLocation(level, above))
    {
        const BlockState* defaultState = Blocks::bubble_column_segment->defaultBlockState();
        const BlockState* bubbleState = defaultState->setValue(DRAG, getFlowDownward(level->getBlockState(pos)));
        level->setBlock(above, bubbleState, Block::UPDATE_CLIENTS, false);
        level->setExtraBlock(above, Blocks::flowing_water->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        return true;
    }

    return false;
}

void BubbleColumnSegment::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (!canSurvive(level, pos, state))
    {
        level->setBlockAndUpdate(pos, Blocks::water->defaultBlockState());
        level->neighborChanged(pos, level->getBlock(pos), pos);
    }
    else
    {
        addBubbleColumnSegment(level, pos);
    }
}

void BubbleColumnSegment::RectifyBubbleFlow(Level* level, const BlockPos& pos, bool down)
{
    bool flow = getFlowDownward(level->getBlockState(pos));
    if (flow != down)
    {
        const BlockState* bubbleState = Blocks::bubble_column_segment->defaultBlockState()->setValue(DRAG, down);
        level->setBlock(pos, bubbleState, Block::UPDATE_CLIENTS, false);

        BlockPos above = pos.above();
        if (level->getBlockId(above.x, above.y, above.z) == BLOCK_ID_BUBBLE_COLUMN_SEGMENT)
        {
            RectifyBubbleFlow(level, above, down);
        }
    }
}

int BubbleColumnSegment::getTickDelay(Level* level)
{
    return 5;
}

void BubbleColumnSegment::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (state->getValue(DRAG))
    {
        const BlockState* aboveState = level->getBlockState(pos.above());
        if (aboveState->getBlock() == Blocks::water || aboveState->getBlock() == Blocks::air
            || level->mRandom->nextInt(3) == 0)
        {
            level->addAlwaysVisibleParticle(ePARTICLE_CURRENT_DOWN, pos.x + 0.3, pos.y + 0.8, pos.z + 0.3, 0.0, 0.0,
                                            0.0, arrayWithLength<int>());
        }

        if (random->nextInt(200) == 0)
        {
            level->playLocalSound((double)pos.x, (double)pos.y, (double)pos.z,
                                  SoundEvents::BUBBLE_COLUMN_WHIRLPOOL_AMBIENT, SoundSource::eSoundSource_Blocks,
                                  0.2f + random->nextFloat() * 0.2f, 0.9f + random->nextFloat() * 0.15f, false, 16.0f);
        }
    }
    else
    {
        const BlockState* aboveState = level->getBlockState(pos.above());
        if (aboveState->getBlock() == Blocks::water || aboveState->getBlock() == Blocks::air
            || level->mRandom->nextInt(4) == 0)
        {
            level->addAlwaysVisibleParticle(ePARTICLE_BUBBLE, pos.x + 0.5, (double)pos.y, pos.z + 0.5, 0.0, 0.04, 0.0,
                                            arrayWithLength<int>());
            level->addAlwaysVisibleParticle(ePARTICLE_BUBBLE, (double)((float)pos.x + random->nextFloat()),
                                            (double)((float)pos.y + random->nextFloat()),
                                            (double)((float)pos.z + random->nextFloat()), 0.0, 0.04, 0.0,
                                            arrayWithLength<int>());
        }

        if (random->nextInt(200) == 0)
        {
            level->playLocalSound((double)pos.x, (double)pos.y, (double)pos.z,
                                  SoundEvents::BUBBLE_COLUMN_UPWARDS_AMBIENT, SoundSource::eSoundSource_Blocks,
                                  0.2f + random->nextFloat() * 0.2f, 0.9f + random->nextFloat() * 0.15f, false, 16.0f);
        }
    }
}

void BubbleColumnSegment::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                          const BlockPos& fromPos)
{
    level->addToTickNextTick(pos, this, getTickDelay(level));
}

bool BubbleColumnSegment::canSurvive(Level* level, const BlockPos& pos, const BlockState* state)
{
    BlockPos below = pos.below();
    int belowId = level->getBlockId(below.x, below.y, below.z);
    bool supported = belowId == BLOCK_ID_BUBBLE_COLUMN_SEGMENT || belowId == BLOCK_ID_MAGMA
                     || belowId == BLOCK_ID_SOUL_SAND;
    bool valid = isValidBubbleColumnLocation(level, pos);

    if (!supported || !valid)
    {
        return false;
    }

    if (belowId == BLOCK_ID_BUBBLE_COLUMN_SEGMENT)
    {
        RectifyBubbleFlow(level, pos, getFlowDownward(level->getBlockState(below)));
    }
    else
    {
        bool down = belowId == BLOCK_ID_MAGMA;
        if (down != getFlowDownward(level->getBlockState(pos)))
        {
            const BlockState* bubbleState = Blocks::bubble_column_segment->defaultBlockState()->setValue(DRAG, down);
            level->setBlock(pos, bubbleState, Block::UPDATE_CLIENTS, false);
        }

        BlockPos above = pos.above();
        if (level->getBlockId(above.x, above.y, above.z) == BLOCK_ID_BUBBLE_COLUMN_SEGMENT)
        {
            RectifyBubbleFlow(level, above, down);
        }
    }

    return true;
}

bool BubbleColumnSegment::canInstantlyTick()
{
    return false;
}

bool BubbleColumnSegment::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    return true;
}

bool BubbleColumnSegment::mayPick()
{
    return false;
}

Item* BubbleColumnSegment::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return NULL;
}

int BubbleColumnSegment::getResourceCount(Random* random)
{
    return 0;
}

int BubbleColumnSegment::getRenderLayer()
{
    return 1;
}

EBlockFaceShape BubbleColumnSegment::getBlockFaceShape(LevelSource* levelSource, const BlockState* state,
                                                       const BlockPos& pos, const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

EBlockRenderShape BubbleColumnSegment::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_INVISIBLE;
}

const BlockState* BubbleColumnSegment::getBlockState(int data)
{
    return defaultBlockState()->setValue(DRAG, data != 0);
}

int BubbleColumnSegment::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(DRAG);
}

bool BubbleColumnSegment::canContainLiquid(const BlockState* state) const
{
    return true;
}

bool BubbleColumnSegment::mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos)
{
    return true;
}

EPistonPushReaction BubbleColumnSegment::getPistonPushReaction(const BlockState* state)
{
    return PISTON_PUSH_DESTROY;
}

BlockStateDefinition* BubbleColumnSegment::createBlockStateDefinition()
{
    static const Property* properties[1] = {DRAG};
    return new BlockStateDefinition(this, properties, 1);
}
